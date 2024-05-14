/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOOrganController.h"

#include <algorithm>
#include <math.h>
#include <wx/datetime.h>
#include <wx/filename.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/txtstrm.h>
#include <wx/wfstream.h>
#include <yaml-cpp/yaml.h>

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
#include "contrib/sha1.h"
#include "control/GOElementCreator.h"
#include "control/GOPushbuttonControl.h"
#include "dialogs/GOProgressDialog.h"
#include "files/GOOpenedFile.h"
#include "files/GOStdFileName.h"
#include "gui/GOGUIBankedGeneralsPanel.h"
#include "gui/GOGUICouplerPanel.h"
#include "gui/GOGUICrescendoPanel.h"
#include "gui/GOGUIDivisionalsPanel.h"
#include "gui/GOGUIFloatingPanel.h"
#include "gui/GOGUIMasterPanel.h"
#include "gui/GOGUIMetronomePanel.h"
#include "gui/GOGUIPanel.h"
#include "gui/GOGUIPanelCreator.h"
#include "gui/GOGUIRecorderPanel.h"
#include "gui/GOGUISequencerPanel.h"
#include "loader/GOLoadThread.h"
#include "loader/GOLoaderFilename.h"
#include "loader/cache/GOCache.h"
#include "loader/cache/GOCacheWriter.h"
#include "midi/GOMidi.h"
#include "midi/GOMidiEvent.h"
#include "midi/GOMidiPlayer.h"
#include "midi/GOMidiRecorder.h"
#include "model/GOCoupler.h"
#include "model/GODivisionalCoupler.h"
#include "model/GOEnclosure.h"
#include "model/GOManual.h"
#include "model/GORank.h"
#include "model/GOSoundingPipe.h"
#include "model/GOSwitch.h"
#include "model/GOTremulant.h"
#include "model/GOWindchest.h"
#include "sound/GOSoundEngine.h"
#include "sound/GOSoundReleaseAlignTable.h"
#include "temperaments/GOTemperament.h"
#include "yaml/go-wx-yaml.h"

#include "go_defs.h"

#include "GOAudioRecorder.h"
#include "GOBuffer.h"
#include "GODocument.h"
#include "GOEvent.h"
#include "GOHash.h"
#include "GOMetronome.h"
#include "GOOrgan.h"
#include "GOPath.h"

static const wxString WX_ORGAN = wxT("Organ");
static const wxString WX_GRANDORGUE_VERSION = wxT("GrandOrgueVersion");

GOOrganController::GOOrganController(
  GOConfig &config, GOMidiDialogCreator *pMidiDialogCreator)
  : GOEventDistributor(this),
    GOOrganModel(config),
    m_config(config),
    m_odf(),
    m_ArchiveID(),
    m_hash(),
    m_FileStore(config),
    m_CacheFilename(),
    m_SettingFilename(),
    m_ODFHash(),
    m_Cacheable(false),
    m_setter(0),
    m_AudioRecorder(NULL),
    m_MidiPlayer(NULL),
    m_MidiRecorder(NULL),
    p_OnStateButton(nullptr),
    m_volume(0),
    m_b_customized(false),
    m_CurrentPitch(999999.0), // for enforcing updating the label first time
    m_OrganModified(false),
    m_ChurchName(),
    m_ChurchAddress(),
    m_OrganBuilder(),
    m_OrganBuildDate(),
    m_OrganComments(),
    m_RecordingDetails(),
    m_InfoFilename(),
    m_panels(),
    m_panelcreators(),
    m_elementcreators(),
    m_soundengine(0),
    m_midi(0),
    m_MidiSamplesetMatch(),
    m_SampleSetId1(0),
    m_SampleSetId2(0),
    m_bitmaps(this),
    m_PitchLabel(this),
    m_TemperamentLabel(this),
    m_MainWindowData(this, wxT("MainWindow")) {
  GOOrganModel::SetMidiDialogCreator(pMidiDialogCreator);
  GOOrganModel::SetModelModificationListener(this);
  m_setter = new GOSetter(this);
  m_pool.SetMemoryLimit(m_config.MemoryLimit() * 1024 * 1024);
}

GOOrganController::~GOOrganController() {
  p_OnStateButton = nullptr;
  m_FileStore.CloseArchives();
  GOEventHandlerList::Cleanup();
  // Just to be sure, that the sound providers are freed before the pool
  m_manuals.clear();
  m_tremulants.clear();
  m_ranks.clear();
  m_VirtualCouplers.Cleanup();
  GOOrganModel::Cleanup();
  GOOrganModel::SetModelModificationListener(nullptr);
  GOOrganModel::SetMidiDialogCreator(nullptr);
  GOOrganModel::SetCombinationController(nullptr);
  m_elementcreators.clear();
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
      m_PitchLabel.SetContent(wxString::Format(_("%f cent"), newPitch));
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

bool GOOrganController::IsCacheable() { return m_Cacheable; }

GOHashType GOOrganController::GenerateCacheHash() {
  GOHash hash;

  UpdateHash(hash);
  hash.Update(sizeof(GOSoundAudioSection));
  hash.Update(sizeof(GOSoundingPipe));
  hash.Update(sizeof(GOSoundReleaseAlignTable));
  hash.Update(BLOCK_HISTORY);
  hash.Update(MAX_READAHEAD);
  hash.Update(SHORT_LOOP_LENGTH);
  GOSoundProvider::UpdateCacheHash(hash);
  hash.Update(sizeof(audio_start_data_segment));
  hash.Update(sizeof(audio_end_data_segment));
  return hash.getHash();
}

void GOOrganController::ReadOrganFile(GOConfigReader &cfg) {
  /* load church info */
  cfg.ReadString(
    ODFSetting, WX_ORGAN, wxT("HauptwerkOrganFileFormatVersion"), false);
  m_ChurchName = cfg.ReadStringTrim(ODFSetting, WX_ORGAN, wxT("ChurchName"));
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
  if (info_filename.IsEmpty()) {
    /* Resolve organ file path */
    fn = GetODFFilename();
    fn.SetExt(wxT("html"));
    if (fn.FileExists() && !m_FileStore.AreArchivesUsed())
      m_InfoFilename = fn.GetFullPath();
    else
      m_InfoFilename = wxEmptyString;
  } else {
    GOLoaderFilename fname;

    fname.Assign(info_filename);
    std::unique_ptr<GOOpenedFile> file = fname.Open(m_FileStore);
    fn = info_filename;
    if (
      file->isValid()
      && (fn.GetExt() == wxT("html") || fn.GetExt() == wxT("htm"))) {
      if (fn.FileExists() && !m_FileStore.AreArchivesUsed())
        m_InfoFilename = fn.GetFullPath();
      else
        m_InfoFilename = wxEmptyString;
    } else {
      m_InfoFilename = wxEmptyString;
      if (m_config.ODFCheck())
        wxLogWarning(_("InfoFilename does not point to a html file"));
    }
  }

  /* load basic organ information */
  unsigned NumberOfPanels = cfg.ReadInteger(
    ODFSetting, WX_ORGAN, wxT("NumberOfPanels"), 0, 100, false);
  cfg.ReadString(CMBSetting, WX_ORGAN, WX_GRANDORGUE_VERSION, false);
  m_volume = cfg.ReadInteger(
    CMBSetting, WX_ORGAN, wxT("Volume"), -120, 100, false, m_config.Volume());
  if (m_volume > 20)
    m_volume = 0;
  m_Temperament
    = cfg.ReadString(CMBSetting, WX_ORGAN, wxT("Temperament"), false);

  // It must be created before GOOrganModel::Load because lots of objects
  // reference to it
  GOOrganModel::SetCombinationController(m_setter);
  m_elementcreators.push_back(m_setter);

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
  m_panelcreators.push_back(new GOGUICouplerPanel(this, m_VirtualCouplers));
  m_panelcreators.push_back(new GOGUIFloatingPanel(this));
  m_panelcreators.push_back(new GOGUIMetronomePanel(this));
  m_panelcreators.push_back(new GOGUICrescendoPanel(this));
  m_panelcreators.push_back(new GOGUIDivisionalsPanel(this));
  m_panelcreators.push_back(new GOGUIBankedGeneralsPanel(this));
  m_panelcreators.push_back(new GOGUISequencerPanel(this));
  m_panelcreators.push_back(new GOGUIMasterPanel(this));
  m_panelcreators.push_back(new GOGUIRecorderPanel(this));

  for (unsigned i = 0; i < m_elementcreators.size(); i++)
    m_elementcreators[i]->Load(cfg);

  p_OnStateButton = GetButtonControl(GOSetter::KEY_ON_STATE);

  if (p_OnStateButton) {
    // we do not want to send midi events on m_OnStateButton together with
    // other events. They will be sent separately.
    UnRegisterSoundStateHandler(p_OnStateButton);
  }

  m_PitchLabel.Load(cfg, wxT("SetterMasterPitch"), _("organ pitch"));
  m_TemperamentLabel.Load(
    cfg, wxT("SetterMasterTemperament"), _("temperament"));
  m_MainWindowData.Load(cfg);

  // Load dialog sizes
  if (GODialogSizeSet::isPresentInCfg(cfg, CMBSetting))
    m_config.m_DialogSizes.Load(cfg, CMBSetting);

  m_panels.resize(0);
  m_panels.push_back(new GOGUIPanel(this));
  m_panels[0]->Load(cfg, wxT(""));

  wxString buffer;

  for (unsigned i = 0; i < NumberOfPanels; i++) {
    buffer.Printf(wxT("Panel%03d"), i + 1);
    m_panels.push_back(new GOGUIPanel(this));
    m_panels[i + 1]->Load(cfg, buffer);
  }

  for (unsigned i = 0; i < m_panelcreators.size(); i++)
    m_panelcreators[i]->CreatePanels(cfg);

  for (unsigned i = 0; i < m_panels.size(); i++)
    m_panels[i]->Layout();

  GetRootPipeConfigNode().SetName(GetChurchName());
  ReadCombinations(cfg);
  m_setter->OnCombinationsLoaded(GetCombinationsDir(), wxEmptyString);

  GOHash hash;
  hash.Update(m_ChurchName.utf8_str(), strlen(m_ChurchName.utf8_str()));
  GOHashType result = hash.getHash();
  m_SampleSetId1 = ((result.hash[0] & 0x7F) << 24)
    | ((result.hash[1] & 0x7F) << 16) | ((result.hash[2] & 0x7F) << 8)
    | (result.hash[3] & 0x7F);
  m_SampleSetId2 = ((result.hash[4] & 0x7F) << 24)
    | ((result.hash[5] & 0x7F) << 16) | ((result.hash[6] & 0x7F) << 8)
    | (result.hash[7] & 0x7F);
}

wxString GOOrganController::GetOrganHash() { return m_hash; }

wxString GOOrganController::GenerateSettingFileName() {
  return m_config.OrganSettingsPath() + wxFileName::GetPathSeparator()
    + GOStdFileName::composeSettingFileName(GetOrganHash(), m_config.Preset());
}

wxString GOOrganController::GenerateCacheFileName() {
  return m_config.OrganCachePath() + wxFileName::GetPathSeparator()
    + GOStdFileName::composeCacheFileName(GetOrganHash(), m_config.Preset());
}

class GOLoadAborted : public std::exception {};

wxString GOOrganController::Load(
  GOProgressDialog *dlg,
  const GOOrgan &organ,
  const wxString &file2,
  bool isGuiOnly) {
  GOLoaderFilename odf_name;

  m_ArchiveID = organ.GetArchiveID();
  if (m_ArchiveID != wxEmptyString) {
    dlg->Setup(1, _("Loading sample set"), _("Parsing organ packages"));

    wxString errMsg;

    if (!m_FileStore.LoadArchives(
          m_config,
          m_config.OrganCachePath(),
          organ.GetArchiveID(),
          organ.GetArchivePath(),
          errMsg))
      return errMsg;
    m_ArchivePath = organ.GetArchivePath();
    m_odf = organ.GetODFPath();
    odf_name.Assign(m_odf);
  } else {
    wxString file = organ.GetODFPath();
    m_odf = GONormalizePath(file);
    odf_name.AssignAbsolute(m_odf);
    m_FileStore.SetDirectory(GOGetPath(m_odf));
  }
  m_hash = organ.GetOrganHash();
  dlg->Setup(
    1, _("Loading sample set"), _("Parsing sample set definition file"));
  m_SettingFilename = GenerateSettingFileName();
  m_CacheFilename = GenerateCacheFileName();
  m_Cacheable = false;

  wxString errMsg;

  GOConfigFileReader odf_ini_file;

  if (!odf_ini_file.Read(odf_name.Open(m_FileStore).get())) {
    errMsg.Printf(_("Unable to read '%s'"), odf_name.GetPath().c_str());
    return errMsg;
  }

  m_ODFHash = odf_ini_file.GetHash();
  m_b_customized = false;
  GOConfigReaderDB ini(m_config.ODFCheck());
  ini.ReadData(odf_ini_file, ODFSetting, false);

  wxString setting_file = file2;
  bool can_read_cmb_directly = true;

  if (setting_file.IsEmpty()) {
    if (wxFileExists(m_SettingFilename)) {
      setting_file = m_SettingFilename;
      m_b_customized = true;
    } else {
      wxString bundledSettingsFile = m_odf.BeforeLast('.') + wxT(".cmb");
      if (!m_FileStore.AreArchivesUsed()) {
        if (wxFileExists(bundledSettingsFile)) {
          setting_file = bundledSettingsFile;
          m_b_customized = true;
        }
      } else {
        if (m_FileStore.FindArchiveContaining(m_odf)->containsFile(
              bundledSettingsFile)) {
          setting_file = bundledSettingsFile;
          m_b_customized = true;
          can_read_cmb_directly = false;
        }
      }
    }
  }

  if (!setting_file.IsEmpty()) {
    GOConfigFileReader extra_odf_config;
    if (can_read_cmb_directly) {
      if (!extra_odf_config.Read(setting_file)) {
        errMsg.Printf(_("Unable to read '%s'"), setting_file.c_str());
        return errMsg;
      }
    } else {
      if (!extra_odf_config.Read(
            m_FileStore.FindArchiveContaining(m_odf)->OpenFile(setting_file))) {
        errMsg.Printf(_("Unable to read '%s'"), setting_file.c_str());
        return errMsg;
      }
    }

    if (
      odf_ini_file.getEntry(WX_ORGAN, wxT("ChurchName")).Trim()
      != extra_odf_config.getEntry(WX_ORGAN, wxT("ChurchName")).Trim())
      wxLogWarning(
        _("This .cmb file was originally created for:\n%s"),
        extra_odf_config.getEntry(WX_ORGAN, wxT("ChurchName")).c_str());

    ini.ReadData(extra_odf_config, CMBSetting, false);
    wxString hash = extra_odf_config.getEntry(WX_ORGAN, wxT("ODFHash"));
    if (hash != wxEmptyString)
      if (hash != m_ODFHash) {
        if (
          wxMessageBox(
            _("The .cmb file does not exactly match the current "
              "ODF. Importing it can cause various problems. "
              "Should it really be imported?"),
            _("Import"),
            wxYES_NO,
            NULL)
          == wxNO) {
          ini.ClearCMB();
        }
      }
  } else {
    bool old_go_settings = ini.ReadData(odf_ini_file, CMBSetting, true);
    if (old_go_settings)
      if (
        wxMessageBox(
          _("The ODF contains GrandOrgue 0.2 styled saved "
            "settings. Should they be imported?"),
          _("Import"),
          wxYES_NO,
          NULL)
        == wxNO) {
        ini.ClearCMB();
      }
  }

  try {
    GOConfigReader cfg(ini, m_config.ODFCheck(), m_config.ODFHw1Check());
    /* skip informational items */
    cfg.ReadString(CMBSetting, WX_ORGAN, wxT("ChurchName"), false);
    cfg.ReadString(CMBSetting, WX_ORGAN, wxT("ChurchAddress"), false);
    cfg.ReadString(CMBSetting, WX_ORGAN, wxT("ODFPath"), false);
    cfg.ReadString(CMBSetting, WX_ORGAN, wxT("ODFHash"), false);
    cfg.ReadString(CMBSetting, WX_ORGAN, wxT("ArchiveID"), false);
    ReadOrganFile(cfg);
  } catch (wxString error_) {
    return error_;
  }
  ini.ReportUnused();

  if (!isGuiOnly) {
    GOBuffer<char> dummy;

    try {
      bool cache_ok = false;

      dummy.resize(1024 * 1024 * 50);
      ResolveReferences();

      /* Figure out list of pipes to load */
      GOCacheObjectDistributor objectDistributor(GetCacheObjects());

      dlg->Reset(objectDistributor.GetNObjects());

      GOCacheObject *obj = nullptr;

      /* Load pipes */
      if (wxFileExists(m_CacheFilename)) {
        wxFile cache_file(m_CacheFilename);
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
            if (!dlg->Update(objectDistributor.GetPos(), obj->GetLoadTitle()))
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
          if (!dlg->Update(objectDistributor.GetPos(), obj->GetLoadTitle()))
            throw GOLoadAborted(); // skip the rest of loading code
        // rethrow exception if any occured in thisWorker.LoadNextObject
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
            UpdateCache(dlg, m_config.CompressCache());
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
    } catch (const wxString &error_) {
      errMsg = error_;
    } catch (const std::exception &e) {
      errMsg = e.what();
    } catch (...) { // We must not allow unhandled exceptions here
      errMsg.Printf("Unknown exception");
    }
    dummy.free();
  }

  m_FileStore.CloseArchives();
  if (errMsg.IsEmpty())
    SetTemperament(m_Temperament);
  return errMsg;
}

// const wxString &WX_CMB = wxT(".cmb");
const wxString &WX_YAML = wxT("yaml");
const char *const INFO = "info";
const char *const CONTENT_TYPE = "content-type";
const wxString WX_GRANDORGUE_COMBINATIONS = "GrandOrgue Combinations";
const char *const ORGAN_NAME = "organ-name";
const char *const GRANDORGUE_VERSION = "grandorgue-version";
const char *const SAVED_TIME = "saved_time";

wxString GOOrganController::ExportCombination(const wxString &fileName) {
  wxString errMsg;
  wxFileOutputStream fOS(fileName);

  if (fOS.IsOk()) {
    YAML::Node globalNode(YAML::NodeType::Map);
    YAML::Node infoNode = globalNode[INFO];

    infoNode[CONTENT_TYPE] = WX_GRANDORGUE_COMBINATIONS;
    infoNode[ORGAN_NAME] = m_ChurchName;
    infoNode[GRANDORGUE_VERSION] = APP_VERSION;
    infoNode[SAVED_TIME] = wxDateTime::Now().Format();

    globalNode << *m_setter;
    globalNode << *m_DivisionalSetter;
    YAML::Emitter outYaml;

    outYaml << YAML::BeginDoc << globalNode;

    static const uint8_t utf8bom[] = {0xEF, 0xBB, 0xBF};
    if (
      fOS.WriteAll(utf8bom, sizeof(utf8bom))
      && fOS.WriteAll(outYaml.c_str(), outYaml.size()))
      m_setter->OnCombinationsSaved(fileName);
    else
      errMsg.Printf(
        wxT("Unable to write all the data to the file '%s'"), fileName);
    fOS.Close();
  } else
    errMsg.Printf(wxT("Unable to open the file '%s' for writing"), fileName);
  return errMsg;
}

/**
 * Check the churchName of the imported combination file. If it differs from the
 * current organ m_ChurchName then ask for the user
 * @param churchName the organ the combination file was saved of
 * @return true if the churchNames are the same or the user agree with importing
 *   the combination file
 */
bool GOOrganController::IsToImportCombinationsFor(
  const wxString &fileName, const wxString &churchName) const {
  bool isToImport = true;

  if (churchName != m_ChurchName) {
    wxLogWarning(
      _("This combination file '%s' was originally made for another organ "
        "'%s'"),
      fileName,
      churchName);
    isToImport = wxMessageBox(
                   wxString::Format(
                     _("This combination file '%s' was originally made for "
                       "another organ '%s'. Importing it can cause various "
                       "problems. Should it really be imported?"),
                     fileName,
                     churchName),
                   _("Import Combinations"),
                   wxYES_NO,
                   NULL)
      == wxYES;
  }
  return isToImport;
}

static std::vector<char> load_file_bytes(const wxString &filePath) {
  wxFile file;
  if (!file.Open(filePath)) {
    throw wxString::Format(
      _("Failed to open '%s': %s"), filePath, strerror(file.GetLastError()));
  }
  std::vector<char> content;
  content.reserve(file.Length());
  char buf[8 * 1024]; // 8 KiB
  ssize_t bytesRead;
  while ((bytesRead = file.Read(buf, sizeof(buf))) != 0) {
    if (bytesRead == wxInvalidOffset) {
      throw wxString::Format(
        _("Failed to read '%s': %s"), filePath, strerror(file.GetLastError()));
    }
    content.insert(content.end(), &buf[0], &buf[bytesRead]);
  }
  return content;
}

static wxString load_file_text_with_encoding_detection(
  const wxString &filePath) {
  std::vector<char> content = load_file_bytes(filePath);
  wxBOM detectedBOM = wxConvAuto::DetectBOM(&content[0], content.size());
  if (detectedBOM != wxBOM_None && detectedBOM != wxBOM_Unknown) {
    // We know what encoding was used for that file.
    // wxConvAuto will use BOM to determine encoding and to decode file content.
    // Note: newer GO versions export yaml files with UTF-8-BOM.
    return wxString(&content[0], wxConvAuto(), content.size());
  } else {
    // Use encoding that was used in older GO versions (system default)
    return wxString(&content[0], *wxConvCurrent, content.size());
  }
}

void GOOrganController::LoadCombination(const wxString &file) {
  wxString errMsg;
  const wxFileName fileName(file);

  try {
    const wxString fileExt = fileName.GetExt();

    if (fileExt == WX_YAML) {
      wxString fileContent = load_file_text_with_encoding_detection(file);
      // Note: wxScopedCharBuffer may point to internals of wxString above
      // fileContent must not be destructed while fileContentInUtf8 is in use
      wxScopedCharBuffer fileContentInUtf8 = fileContent.utf8_str();
      YAML::Node cmbNode = YAML::Load(fileContentInUtf8.data());
      YAML::Node cmbInfoNode = cmbNode[INFO];
      const wxString contentType = cmbInfoNode[CONTENT_TYPE].as<wxString>();

      if (contentType != WX_GRANDORGUE_COMBINATIONS)
        throw wxString::Format(
          _("The file '%s' is not a GrandOrgue Combination file"), file);
      if (IsToImportCombinationsFor(
            file, cmbInfoNode[ORGAN_NAME].as<wxString>())) {
        cmbNode >> *m_setter;
        cmbNode >> *m_DivisionalSetter;
        m_setter->OnCombinationsLoaded(fileName.GetPath(), file);
      }
    } else {
      GOConfigFileReader odf_ini_file;

      if (!odf_ini_file.Read(file))
        throw wxString::Format(_("Unable to read '%s'"), file.c_str());

      GOConfigReaderDB ini;
      ini.ReadData(odf_ini_file, CMBSetting, false);
      GOConfigReader cfg(ini);

      wxString church_name
        = cfg.ReadString(CMBSetting, WX_ORGAN, wxT("ChurchName"));
      if (!IsToImportCombinationsFor(file, church_name))
        return;

      wxString hash = odf_ini_file.getEntry(WX_ORGAN, wxT("ODFHash"));
      if (hash != wxEmptyString)
        if (hash != m_ODFHash) {
          wxLogWarning(
            _("The combination file does not exactly match the current ODF."));
        }
      /* skip informational items */
      cfg.ReadString(CMBSetting, WX_ORGAN, wxT("ChurchAddress"), false);
      cfg.ReadString(CMBSetting, WX_ORGAN, wxT("ODFPath"), false);

      ReadCombinations(cfg);
      m_setter->OnCombinationsLoaded(GetCombinationsDir(), wxEmptyString);
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

bool GOOrganController::CachePresent() { return wxFileExists(m_CacheFilename); }

bool GOOrganController::UpdateCache(GOProgressDialog *dlg, bool compress) {
  DeleteCache();

  /* Figure out the list of pipes to save */
  GOCacheObjectDistributor objectDistributor(GetCacheObjects());

  dlg->Setup(objectDistributor.GetNObjects(), _("Creating sample cache"));

  wxFileOutputStream file(m_CacheFilename);
  GOCacheWriter writer(file, compress);

  /* Save pipes to cache */
  bool cache_save_ok = writer.WriteHeader();

  GOHashType hash = GenerateCacheHash();
  if (!writer.Write(&hash, sizeof(hash)))
    cache_save_ok = false;

  for (unsigned i = 0; cache_save_ok; i++) {
    GOCacheObject *obj = objectDistributor.FetchNext();

    if (!obj)
      break;
    if (!obj->SaveCache(writer)) {
      cache_save_ok = false;
      wxLogError(
        _("Save of %s to the cache failed"), obj->GetLoadTitle().c_str());
    }
    if (!dlg->Update(objectDistributor.GetPos(), obj->GetLoadTitle())) {
      writer.Close();
      DeleteCache();
      return false;
    }
  }

  writer.Close();
  if (!cache_save_ok) {
    DeleteCache();
    return false;
  }
  return true;
}

void GOOrganController::DeleteCache() {
  if (CachePresent())
    wxRemoveFile(m_CacheFilename);
}

void GOOrganController::DeleteSettings() { wxRemoveFile(m_SettingFilename); }

bool GOOrganController::Save() {
  if (!Export(m_SettingFilename))
    return false;
  ResetOrganModified();
  return true;
}

bool GOOrganController::Export(const wxString &cmb) {
  GOConfigFileWriter cfg_file;
  GOConfigWriter cfg(cfg_file, false);

  m_b_customized = true;
  cfg.WriteString(WX_ORGAN, wxT("ODFHash"), m_ODFHash);
  cfg.WriteString(WX_ORGAN, wxT("ChurchName"), m_ChurchName);
  cfg.WriteString(WX_ORGAN, wxT("ChurchAddress"), m_ChurchAddress);
  cfg.WriteString(WX_ORGAN, wxT("ODFPath"), GetODFFilename());
  if (m_ArchiveID != wxEmptyString)
    cfg.WriteString(WX_ORGAN, wxT("ArchiveID"), m_ArchiveID);
  cfg.WriteString(WX_ORGAN, WX_GRANDORGUE_VERSION, wxT(APP_VERSION));
  cfg.WriteInteger(WX_ORGAN, wxT("Volume"), m_volume);
  cfg.WriteString(WX_ORGAN, wxT("Temperament"), m_Temperament);

  GOEventDistributor::Save(cfg);
  GetDialogSizeSet().Save(cfg);
  m_VirtualCouplers.Save(cfg);

  wxString tmp_name = cmb + wxT(".new");

  if (::wxFileExists(tmp_name) && !::wxRemoveFile(tmp_name)) {
    wxLogError(_("Could not write to '%s'"), tmp_name);
    return false;
  }
  if (!cfg_file.Save(tmp_name)) {
    wxLogError(_("Could not write to '%s'"), tmp_name);
    return false;
  }
  if (!GORenameFile(tmp_name, cmb))
    return false;
  return true;
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

// GODocument *GOOrganController::GetDocument() { return m_doc; }

void GOOrganController::SetVolume(int volume) { m_volume = volume; }

int GOOrganController::GetVolume() { return m_volume; }

GOSetter *GOOrganController::GetSetter() { return m_setter; }

GOGUIPanel *GOOrganController::GetPanel(unsigned index) {
  return m_panels[index];
}

unsigned GOOrganController::GetPanelCount() { return m_panels.size(); }

void GOOrganController::AddPanel(GOGUIPanel *panel) {
  m_panels.push_back(panel);
}

const wxString &GOOrganController::GetChurchName() { return m_ChurchName; }

const wxString &GOOrganController::GetChurchAddress() {
  return m_ChurchAddress;
}

const wxString &GOOrganController::GetOrganBuilder() { return m_OrganBuilder; }

const wxString &GOOrganController::GetOrganBuildDate() {
  return m_OrganBuildDate;
}

const wxString &GOOrganController::GetOrganComments() {
  return m_OrganComments;
}

const wxString &GOOrganController::GetRecordingDetails() {
  return m_RecordingDetails;
}

const wxString &GOOrganController::GetInfoFilename() { return m_InfoFilename; }

bool GOOrganController::IsCustomized() { return m_b_customized; }

const wxString GOOrganController::GetODFFilename() { return m_odf; }

const wxString GOOrganController::GetOrganPathInfo() {
  if (m_ArchiveID == wxEmptyString)
    return GetODFFilename();
  const GOArchiveFile *archive = m_config.GetArchiveByID(m_ArchiveID);
  wxString name = GetODFFilename();
  if (archive)
    name += wxString::Format(
      _(" from '%s' (%s)"), archive->GetName().c_str(), m_ArchiveID.c_str());
  else
    name += wxString::Format(_(" from %s"), m_ArchiveID.c_str());
  return name;
}

GOOrgan GOOrganController::GetOrganInfo() {
  return GOOrgan(
    GetODFFilename(),
    m_ArchiveID,
    m_ArchivePath,
    GetChurchName(),
    GetOrganBuilder(),
    GetRecordingDetails());
}

const wxString GOOrganController::GetSettingFilename() {
  return m_SettingFilename;
}

const wxString GOOrganController::GetCacheFilename() { return m_CacheFilename; }

wxString GOOrganController::GetCombinationsDir() const {
  return wxFileName(m_config.OrganCombinationsPath(), m_ChurchName)
    .GetFullPath();
}

GOMemoryPool &GOOrganController::GetMemoryPool() { return m_pool; }

GOConfig &GOOrganController::GetSettings() { return m_config; }

GOBitmapCache &GOOrganController::GetBitmapCache() { return m_bitmaps; }

GOMidi *GOOrganController::GetMidi() { return m_midi; }

void GOOrganController::LoadMIDIFile(wxString const &filename) {
  m_MidiPlayer->LoadFile(
    filename, GetODFManualCount() - 1, GetFirstManualIndex() == 0);
}

void GOOrganController::Abort() {
  m_soundengine = NULL;

  GOEventDistributor::AbortPlayback();

  m_MidiPlayer->StopPlaying();
  m_MidiRecorder->StopRecording();
  m_AudioRecorder->StopRecording();
  m_AudioRecorder->SetAudioRecorder(NULL);
  if (p_OnStateButton)
    p_OnStateButton->AbortPlaybackExt();
  GOOrganModel::SetMidi(nullptr, nullptr);
  m_midi = NULL;
}

void GOOrganController::PreconfigRecorder() {
  for (unsigned i = GetFirstManualIndex(); i <= GetManualAndPedalCount(); i++) {
    wxString id = wxString::Format(wxT("M%d"), i);
    m_MidiRecorder->PreconfigureMapping(id, false);
  }
}

void GOOrganController::PreparePlayback(
  GOSoundEngine *engine, GOMidi *midi, GOSoundRecorder *recorder) {
  m_soundengine = engine;
  m_midi = midi;
  m_MidiRecorder->SetOutputDevice(m_config.MidiRecorderOutputDevice());
  m_AudioRecorder->SetAudioRecorder(recorder);

  m_MidiRecorder->Clear();
  PreconfigRecorder();
  m_MidiRecorder->SetSamplesetId(m_SampleSetId1, m_SampleSetId2);
  PreconfigRecorder();

  m_MidiSamplesetMatch.clear();
  GOOrganModel::SetMidi(midi, m_MidiRecorder);
  GOEventDistributor::PreparePlayback(engine);

  m_setter->UpdateModified(m_OrganModified);

  GOEventDistributor::StartPlayback();
  GOEventDistributor::PrepareRecording();

  // Light the OnState button
  if (p_OnStateButton) {
    p_OnStateButton->PreparePlaybackExt(engine);
    p_OnStateButton->StartPlaybackExt();
    p_OnStateButton->PrepareRecordingExt();
  }
}

void GOOrganController::PrepareRecording() {
  m_MidiRecorder->Clear();
  PreconfigRecorder();
  m_MidiRecorder->SetSamplesetId(m_SampleSetId1, m_SampleSetId2);
  PreconfigRecorder();

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
  m_TemperamentLabel.SetContent(wxGetTranslation(temperament.GetName()));
  for (unsigned k = 0; k < m_ranks.size(); k++)
    m_ranks[k]->SetTemperament(temperament);
}

void GOOrganController::SetTemperament(wxString name) {
  const GOTemperament &temperament
    = m_config.GetTemperaments().GetTemperament(name);
  m_Temperament = temperament.GetName();
  SetTemperament(temperament);
}

wxString GOOrganController::GetTemperament() { return m_Temperament; }

void GOOrganController::AllNotesOff() {
  for (unsigned k = GetFirstManualIndex(); k <= GetManualAndPedalCount(); k++)
    GetManual(k)->AllNotesOff();
}

GOLabelControl *GOOrganController::GetPitchLabel() { return &m_PitchLabel; }

GOLabelControl *GOOrganController::GetTemperamentLabel() {
  return &m_TemperamentLabel;
}

GOMainWindowData *GOOrganController::GetMainWindowData() {
  return &m_MainWindowData;
}
