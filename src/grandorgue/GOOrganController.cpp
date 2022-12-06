/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOOrganController.h"

#include <math.h>
#include <wx/filename.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/stream.h>
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
#include "contrib/sha1.h"
#include "control/GOElementCreator.h"
#include "control/GOPushbuttonControl.h"
#include "dialogs/GOProgressDialog.h"
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
#include "midi/GOMidi.h"
#include "midi/GOMidiEvent.h"
#include "midi/GOMidiPlayer.h"
#include "midi/GOMidiRecorder.h"
#include "model/GOEnclosure.h"
#include "model/GOManual.h"
#include "model/GORank.h"
#include "model/GOSoundingPipe.h"
#include "model/GOSwitch.h"
#include "model/GOTremulant.h"
#include "model/GOWindchest.h"
#include "sound/GOSoundEngine.h"
#include "temperaments/GOTemperament.h"

#include "GOAudioRecorder.h"
#include "GOBuffer.h"
#include "GOCache.h"
#include "GOCacheWriter.h"
#include "GOCoupler.h"
#include "GODivisionalCoupler.h"
#include "GODocument.h"
#include "GOEvent.h"
#include "GOFile.h"
#include "GOHash.h"
#include "GOMetronome.h"
#include "GOOrgan.h"
#include "GOPath.h"
#include "GOReleaseAlignTable.h"

GOOrganController::GOOrganController(GODocument *doc, GOConfig &settings)
  : GOEventDistributor(this),
    GOOrganModel(settings),
    m_doc(doc),
    m_odf(),
    m_ArchiveID(),
    m_hash(),
    m_FileStore(settings),
    m_CacheFilename(),
    m_SettingFilename(),
    m_ODFHash(),
    m_Cacheable(false),
    m_setter(0),
    m_AudioRecorder(NULL),
    m_MidiPlayer(NULL),
    m_MidiRecorder(NULL),
    m_volume(0),
    m_IgnorePitch(false),
    m_b_customized(false),
    m_OrganModified(false),
    m_DivisionalsStoreIntermanualCouplers(false),
    m_DivisionalsStoreIntramanualCouplers(false),
    m_DivisionalsStoreTremulants(false),
    m_GeneralsStoreDivisionalCouplers(false),
    m_CombinationsStoreNonDisplayedDrawstops(false),
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
    m_UsedSections(),
    m_soundengine(0),
    m_midi(0),
    m_MidiSamplesetMatch(),
    m_SampleSetId1(0),
    m_SampleSetId2(0),
    m_bitmaps(this),
    m_PipeConfig(NULL, this, this),
    m_config(settings),
    m_GeneralTemplate(this),
    m_PitchLabel(this),
    m_TemperamentLabel(this),
    m_MainWindowData(this, wxT("MainWindow")) {
  GOOrganModel::SetModificationListener(this);
  m_pool.SetMemoryLimit(m_config.MemoryLimit() * 1024 * 1024);
}

GOOrganController::~GOOrganController() {
  m_FileStore.CloseArchives();
  GOEventHandlerList::Cleanup();
  // Just to be sure, that the sound providers are freed before the pool
  m_manuals.clear();
  m_tremulants.clear();
  m_ranks.clear();
  GOOrganModel::SetModificationListener(nullptr);
}

void GOOrganController::SetOrganModified(bool modified) {
  if (modified != m_OrganModified) {
    m_OrganModified = modified;
    m_setter->UpdateModified(modified);
  }
}

void GOOrganController::OnIsModifiedChanged(bool modified) {
  if (modified) // If the organ model modified then the organ is also modified
    SetOrganModified(true);
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
  hash.Update(sizeof(GOAudioSection));
  hash.Update(sizeof(GOSoundingPipe));
  hash.Update(sizeof(GOReleaseAlignTable));
  hash.Update(BLOCK_HISTORY);
  hash.Update(MAX_READAHEAD);
  hash.Update(SHORT_LOOP_LENGTH);
  hash.Update(sizeof(attack_section_info));
  hash.Update(sizeof(release_section_info));
  hash.Update(sizeof(audio_start_data_segment));
  hash.Update(sizeof(audio_end_data_segment));

  return hash.getHash();
}

void GOOrganController::ReadOrganFile(GOConfigReader &cfg) {
  wxString group = wxT("Organ");

  /* load church info */
  cfg.ReadString(
    ODFSetting, group, wxT("HauptwerkOrganFileFormatVersion"), false);
  m_ChurchName = cfg.ReadStringTrim(ODFSetting, group, wxT("ChurchName"));
  m_ChurchAddress = cfg.ReadString(ODFSetting, group, wxT("ChurchAddress"));
  m_OrganBuilder
    = cfg.ReadString(ODFSetting, group, wxT("OrganBuilder"), false);
  m_OrganBuildDate
    = cfg.ReadString(ODFSetting, group, wxT("OrganBuildDate"), false);
  m_OrganComments
    = cfg.ReadString(ODFSetting, group, wxT("OrganComments"), false);
  m_RecordingDetails
    = cfg.ReadString(ODFSetting, group, wxT("RecordingDetails"), false);
  wxString info_filename
    = cfg.ReadFileName(ODFSetting, group, wxT("InfoFilename"), false);
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

    fname.Assign(m_FileStore, info_filename);
    std::unique_ptr<GOFile> file = fname.Open();
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
  unsigned NumberOfPanels
    = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfPanels"), 0, 100, false);
  m_PipeConfig.Load(cfg, group, wxEmptyString);
  m_DivisionalsStoreIntermanualCouplers = cfg.ReadBoolean(
    ODFSetting, group, wxT("DivisionalsStoreIntermanualCouplers"));
  m_DivisionalsStoreIntramanualCouplers = cfg.ReadBoolean(
    ODFSetting, group, wxT("DivisionalsStoreIntramanualCouplers"));
  m_DivisionalsStoreTremulants
    = cfg.ReadBoolean(ODFSetting, group, wxT("DivisionalsStoreTremulants"));
  m_GeneralsStoreDivisionalCouplers = cfg.ReadBoolean(
    ODFSetting, group, wxT("GeneralsStoreDivisionalCouplers"));
  m_CombinationsStoreNonDisplayedDrawstops = cfg.ReadBoolean(
    ODFSetting,
    group,
    wxT("CombinationsStoreNonDisplayedDrawstops"),
    false,
    true);
  m_volume = cfg.ReadInteger(
    CMBSetting, group, wxT("Volume"), -120, 100, false, m_config.Volume());
  if (m_volume > 20)
    m_volume = 0;
  m_Temperament = cfg.ReadString(CMBSetting, group, wxT("Temperament"), false);
  m_IgnorePitch
    = cfg.ReadBoolean(CMBSetting, group, wxT("IgnorePitch"), false, false);
  m_releaseTail = (unsigned)cfg.ReadInteger(
    CMBSetting,
    group,
    wxT("ReleaseTail"),
    0,
    3000,
    false,
    m_config.ReleaseLength());

  GOOrganModel::Load(cfg, this);
  wxString buffer;

  for (unsigned i = 0; i < m_enclosures.size(); i++)
    m_enclosures[i]->SetElementID(
      GetRecorderElementID(wxString::Format(wxT("E%d"), i)));

  for (unsigned i = 0; i < m_switches.size(); i++)
    m_switches[i]->SetElementID(
      GetRecorderElementID(wxString::Format(wxT("S%d"), i)));

  for (unsigned i = 0; i < m_tremulants.size(); i++)
    m_tremulants[i]->SetElementID(
      GetRecorderElementID(wxString::Format(wxT("T%d"), i)));

  m_setter = new GOSetter(this);
  m_elementcreators.push_back(m_setter);
  m_DivisionalSetter = new GODivisionalSetter(this);
  m_elementcreators.push_back(m_DivisionalSetter);
  m_AudioRecorder = new GOAudioRecorder(this);
  m_MidiRecorder = new GOMidiRecorder(this);
  m_MidiPlayer = new GOMidiPlayer(this);
  m_elementcreators.push_back(m_AudioRecorder);
  m_elementcreators.push_back(m_MidiPlayer);
  m_elementcreators.push_back(m_MidiRecorder);
  m_elementcreators.push_back(new GOMetronome(this));
  m_panelcreators.push_back(new GOGUICouplerPanel(this));
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

  m_PitchLabel.Load(cfg, wxT("SetterMasterPitch"), _("organ pitch"));
  m_TemperamentLabel.Load(
    cfg, wxT("SetterMasterTemperament"), _("temperament"));
  m_MainWindowData.Load(cfg);

  m_panels.resize(0);
  m_panels.push_back(new GOGUIPanel(this));
  m_panels[0]->Load(cfg, wxT(""));

  for (unsigned i = 0; i < NumberOfPanels; i++) {
    buffer.Printf(wxT("Panel%03d"), i + 1);
    m_panels.push_back(new GOGUIPanel(this));
    m_panels[i + 1]->Load(cfg, buffer);
  }

  for (unsigned i = 0; i < m_panelcreators.size(); i++)
    m_panelcreators[i]->CreatePanels(cfg);

  for (unsigned i = 0; i < m_panels.size(); i++)
    m_panels[i]->Layout();

  m_GeneralTemplate.InitGeneral();
  for (unsigned i = m_FirstManual; i < m_manuals.size(); i++)
    m_manuals[i]->GetDivisionalTemplate().InitDivisional(i);

  m_PipeConfig.SetName(GetChurchName());
  ReadCombinations(cfg);

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

wxString GOOrganController::Load(
  GOProgressDialog *dlg, const GOOrgan &organ, const wxString &file2) {
  GOLoaderFilename odf_name;

  m_ArchiveID = organ.GetArchiveID();
  if (m_ArchiveID != wxEmptyString) {
    dlg->Setup(1, _("Loading sample set"), _("Parsing organ packages"));

    wxString errMsg;

    if (!m_FileStore.LoadArchives(
          m_config, m_config.OrganCachePath(), organ.GetArchiveID(), errMsg))
      return errMsg;

    m_odf = organ.GetODFPath();
    odf_name.Assign(m_FileStore, m_odf);
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

  GOConfigFileReader odf_ini_file;

  if (!odf_ini_file.Read(odf_name.Open().get())) {
    wxString error;
    error.Printf(_("Unable to read '%s'"), odf_name.GetTitle().c_str());
    return error;
  }

  m_ODFHash = odf_ini_file.GetHash();
  wxString error = wxT("!");
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
        error.Printf(_("Unable to read '%s'"), setting_file.c_str());
        return error;
      }
    } else {
      if (!extra_odf_config.Read(
            m_FileStore.FindArchiveContaining(m_odf)->OpenFile(setting_file))) {
        error.Printf(_("Unable to read '%s'"), setting_file.c_str());
        return error;
      }
    }

    if (
      odf_ini_file.getEntry(wxT("Organ"), wxT("ChurchName")).Trim()
      != extra_odf_config.getEntry(wxT("Organ"), wxT("ChurchName")).Trim())
      wxLogWarning(
        _("This .cmb file was originally created for:\n%s"),
        extra_odf_config.getEntry(wxT("Organ"), wxT("ChurchName")).c_str());

    ini.ReadData(extra_odf_config, CMBSetting, false);
    wxString hash = extra_odf_config.getEntry(wxT("Organ"), wxT("ODFHash"));
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
    GOConfigReader cfg(ini, m_config.ODFCheck());
    /* skip informational items */
    cfg.ReadString(CMBSetting, wxT("Organ"), wxT("ChurchName"), false);
    cfg.ReadString(CMBSetting, wxT("Organ"), wxT("ChurchAddress"), false);
    cfg.ReadString(CMBSetting, wxT("Organ"), wxT("ODFPath"), false);
    cfg.ReadString(CMBSetting, wxT("Organ"), wxT("ODFHash"), false);
    cfg.ReadString(CMBSetting, wxT("Organ"), wxT("ArchiveID"), false);
    ReadOrganFile(cfg);
  } catch (wxString error_) {
    return error_;
  }
  ini.ReportUnused();

  GOBuffer<char> dummy;

  try {
    dummy.resize(1024 * 1024 * 50);

    wxString load_error;
    bool cache_ok = false;

    ResolveReferences();

    /* Figure out list of pipes to load */
    GOCacheObjectDistributor objectDistributor(GetCacheObjects());

    dlg->Reset(objectDistributor.GetNObjects());

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

      if (cache_ok) {
        try {
          while (true) {
            GOCacheObject *obj = objectDistributor.fetchNext();

            if (!obj)
              break;
            if (!obj->LoadCache(m_pool, reader)) {
              cache_ok = false;
              wxLogError(
                _("Cache load failure: Failed to read %s from cache."),
                obj->GetLoadTitle().c_str());
              break;
            }
            if (!dlg->Update(objectDistributor.GetPos(), obj->GetLoadTitle())) {
              dummy.free();
              SetTemperament(m_Temperament);
              GOMessageBox(
                _("Load aborted by the user - only parts of the "
                  "organ are loaded."),
                _("Load error"),
                wxOK | wxICON_ERROR,
                NULL);
              m_FileStore.CloseArchives();
              return wxEmptyString;
            }
          }
          if (objectDistributor.IsComplete())
            m_Cacheable = true;
        } catch (wxString msg) {
          cache_ok = false;
          wxLogError(_("Cache load failure: %s"), msg.c_str());
        }
      }

      if (!cache_ok && !m_config.ManageCache()) {
        GOMessageBox(
          _("The cache for this organ is outdated. Please update or "
            "delete it."),
          _("Warning"),
          wxOK | wxICON_WARNING,
          NULL);
      }

      reader.Close();
    }

    if (!cache_ok) {
      ptr_vector<GOLoadThread> threads;
      for (unsigned i = 0; i < m_config.LoadConcurrency(); i++)
        threads.push_back(new GOLoadThread(m_pool, objectDistributor));

      for (unsigned i = 0; i < threads.size(); i++)
        threads[i]->Run();

      GOCacheObject *obj;

      while ((obj = objectDistributor.fetchNext())) {
        obj->LoadData(m_pool);
        if (!dlg->Update(objectDistributor.GetPos(), obj->GetLoadTitle())) {
          dummy.free();
          SetTemperament(m_Temperament);
          GOMessageBox(
            _("Load aborted by the user - only parts of the organ "
              "are loaded."),
            _("Load error"),
            wxOK | wxICON_ERROR,
            NULL);
          m_FileStore.CloseArchives();
          return wxEmptyString;
        }
      }

      for (unsigned i = 0; i < threads.size(); i++)
        threads[i]->checkResult();
      if (objectDistributor.IsComplete())
        m_Cacheable = true;
      if (m_config.ManageCache() && m_Cacheable)
        UpdateCache(dlg, m_config.CompressCache());
    }
    SetTemperament(m_Temperament);
  } catch (GOOutOfMemory e) {
    dummy.free();
    GOMessageBox(
      _("Out of memory - only parts of the organ are loaded. Please "
        "reduce memory footprint via the sample loading settings."),
      _("Load error"),
      wxOK | wxICON_ERROR,
      NULL);
    m_FileStore.CloseArchives();
    return wxEmptyString;
  } catch (wxString error_) {
    dummy.free();
    return error_;
  }
  dummy.free();

  m_FileStore.CloseArchives();
  return wxEmptyString;
}

void GOOrganController::LoadCombination(const wxString &file) {
  try {
    GOConfigFileReader odf_ini_file;

    if (!odf_ini_file.Read(file))
      throw wxString::Format(_("Unable to read '%s'"), file.c_str());

    GOConfigReaderDB ini;
    ini.ReadData(odf_ini_file, CMBSetting, false);
    GOConfigReader cfg(ini);

    wxString church_name
      = cfg.ReadString(CMBSetting, wxT("Organ"), wxT("ChurchName"));
    if (church_name != m_ChurchName)
      if (
        wxMessageBox(
          _("This combination file was originally made for "
            "another organ. Importing it can cause various "
            "problems. Should it really be imported?"),
          _("Import"),
          wxYES_NO,
          NULL)
        == wxNO)
        return;

    wxString hash = odf_ini_file.getEntry(wxT("Organ"), wxT("ODFHash"));
    if (hash != wxEmptyString)
      if (hash != m_ODFHash) {
        wxLogWarning(
          _("The combination file does not exactly match the current ODF."));
      }
    /* skip informational items */
    cfg.ReadString(CMBSetting, wxT("Organ"), wxT("ChurchAddress"), false);
    cfg.ReadString(CMBSetting, wxT("Organ"), wxT("ODFPath"), false);

    ReadCombinations(cfg);
  } catch (wxString error) {
    wxLogError(wxT("%s\n"), error.c_str());
    GOMessageBox(error, _("Load error"), wxOK | wxICON_ERROR, NULL);
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
    GOCacheObject *obj = objectDistributor.fetchNext();

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
  wxString fn = cmb;
  wxString tmp_name = fn + wxT(".new");
  wxString buffer;
  bool prefix = false;

  GOConfigFileWriter cfg_file;
  m_b_customized = true;

  GOConfigWriter cfg(cfg_file, prefix);
  cfg.WriteString(wxT("Organ"), wxT("ODFHash"), m_ODFHash);
  cfg.WriteString(wxT("Organ"), wxT("ChurchName"), m_ChurchName);
  cfg.WriteString(wxT("Organ"), wxT("ChurchAddress"), m_ChurchAddress);
  cfg.WriteString(wxT("Organ"), wxT("ODFPath"), GetODFFilename());
  if (m_ArchiveID != wxEmptyString)
    cfg.WriteString(wxT("Organ"), wxT("ArchiveID"), m_ArchiveID);

  cfg.WriteInteger(wxT("Organ"), wxT("Volume"), m_volume);

  cfg.WriteString(wxT("Organ"), wxT("Temperament"), m_Temperament);
  cfg.WriteBoolean(wxT("Organ"), wxT("IgnorePitch"), m_IgnorePitch);
  cfg.WriteInteger(wxT("Organ"), wxT("ReleaseTail"), (int)m_releaseTail);

  GOEventDistributor::Save(cfg);

  if (::wxFileExists(tmp_name) && !::wxRemoveFile(tmp_name)) {
    wxLogError(_("Could not write to '%s'"), tmp_name.c_str());
    return false;
  }
  if (!cfg_file.Save(tmp_name)) {
    wxLogError(_("Could not write to '%s'"), tmp_name.c_str());
    return false;
  }
  if (!GORenameFile(tmp_name, fn))
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

GODocument *GOOrganController::GetDocument() { return m_doc; }

void GOOrganController::SetVolume(int volume) { m_volume = volume; }

int GOOrganController::GetVolume() { return m_volume; }

void GOOrganController::SetIgnorePitch(bool ignore) { m_IgnorePitch = ignore; }

bool GOOrganController::GetIgnorePitch() { return m_IgnorePitch; }

void GOOrganController::SetReleaseTail(unsigned releaseTail) {
  m_releaseTail = releaseTail;
  SetOrganModified();
}

bool GOOrganController::DivisionalsStoreIntermanualCouplers() {
  return m_DivisionalsStoreIntermanualCouplers;
}

bool GOOrganController::DivisionalsStoreIntramanualCouplers() {
  return m_DivisionalsStoreIntramanualCouplers;
}

bool GOOrganController::DivisionalsStoreTremulants() {
  return m_DivisionalsStoreTremulants;
}

bool GOOrganController::CombinationsStoreNonDisplayedDrawstops() {
  return m_CombinationsStoreNonDisplayedDrawstops;
}

bool GOOrganController::GeneralsStoreDivisionalCouplers() {
  return m_GeneralsStoreDivisionalCouplers;
}

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

GOPipeConfigNode &GOOrganController::GetPipeConfig() { return m_PipeConfig; }

void GOOrganController::UpdateAmplitude() {}

void GOOrganController::UpdateTuning() {
  m_PitchLabel.SetContent(
    wxString::Format(_("%f cent"), m_PipeConfig.GetPipeConfig().GetTuning()));
}

void GOOrganController::UpdateAudioGroup() {}

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

GOMemoryPool &GOOrganController::GetMemoryPool() { return m_pool; }

GOConfig &GOOrganController::GetSettings() { return m_config; }

GOBitmapCache &GOOrganController::GetBitmapCache() { return m_bitmaps; }

GOSoundSampler *GOOrganController::StartSample(
  const GOSoundProvider *pipe,
  int sampler_group_id,
  unsigned audio_group,
  unsigned velocity,
  unsigned delay,
  uint64_t last_stop) {
  if (!m_soundengine)
    return NULL;
  return m_soundengine->StartSample(
    pipe, sampler_group_id, audio_group, velocity, delay, last_stop);
}

uint64_t GOOrganController::StopSample(
  const GOSoundProvider *pipe, GOSoundSampler *handle) {
  if (m_soundengine)
    return m_soundengine->StopSample(pipe, handle);
  return 0;
}

void GOOrganController::SwitchSample(
  const GOSoundProvider *pipe, GOSoundSampler *handle) {
  if (m_soundengine)
    m_soundengine->SwitchSample(pipe, handle);
}

void GOOrganController::UpdateVelocity(
  const GOSoundProvider *pipe, GOSoundSampler *handle, unsigned velocity) {
  if (m_soundengine)
    m_soundengine->UpdateVelocity(pipe, handle, velocity);
}

void GOOrganController::SendMidiMessage(GOMidiEvent &e) {
  if (m_midi)
    m_midi->Send(e);
}

void GOOrganController::SendMidiRecorderMessage(GOMidiEvent &e) {
  if (m_MidiRecorder)
    m_MidiRecorder->SendMidiRecorderMessage(e);
}

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
  GOEventDistributor::PreparePlayback();

  m_setter->UpdateModified(m_OrganModified);

  GOEventDistributor::StartPlayback();
  GOEventDistributor::PrepareRecording();
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
  if (event.GetMidiType() == MIDI_RESET) {
    Reset();
    return;
  }
  while (m_MidiSamplesetMatch.size() < event.GetDevice())
    m_MidiSamplesetMatch.push_back(true);

  if (event.GetMidiType() == MIDI_SYSEX_GO_CLEAR)
    m_MidiSamplesetMatch[event.GetDevice()] = true;
  else if (event.GetMidiType() == MIDI_SYSEX_GO_SAMPLESET) {
    if (
      event.GetKey() == m_SampleSetId1 && event.GetValue() == m_SampleSetId2) {
      m_MidiSamplesetMatch[event.GetDevice()] = true;
    } else {
      m_MidiSamplesetMatch[event.GetDevice()] = false;
      return;
    }
  } else if (event.GetMidiType() == MIDI_SYSEX_GO_SETUP) {
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
  m_setter->ResetDisplay();
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

int GOOrganController::GetRecorderElementID(wxString name) {
  return m_config.GetMidiMap().GetElementByString(name);
}

GOCombinationDefinition &GOOrganController::GetGeneralTemplate() {
  return m_GeneralTemplate;
}

GOLabelControl *GOOrganController::GetPitchLabel() { return &m_PitchLabel; }

GOLabelControl *GOOrganController::GetTemperamentLabel() {
  return &m_TemperamentLabel;
}

GOMainWindowData *GOOrganController::GetMainWindowData() {
  return &m_MainWindowData;
}

void GOOrganController::MarkSectionInUse(wxString name) {
  if (m_UsedSections[name])
    throw wxString::Format(_("Section %s already in use"), name.c_str());
  m_UsedSections[name] = true;
}
