/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOOrganController.h"

#include <algorithm>
#include <cassert>

#include <wx/filename.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/txtstrm.h>
#include <wx/wfstream.h>

#include "archive/GOArchive.h"
#include "archive/GOArchiveFile.h"
#include "combinations/GODivisionalSetter.h"
#include "combinations/GOSetter.h"
#include "combinations/control/GOGeneralButtonControl.h"
#include "config/GOConfig.h"
#include "config/GOConfigFileReader.h"
#include "config/GOConfigFileWriter.h"
#include "config/GOConfigReader.h"
#include "config/GOConfigReaderDB.h"
#include "config/GOConfigWriter.h"
#include "control/GOElementCreator.h"
#include "files/GOOpenedFile.h"
#include "gui/GOGuiImageCache.h"
#include "gui/dialogs/go-message-boxes.h"
#include "gui/panels/GOGUIPanel.h"
#include "loader/GOLoadThread.h"
#include "loader/GOLoaderFilename.h"
#include "loader/GOOrganReader.h"
#include "loader/GOProgressMonitor.h"
#include "loader/cache/GOCache.h"
#include "loader/cache/GOCacheWriter.h"
#include "midi/GOMidiPlayer.h"
#include "midi/GOMidiRecorder.h"
#include "midi/GOMidiSystem.h"
#include "midi/events/GOMidiEvent.h"
#include "model/GODivisionalCoupler.h"
#include "model/GOEnclosure.h"
#include "model/GOManual.h"
#include "model/GORank.h"
#include "model/GOSoundingPipe.h"
#include "model/GOSwitch.h"
#include "model/GOTremulant.h"
#include "sound/GOSoundSystem.h"
#include "sound/playing/GOSoundReleaseAlignTable.h"
#include "temperaments/GOTemperament.h"
#include "yaml/GOYamlModel.h"

#include "go_defs.h"

#include "GOAudioRecorder.h"
#include "GOBuffer.h"
#include "GOEvent.h"
#include "GOHash.h"
#include "GOMetronome.h"
#include "GOOrgan.h"
#include "GOTimer.h"
#include "go_path.h"

static const wxString WX_ORGAN = wxT("Organ");
static const wxString WX_GRANDORGUE_VERSION = wxT("GrandOrgueVersion");

GOOrganController::GOOrganController(GOConfig &config, bool isAppInitialized)
  : GOEventDistributor(this),
    GOOrganModel(config),
    m_config(config),
    m_ConfiguredOrgan(wxEmptyString),
    m_FileStore(config),
    m_Cacheable(false),
    m_IsOrganCoreDataLoaded(false),
    m_IsOrganGuiLoaded(false),
    m_IsObjectsLoaded(false),
    m_setter(0),
    m_AudioRecorder(NULL),
    m_MidiPlayer(NULL),
    m_MidiRecorder(NULL),
    m_timer(NULL),
    p_OnStateButton(nullptr),
    m_CurrentPitch(999999.0), // for enforcing updating the label first time
    m_OrganModified(false),
    m_midi(0),
    m_SampleSetId1(0),
    m_SampleSetId2(0),
    m_SoundEngine(*this, m_pool),
    mp_ImageCache(nullptr),
    m_PitchLabel(*this),
    m_TemperamentLabel(*this),
    m_MainWindowData(*this, wxT("MainWindow")) {
  // GOTimer needs no live wx event loop to construct (only to fire), so it
  // is created unconditionally: GOAudioRecorder/GOMidiPlayer/GOMidiRecorder/
  // GOMetronome (created by LoadOrganCoreData(), independent of
  // isAppInitialized) all assume GetTimer() is non-null.
  m_timer = new GOTimer();
  if (isAppInitialized) {
    // Load here objects that needs App (wx) to be loaded
    mp_ImageCache = new GOGuiImageCache(m_FileStore);
  }
  GOOrganModel::SetModelModificationListener(this);
  m_setter = new GOSetter(this);
  // Register m_setter for ownership immediately: m_elementcreators.clear()
  // in the destructor is what frees it, and Load() (which used to be the
  // only place registering it) may never run (e.g. in tests, or if loading
  // fails before reaching that point), which would otherwise leak it.
  m_elementcreators.push_back(m_setter);
  m_pool.SetMemoryLimit(m_config.MemoryLimit() * 1024 * 1024);
}

GOOrganController::~GOOrganController() {
  // Callers must call Clear() explicitly before destroying this object (see
  // its doc-comment): OnClear() is virtual, and a call made from here would
  // never reach a subclass override, since by now the object's dynamic
  // type has already unwound to GOOrganController.
  //
  // ClearObjects() must run before OnClear() (Clear()'s own order), so by
  // the time we get here it is already too late to call it - OnClear() is
  // expected to have already run (by the caller) with ClearObjects()
  // having already preceded it. We only assert that it did.
  //
  // ClearOrganCoreData() is different: it is non-virtual, idempotent, and
  // has no ordering dependency on OnClear() having actually reached a
  // subclass override, so it is safe - and necessary - to (re-)run it here
  // unconditionally, regardless of NDEBUG: it frees m_elementcreators,
  // which must happen before m_timer is deleted below.
  assert(!m_IsObjectsLoaded);
  assert(!m_IsOrganGuiLoaded);
  ClearOrganCoreData();
  m_FileStore.CloseArchives();
  if (mp_ImageCache)
    delete mp_ImageCache;
  if (m_timer)
    delete m_timer;
}

void GOOrganController::ClearObjects() {
  if (m_IsObjectsLoaded) {
    m_Cacheable = false;
    m_IsObjectsLoaded = false;
  }
}

void GOOrganController::ClearOrganGuiData() {
  if (m_IsOrganGuiLoaded) {
    m_panels.clear();
    m_IsOrganGuiLoaded = false;
  }
}

void GOOrganController::ClearOrganCoreData() {
  if (m_IsOrganCoreDataLoaded) {
    p_OnStateButton = nullptr;

    // Just to be sure, that the sound providers are freed before the pool
    GOOrganModel::Cleanup();

    m_VirtualCouplers.Cleanup();
    GOOrganModel::SetModelModificationListener(nullptr);
    GOOrganModel::SetCombinationController(nullptr);

    // m_elementcreators (a ptr_vector) is the sole owner of m_setter,
    // m_DivisionalSetter, m_AudioRecorder, m_MidiRecorder, m_MidiPlayer, and
    // the anonymous GOMetronome pushed onto it in LoadOrganCoreData().
    // ptr_vector::clear() deletes every contained pointer (see
    // ptrvector.h:59-68), so this single call is what actually frees all
    // six objects; the raw-pointer members below are then only reset to
    // nullptr to avoid leaving them dangling, not separately deleted.
    m_elementcreators.clear();

    m_setter = nullptr;
    m_DivisionalSetter = nullptr;
    m_AudioRecorder = nullptr;
    m_MidiRecorder = nullptr;
    m_MidiPlayer = nullptr;

    m_IsOrganCoreDataLoaded = false;
  }
}

void GOOrganController::Clear() {
  ClearObjects();
  OnClear();
  ClearOrganCoreData();
}

void GOOrganController::SetOrganModified(bool modified) {
  if (modified != m_OrganModified) {
    m_OrganModified = modified;
    m_setter->UpdateModified(modified);
  }
  GOModificationProxy::OnIsModifiedChanged(modified);
}

void GOOrganController::OnIsModifiedChanged(bool modified) {
  if (modified) {
    // Update the pitch label if it has been changed
    const float newPitch
      = GetRootPipeConfigNode().GetPipeConfig().GetManualTuning();

    if (newPitch != m_CurrentPitch) {
      m_PitchLabel.SetContent(wxString::Format(_("%0.1f cent"), newPitch));
      m_CurrentPitch = newPitch;
    }
    // If the organ model is modified then the organ is also modified
    SetOrganModified(true);
  }
  // else nothing because the organ may be modified without the model
}

void GOOrganController::ResetOrganModified() {
  // if the whole organ becomes not modified then the model also becomes
  // not modified
  ResetOrganModelModified();
  SetOrganModified(false);
}

GOHashType GOOrganController::GenerateCacheHash() {
  GOHash hash;

  UpdateHash(hash);
  hash.Update(sizeof(GOSoundAudioSection));
  hash.Update(sizeof(GOSoundingPipe));
  hash.Update(sizeof(GOSoundReleaseAlignTable));
  hash.Update(BLOCK_HISTORY);
  hash.Update(GOSoundAudioSection::getMaxReadAhead());
  hash.Update(SHORT_LOOP_LENGTH);
  GOSoundProvider::UpdateCacheHash(hash);
  hash.Update(sizeof(GOSoundAudioSection::StartSegment));
  hash.Update(sizeof(GOSoundAudioSection::EndSegment));
  return hash.getHash();
}

void GOOrganController::LoadOrganCoreData(GOConfigReader &cfg) {
  m_IsOrganCoreDataLoaded = true;

  /* load church info */
  cfg.ReadString(
    ODFSetting, WX_ORGAN, wxT("HauptwerkOrganFileFormatVersion"), false);
  m_ChurchAddress = cfg.ReadString(ODFSetting, WX_ORGAN, wxT("ChurchAddress"));
  m_OrganBuilder
    = cfg.ReadString(ODFSetting, WX_ORGAN, wxT("OrganBuilder"), false);
  m_OrganBuildDate
    = cfg.ReadString(ODFSetting, WX_ORGAN, wxT("OrganBuildDate"), false);
  m_OrganComments
    = cfg.ReadString(ODFSetting, WX_ORGAN, wxT("OrganComments"), false);
  m_RecordingDetails
    = cfg.ReadString(ODFSetting, WX_ORGAN, wxT("RecordingDetails"), false);
  wxString info_filename
    = cfg.ReadFileName(ODFSetting, WX_ORGAN, wxT("InfoFilename"), false);
  wxFileName fn;
  m_InfoFilename = wxEmptyString;
  if (info_filename.IsEmpty()) {
    /* Resolve organ file path */
    fn = GetODFFilename();
    fn.SetExt(wxT("html"));
    if (fn.FileExists() && !m_FileStore.AreArchivesUsed())
      m_InfoFilename = fn.GetFullPath();
  } else {
    if (!m_FileStore.AreArchivesUsed()) {
      fn = GOLoaderFilename::generateFullPath(
        info_filename, wxFileName(GetODFFilename()).GetPath());
      if (
        fn.FileExists()
        && (fn.GetExt() == wxT("html") || fn.GetExt() == wxT("htm")))
        m_InfoFilename = fn.GetFullPath();
      else if (m_config.ODFCheck())
        wxLogWarning(
          _("InfoFilename %s either does not exist or is not a html file"),
          fn.GetFullPath());
    }
  }

  /* load basic organ information */
  cfg.ReadString(CMBSetting, WX_ORGAN, WX_GRANDORGUE_VERSION, false);

  int volume = cfg.ReadInteger(
    CMBSetting, WX_ORGAN, wxT("Volume"), -120, 100, false, m_config.Volume());

  m_SoundEngine.SetVolume(volume > 20 ? 0 : volume);
  m_Temperament
    = cfg.ReadString(CMBSetting, WX_ORGAN, wxT("Temperament"), false);

  // It must be created before GOOrganModel::Load because lots of objects
  // reference to it
  GOOrganModel::SetCombinationController(m_setter);

  GOOrganModel::Load(cfg);

  m_VirtualCouplers.Load(*this, cfg);

  GOOrganModel::LoadCmbButtons(cfg);

  m_DivisionalSetter = new GODivisionalSetter(this, m_setter->GetState());
  m_elementcreators.push_back(m_DivisionalSetter);
  m_AudioRecorder = new GOAudioRecorder(this);
  m_MidiRecorder = new GOMidiRecorder(this);
  m_MidiPlayer = new GOMidiPlayer(this);
  m_elementcreators.push_back(m_AudioRecorder);
  m_elementcreators.push_back(m_MidiPlayer);
  m_elementcreators.push_back(m_MidiRecorder);
  m_elementcreators.push_back(new GOMetronome(this));

  for (unsigned i = 0; i < m_elementcreators.size(); i++)
    m_elementcreators[i]->Load(cfg);

  p_OnStateButton = GetButtonControl(GOSetter::KEY_ON_STATE);

  if (p_OnStateButton) {
    // we do not want to send midi events on m_OnStateButton together with
    // other events. They will be sent separately.
    UnRegisterLifecycleListener(p_OnStateButton);
  }

  // Load dialog sizes
  if (GODialogSizeSet::isPresentInCfg(cfg, CMBSetting))
    m_config.m_DialogSizes.Load(cfg, CMBSetting);

  const wxString &organName = GetOrganName();

  GetRootPipeConfigNode().SetName(organName);
  OnIsModifiedChanged(true);
  ReadCombinations(cfg);
  m_setter->OnCombinationsLoaded(GetCombinationsDir(), wxEmptyString);
  ResetOrganModified();

  GOHash hash;
  const auto organNameUtf8 = organName.utf8_str();

  hash.Update(organNameUtf8, strlen(organNameUtf8));
  GOHashType result = hash.getHash();
  m_SampleSetId1 = ((result.hash[0] & 0x7F) << 24)
    | ((result.hash[1] & 0x7F) << 16) | ((result.hash[2] & 0x7F) << 8)
    | (result.hash[3] & 0x7F);
  m_SampleSetId2 = ((result.hash[4] & 0x7F) << 24)
    | ((result.hash[5] & 0x7F) << 16) | ((result.hash[6] & 0x7F) << 8)
    | (result.hash[7] & 0x7F);
}

void GOOrganController::LoadOrganGuiData(GOConfigReader &cfg) {
  m_IsOrganGuiLoaded = true;

  unsigned NumberOfPanels = cfg.ReadInteger(
    ODFSetting, WX_ORGAN, wxT("NumberOfPanels"), 0, 100, false);

  m_PitchLabel.Load(cfg, wxT("SetterMasterPitch"), _("organ pitch"));
  m_TemperamentLabel.Load(
    cfg, wxT("SetterMasterTemperament"), _("temperament"));
  m_MainWindowData.Load(cfg);

  m_panels.resize(0);
  m_panels.push_back(new GOGUIPanel(this, GetImageCache(), GetMouseState()));
  m_panels[0]->Load(cfg, wxT(""));

  wxString buffer;

  for (unsigned i = 0; i < NumberOfPanels; i++) {
    buffer.Printf(wxT("Panel%03d"), i + 1);
    m_panels.push_back(new GOGUIPanel(this, GetImageCache(), GetMouseState()));
    m_panels[i + 1]->Load(cfg, buffer);
  }

  m_StopWindowSizeKeeper.Load(cfg, wxT("Stops"));
}

class GOLoadAborted : public std::exception {};

void GOOrganController::LoadObjects(GOProgressMonitor &monitor) {
  m_IsObjectsLoaded = true;

  GOBuffer<char> dummy;

  try {
    bool cache_ok = false;

    dummy.resize(1024 * 1024 * 50);
    ResolveReferences();

    /* Figure out list of pipes to load */
    GOCacheObjectDistributor objectDistributor(GetCacheObjects());

    monitor.Reset(objectDistributor.GetNObjects());

    GOCacheObject *obj = nullptr;

    /* Load pipes */
    if (wxFileExists(m_LoadedOrganInfo.cacheFilePath)) {
      wxFile cache_file(m_LoadedOrganInfo.cacheFilePath);
      GOCache reader(cache_file, m_pool);
      cache_ok = cache_file.IsOpened();

      if (cache_ok) {
        GOHashType hash1, hash2;
        if (!reader.ReadHeader()) {
          cache_ok = false;
          wxLogWarning(_("Cache file had bad magic bypassing cache."));
        }
        hash1 = GenerateCacheHash();
        if (
          !reader.Read(&hash2, sizeof(hash2))
          || memcmp(&hash1, &hash2, sizeof(hash1))) {
          cache_ok = false;
          reader.FreeCacheFile();
          wxLogWarning(_("Cache file had diffent hash bypassing cache."));
        }
      }

      GOCacheObject *obj = nullptr;

      if (cache_ok) {
        while ((obj = objectDistributor.FetchNext())) {
          if (!obj->LoadFromCacheWithoutExc(m_pool, reader)) {
            wxLogWarning(_("Cache load failure: %s"), obj->GetLoadError());
            break;
          }
          if (!monitor.Update(objectDistributor.GetPos(), obj->GetLoadTitle()))
            throw GOLoadAborted(); // Skip the rest of the loading code
        }
        if (!obj)
          m_Cacheable = true;
        else
          // obj points to an object with a load error. We will try to load
          // it from the file later
          cache_ok = false;
      }

      if (!cache_ok && !m_config.ManageCache())
        wxLogWarning(_("The cache for this organ is outdated. Please update "
                       "or delete it."));

      reader.Close();
    }

    if (!cache_ok) {
      GOLoadWorker thisWorker(m_FileStore, m_pool, objectDistributor);
      ptr_vector<GOLoadThread> threads;

      // Create and run additional worker threads
      for (unsigned i = 0; i < m_config.LoadConcurrency(); i++)
        threads.push_back(
          new GOLoadThread(m_FileStore, m_pool, objectDistributor));
      for (unsigned i = 0; i < threads.size(); i++)
        threads[i]->Run();

      // try to load the object that we could not load from cache
      if (obj)
        thisWorker.LoadObjectNoExc(obj);

      while (thisWorker.LoadNextObject(obj))
        // show the progress and process possible Cancel
        if (!monitor.Update(objectDistributor.GetPos(), obj->GetLoadTitle()))
          throw GOLoadAborted(); // skip the rest of loading code
      // rethrow exception if any occurred in thisWorker.LoadNextObject
      bool wereExceptions = thisWorker.WereExceptions();

      for (unsigned i = 0; i < threads.size(); i++)
        wereExceptions |= threads[i]->CheckExceptions();
      if (wereExceptions) {
        for (auto obj : GetCacheObjects()) {
          if (!obj->IsReady())
            wxLogError(obj->GetLoadError());
        }
        GOMessageBox(
          _("There are errors while loading the organ. See Log Messages."),
          _("Load error"),
          wxOK | wxICON_ERROR,
          NULL);
      } else {
        if (objectDistributor.IsComplete())
          m_Cacheable = true;
        if (m_config.ManageCache() && m_Cacheable)
          UpdateCache(m_config.CompressCache(), monitor);
      }

      // Despite a possible exception automatic calling ~GOLoadThread from
      // ~ptr_vector stops all additional worker threads
    }
  } catch (const GOOutOfMemory &e) {
    GOMessageBox(
      _("Out of memory - only parts of the organ are loaded. Please "
        "reduce memory footprint via the sample loading settings."),
      _("Load error"),
      wxOK | wxICON_ERROR,
      NULL);
  } catch (const GOLoadAborted &) {
    GOMessageBox(
      _("Load aborted by the user - only parts of the organ are loaded."),
      _("Load error"),
      wxOK | wxICON_ERROR,
      NULL);
  }
}

wxString GOOrganController::Load(
  const GOOrgan &organ,
  const wxString &file2,
  bool isGuiOnly,
  GOProgressMonitor &monitor) {
  wxString errMsg;

  try {
    GOOrganReader organReader(m_config, organ, file2, m_FileStore, monitor);

    m_ConfiguredOrgan = organ;
    m_LoadedOrganInfo = organReader.GetLoadedOrganInfo();
    m_Cacheable = false;

    LoadOrganCoreData(organReader.GetConfigReader());
    OnLoad(organReader.GetConfigReader());
    organReader.ReportUnused();

    if (!isGuiOnly)
      LoadObjects(monitor);
  } catch (const wxString &error_) {
    errMsg = error_;
  } catch (const std::exception &e) {
    errMsg = e.what();
  } catch (...) { // We must not allow unhandled exceptions here
    errMsg.Printf("Unknown exception");
  }
  m_FileStore.CloseArchives();
  if (errMsg.IsEmpty())
    SetTemperament(m_Temperament);
  return errMsg;
}

// const wxString &WX_CMB = wxT(".cmb");
const wxString &WX_YAML = wxT("yaml");
const wxString WX_GRANDORGUE_COMBINATIONS = "GrandOrgue Combinations";

wxString GOOrganController::ExportCombination(const wxString &fileName) {
  GOYamlModel::Out yamlOut(GetOrganName(), WX_GRANDORGUE_COMBINATIONS);

  yamlOut << *m_setter;
  yamlOut << *m_DivisionalSetter;

  const wxString errMsg = yamlOut.writeTo(fileName);

  m_setter->OnCombinationsSaved(fileName);
  return errMsg;
}

void GOOrganController::LoadCombination(const wxString &file) {
  wxString errMsg;
  const wxFileName fileName(file);

  try {
    const wxString fileExt = fileName.GetExt();

    if (fileExt == WX_YAML) {
      GOYamlModel::In inYaml(GetOrganName(), file, WX_GRANDORGUE_COMBINATIONS);

      if (is_to_import_to_this_organ(
            GetOrganName(),
            WX_GRANDORGUE_COMBINATIONS,
            file,
            inYaml.GetFileOrganName())) {
        inYaml >> *m_setter;
        inYaml >> *m_DivisionalSetter;
        m_setter->OnCombinationsLoaded(fileName.GetPath(), file);
      }
    } else {
      GOConfigFileReader odf_ini_file;

      if (!odf_ini_file.Read(file))
        throw wxString::Format(_("Unable to read '%s'"), file.c_str());

      GOConfigReaderDB ini;
      ini.ReadData(odf_ini_file, CMBSetting, false);
      GOConfigReader cfg(ini);
      wxString fileOrganName
        = cfg.ReadString(CMBSetting, WX_ORGAN, wxT("ChurchName"));

      if (is_to_import_to_this_organ(
            GetOrganName(), wxT("Organ Settings"), file, fileOrganName)) {
        wxString hash = odf_ini_file.getEntry(WX_ORGAN, wxT("ODFHash"));
        if (hash != wxEmptyString)
          if (hash != m_LoadedOrganInfo.odfHash) {
            wxLogWarning(_(
              "The combination file does not exactly match the current ODF."));
          }
        /* skip informational items */
        cfg.ReadString(CMBSetting, WX_ORGAN, wxT("ChurchAddress"), false);
        cfg.ReadString(CMBSetting, WX_ORGAN, wxT("ODFPath"), false);

        ReadCombinations(cfg);
        m_setter->OnCombinationsLoaded(GetCombinationsDir(), wxEmptyString);
      }
    }
    SetOrganModified();
  } catch (const wxString &error) {
    errMsg = error;
  } catch (const std::exception &e) {
    errMsg = e.what();
  } catch (...) { // We must not allow unhandled exceptions here
    errMsg.Printf("Unknown exception");
  }
  if (!errMsg.IsEmpty()) {
    wxLogError(errMsg);
    GOMessageBox(errMsg, _("Load error"), wxOK | wxICON_ERROR, NULL);
  }
}

bool GOOrganController::UpdateCache(bool compress, GOProgressMonitor &monitor) {
  bool isOk = false;

  DeleteCache();

  /* Figure out the list of pipes to save */
  GOCacheObjectDistributor objectDistributor(GetCacheObjects());

  monitor.Setup(objectDistributor.GetNObjects(), _("Creating sample cache"));

  wxFileOutputStream file(m_LoadedOrganInfo.cacheFilePath);

  if (file.IsOk()) {
    GOCacheWriter writer(file, compress);

    /* Save pipes to cache */
    isOk = writer.WriteHeader();

    GOHashType hash = GenerateCacheHash();
    if (!writer.Write(&hash, sizeof(hash)))
      isOk = false;

    while (isOk) {
      GOCacheObject *obj = objectDistributor.FetchNext();

      if (!obj)
        break;
      if (!obj->SaveCache(writer)) {
        isOk = false;
        wxLogError(
          _("Save of %s to the cache failed"), obj->GetLoadTitle().c_str());
      }
      if (!monitor.Update(objectDistributor.GetPos(), obj->GetLoadTitle())) {
        writer.Close();
        DeleteCache();
        isOk = false;
      }
    }
    writer.Close();
    if (!isOk)
      DeleteCache();
  } else
    wxLogError(
      _("Opening the cache file %s failed"), m_LoadedOrganInfo.cacheFilePath);
  return isOk;
}

void GOOrganController::DeleteCache() {
  if (CachePresent())
    wxRemoveFile(m_LoadedOrganInfo.cacheFilePath);
}

void GOOrganController::DeleteSettings() {
  wxRemoveFile(m_LoadedOrganInfo.settingsFilePath);
}

void GOOrganController::SaveOrganCoreData(GOConfigWriter &cfg) {
  m_LoadedOrganInfo.isCustomized = true;
  cfg.WriteString(WX_ORGAN, wxT("ODFHash"), m_LoadedOrganInfo.odfHash);
  cfg.WriteString(WX_ORGAN, wxT("ChurchName"), GetOrganName());
  cfg.WriteString(WX_ORGAN, wxT("ChurchAddress"), m_ChurchAddress);
  cfg.WriteString(WX_ORGAN, wxT("ODFPath"), GetODFFilename());
  if (m_ConfiguredOrgan.GetArchiveID() != wxEmptyString)
    cfg.WriteString(
      WX_ORGAN, wxT("ArchiveID"), m_ConfiguredOrgan.GetArchiveID());
  cfg.WriteString(WX_ORGAN, WX_GRANDORGUE_VERSION, wxT(APP_VERSION));
  cfg.WriteInteger(WX_ORGAN, wxT("Volume"), m_SoundEngine.GetVolume());
  cfg.WriteString(WX_ORGAN, wxT("Temperament"), m_Temperament);

  GOEventDistributor::Save(cfg);
  GetDialogSizeSet().Save(cfg);
  m_VirtualCouplers.Save(cfg);
}

void GOOrganController::OnSave(GOConfigWriter &cfg) {
  m_StopWindowSizeKeeper.Save(cfg);
}

static bool write_config_file(
  GOConfigFileWriter &cfgFile, const wxString &path) {
  wxString tmpName = path + wxT(".new");
  bool isOk = !::wxFileExists(tmpName) || ::wxRemoveFile(tmpName);

  if (!isOk)
    wxLogError(_("Could not write to '%s'"), tmpName);
  else {
    isOk = cfgFile.Save(tmpName);
    if (!isOk)
      wxLogError(_("Could not write to '%s'"), tmpName);
    else
      isOk = go_rename_file(tmpName, path);
  }
  return isOk;
}

bool GOOrganController::Save(const wxString &path) {
  GOConfigFileWriter cfgFile;
  GOConfigWriter cfg(cfgFile, false);

  SaveOrganCoreData(cfg);
  OnSave(cfg);

  bool isOk = write_config_file(
    cfgFile, path.IsEmpty() ? m_LoadedOrganInfo.settingsFilePath : path);
  if (isOk && path.IsEmpty())
    ResetOrganModified();
  return isOk;
}

GOEnclosure *GOOrganController::GetEnclosure(
  const wxString &name, bool is_panel) {
  for (unsigned i = 0; i < m_elementcreators.size(); i++) {
    GOEnclosure *c = m_elementcreators[i]->GetEnclosure(name, is_panel);
    if (c)
      return c;
  }
  return NULL;
}

GOLabelControl *GOOrganController::GetLabel(
  const wxString &name, bool is_panel) {
  for (unsigned i = 0; i < m_elementcreators.size(); i++) {
    GOLabelControl *c = m_elementcreators[i]->GetLabelControl(name, is_panel);
    if (c)
      return c;
  }
  return NULL;
}

GOButtonControl *GOOrganController::GetButtonControl(
  const wxString &name, bool is_panel) {
  for (unsigned i = 0; i < m_elementcreators.size(); i++) {
    GOButtonControl *c = m_elementcreators[i]->GetButtonControl(name, is_panel);
    if (c)
      return c;
  }
  return NULL;
}

const wxString GOOrganController::GetOrganPathInfo() {
  const wxString &archiveID = m_ConfiguredOrgan.GetArchiveID();

  if (archiveID == wxEmptyString)
    return GetODFFilename();
  const GOArchiveFile *archive = m_config.GetArchiveByID(archiveID);
  wxString name = GetODFFilename();
  if (archive)
    name += wxString::Format(
      _(" from '%s' (%s)"), archive->GetName().c_str(), archiveID.c_str());
  else
    name += wxString::Format(_(" from %s"), archiveID.c_str());
  return name;
}

GOOrgan GOOrganController::GetOrganInfo() {
  return GOOrgan(
    GetODFFilename(),
    m_ConfiguredOrgan.GetArchiveID(),
    m_ConfiguredOrgan.GetArchivePath(),
    GetOrganName(),
    GetOrganBuilder(),
    GetRecordingDetails());
}

wxString GOOrganController::GetCombinationsDir() const {
  return wxFileName(m_config.OrganCombinationsPath(), GetOrganName())
    .GetFullPath();
}

void GOOrganController::LoadMIDIFile(
  wxString const &filename,
  const GOConfig::MidiChannelMappingChooser &chooseMapping) {
  const bool hasPedal = GetFirstManualIndex() == 0;
  std::vector<int> midiInputNumbers;

  for (unsigned n = GetODFManualCount(), i = GetFirstManualIndex(); i < n; i++)
    midiInputNumbers.push_back(GetManual(i)->GetMidiInputNumber());

  m_MidiPlayer->LoadFile(filename, hasPedal, midiInputNumbers, chooseMapping);
}

void GOOrganController::PreconfigRecorder() {
  for (unsigned i = GetFirstManualIndex(); i <= GetManualAndPedalCount(); i++) {
    wxString id = wxString::Format(wxT("M%d"), i);
    m_MidiRecorder->PreconfigureMapping(id, false);
  }
}

void GOOrganController::StartOrgan(
  GOSoundSystem &soundSystem, GOMidiSystem &midi) {
  const std::vector<GOSoundOrganEngine::AudioOutputConfig> audioOutputConfigs
    = GOSoundOrganEngine::createAudioOutputConfigs(
      m_config, m_config.GetAudioGroups().size());

  m_SoundEngine.SetFromConfig(m_config);
  m_SoundEngine.BuildEngine(
    audioOutputConfigs,
    soundSystem.GetSamplesPerBuffer(),
    soundSystem.GetSampleRate(),
    soundSystem.GetAudioRecorder());
  m_SoundEngine.StartEngine();
  soundSystem.ConnectToEngine(m_SoundEngine);

  m_midi = &midi;
  m_MidiRecorder->SetOutputDevice(m_config.MidiRecorderOutputDevice());
  m_AudioRecorder->SetAudioRecorder(&soundSystem.GetAudioRecorder());

  m_MidiRecorder->Clear();
  PreconfigRecorder();
  m_MidiRecorder->SetSamplesetId(m_SampleSetId1, m_SampleSetId2);
  PreconfigRecorder();

  m_MidiSamplesetMatch.clear();
  GOOrganModel::SetMidi(&midi, m_MidiRecorder);
  GOOrganModel::GOSoundOrganInterfaceProxy::Connect(
    &m_SoundEngine.GetSamplerPlayer());
  GOEventDistributor::PreparePlayback();

  m_setter->UpdateModified(m_OrganModified);

  GOEventDistributor::StartPlayback();
  GOEventDistributor::PrepareRecording();
  m_MidiPlayer->Setup(&midi);

  // Light the OnState button
  if (p_OnStateButton) {
    p_OnStateButton->PreparePlayback();
    p_OnStateButton->StartPlayback();
    p_OnStateButton->PrepareRecording();
  }
}

void GOOrganController::StopOrgan(GOSoundSystem &soundSystem) {
  GOEventDistributor::AbortPlayback();

  m_MidiPlayer->Cleanup();
  m_MidiRecorder->StopRecording();
  m_AudioRecorder->StopRecording();
  m_AudioRecorder->SetAudioRecorder(NULL);
  if (p_OnStateButton)
    p_OnStateButton->AbortPlayback();
  GOOrganModel::GOSoundOrganInterfaceProxy::Disconnect();
  GOOrganModel::SetMidi(nullptr, nullptr);
  m_midi = NULL;

  soundSystem.DisconnectFromEngine(m_SoundEngine);
  m_SoundEngine.StopEngine();
  m_SoundEngine.DestroyEngine();
}

void GOOrganController::PrepareRecording() {
  m_MidiRecorder->Clear();
  PreconfigRecorder();
  m_MidiRecorder->SetSamplesetId(m_SampleSetId1, m_SampleSetId2);
  PreconfigRecorder();

  // The broadcasted PrepareRecording() below makes every MIDI-sending object
  // resend its current value so that the new recording starts from a known
  // state and can later be played back correctly. SetToSendMidi(false)
  // keeps that resend from reaching external MIDI outputs (physical panels,
  // LCDs, SysEx-driven consoles), which did not actually change and should
  // not be refreshed just because recording started. SetToRecordMidi(true)
  // ensures the resent values are still written into the recording itself.
  GOMidiSendProxy::StateRestorer midiSendStateRestorer(*this);

  SetToSendMidi(false);
  SetToRecordMidi(true);
  GOEventDistributor::PrepareRecording();
}

void GOOrganController::Update() {
  for (unsigned i = 0; i < m_switches.size(); i++)
    m_switches[i]->Update();

  for (unsigned i = m_FirstManual; i < m_manuals.size(); i++)
    m_manuals[i]->Update();

  for (unsigned i = 0; i < m_tremulants.size(); i++)
    m_tremulants[i]->Update();

  for (unsigned i = 0; i < m_DivisionalCoupler.size(); i++)
    m_DivisionalCoupler[i]->Update();

  m_setter->Update();
}

void GOOrganController::ProcessMidi(const GOMidiEvent &event) {
  if (event.GetMidiType() == GOMidiEvent::MIDI_RESET) {
    Reset();
    return;
  }
  while (m_MidiSamplesetMatch.size() < event.GetDevice())
    m_MidiSamplesetMatch.push_back(true);

  if (event.GetMidiType() == GOMidiEvent::MIDI_SYSEX_GO_CLEAR)
    m_MidiSamplesetMatch[event.GetDevice()] = true;
  else if (event.GetMidiType() == GOMidiEvent::MIDI_SYSEX_GO_SAMPLESET) {
    if (
      event.GetKey() == m_SampleSetId1 && event.GetValue() == m_SampleSetId2) {
      m_MidiSamplesetMatch[event.GetDevice()] = true;
    } else {
      m_MidiSamplesetMatch[event.GetDevice()] = false;
      return;
    }
  } else if (event.GetMidiType() == GOMidiEvent::MIDI_SYSEX_GO_SETUP) {
    if (!m_MidiSamplesetMatch[event.GetDevice()])
      return;
  }

  GOEventDistributor::SendMidi(event);
}

void GOOrganController::Reset() {
  for (unsigned l = 0; l < GetSwitchCount(); l++)
    GetSwitch(l)->Reset();
  for (unsigned k = GetFirstManualIndex(); k <= GetManualAndPedalCount(); k++)
    GetManual(k)->Reset();
  for (unsigned l = 0; l < GetTremulantCount(); l++)
    GetTremulant(l)->Reset();
  for (unsigned j = 0; j < GetDivisionalCouplerCount(); j++)
    GetDivisionalCoupler(j)->Reset();
  for (unsigned k = 0; k < GetGeneralCount(); k++)
    GetGeneral(k)->Display(false);
  m_setter->ResetCmbButtons();
}

void GOOrganController::SetTemperament(const GOTemperament &temperament) {
  m_TemperamentLabel.SetContent(temperament.GetTitle());
  for (unsigned k = 0; k < m_ranks.size(); k++)
    m_ranks[k]->SetTemperament(temperament);
}

void GOOrganController::SetTemperament(const wxString &name) {
  const GOTemperament &temperament
    = m_config.GetTemperaments().GetTemperament(name);
  m_Temperament = temperament.GetName();
  SetTemperament(temperament);
}

void GOOrganController::AllNotesOff() {
  for (unsigned k = GetFirstManualIndex(); k <= GetManualAndPedalCount(); k++)
    GetManual(k)->AllNotesOff();
}
