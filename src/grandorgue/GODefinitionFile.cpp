/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GODefinitionFile.h"

#include <math.h>
#include <wx/filename.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/stream.h>
#include <wx/wfstream.h>

#include "archive/GOArchive.h"
#include "archive/GOArchiveFile.h"
#include "archive/GOArchiveManager.h"
#include "config/GOConfig.h"
#include "config/GOConfigFileReader.h"
#include "config/GOConfigFileWriter.h"
#include "config/GOConfigReader.h"
#include "config/GOConfigReaderDB.h"
#include "config/GOConfigWriter.h"
#include "contrib/sha1.h"
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
#include "midi/GOMidi.h"
#include "midi/GOMidiEvent.h"
#include "midi/GOMidiPlayer.h"
#include "midi/GOMidiRecorder.h"
#include "sound/GOSoundEngine.h"
#include "temperaments/GOTemperament.h"

#include "GOAudioRecorder.h"
#include "GOBuffer.h"
#include "GOCache.h"
#include "GOCacheWriter.h"
#include "GOCoupler.h"
#include "GODivisional.h"
#include "GODivisionalCoupler.h"
#include "GODocument.h"
#include "GOElementCreator.h"
#include "GOEnclosure.h"
#include "GOEvent.h"
#include "GOFile.h"
#include "GOFilename.h"
#include "GOGeneral.h"
#include "GOHash.h"
#include "GOLoadThread.h"
#include "GOManual.h"
#include "GOMetronome.h"
#include "GOOrgan.h"
#include "GOPath.h"
#include "GOPushbutton.h"
#include "GORank.h"
#include "GOReleaseAlignTable.h"
#include "GOSetter.h"
#include "GOSoundingPipe.h"
#include "GOSwitch.h"
#include "GOTremulant.h"
#include "GOWindchest.h"

GODefinitionFile::GODefinitionFile(GODocument *doc, GOConfig &settings)
  : m_doc(doc),
    m_odf(),
    m_ArchiveID(),
    m_hash(),
    m_path(),
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
    m_archives(),
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
  m_pool.SetMemoryLimit(m_config.MemoryLimit() * 1024 * 1024);
}

bool GODefinitionFile::IsCacheable() { return m_Cacheable; }

GOHashType GODefinitionFile::GenerateCacheHash() {
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

void GODefinitionFile::ReadOrganFile(GOConfigReader &cfg) {
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
    = cfg.ReadStringTrim(ODFSetting, group, wxT("InfoFilename"), false);
  wxFileName fn;
  if (info_filename.IsEmpty()) {
    /* Resolve organ file path */
    fn = GetODFFilename();
    fn.SetExt(wxT("html"));
    if (fn.FileExists() && !useArchives())
      m_InfoFilename = fn.GetFullPath();
    else
      m_InfoFilename = wxEmptyString;
  } else {
    GOFilename fname;
    fname.Assign(info_filename, this);
    std::unique_ptr<GOFile> file = fname.Open();
    fn = info_filename;
    if (
      file->isValid()
      && (fn.GetExt() == wxT("html") || fn.GetExt() == wxT("htm"))) {
      if (fn.FileExists() && !useArchives())
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

  GOModel::Load(cfg, this);
  wxString buffer;

  for (unsigned i = 0; i < m_enclosure.size(); i++)
    m_enclosure[i]->SetElementID(
      GetRecorderElementID(wxString::Format(wxT("E%d"), i)));

  for (unsigned i = 0; i < m_switches.size(); i++)
    m_switches[i]->SetElementID(
      GetRecorderElementID(wxString::Format(wxT("S%d"), i)));

  for (unsigned i = 0; i < m_tremulant.size(); i++)
    m_tremulant[i]->SetElementID(
      GetRecorderElementID(wxString::Format(wxT("T%d"), i)));

  m_setter = new GOSetter(this);
  m_elementcreators.push_back(m_setter);
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
  for (unsigned i = m_FirstManual; i < m_manual.size(); i++)
    m_manual[i]->GetDivisionalTemplate().InitDivisional(i);

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

wxString GODefinitionFile::GetOrganHash() { return m_hash; }

wxString GODefinitionFile::GenerateSettingFileName() {
  return m_config.OrganSettingsPath() + wxFileName::GetPathSeparator()
    + GOStdFileName::composeSettingFileName(GetOrganHash(), m_config.Preset());
}

wxString GODefinitionFile::GenerateCacheFileName() {
  return m_config.OrganCachePath() + wxFileName::GetPathSeparator()
    + GOStdFileName::composeCacheFileName(GetOrganHash(), m_config.Preset());
}

bool GODefinitionFile::LoadArchive(
  wxString ID, wxString &name, const wxString &parentID) {
  GOArchiveManager manager(m_config, m_config.OrganCachePath());
  GOArchive *archive = manager.LoadArchive(ID);
  if (archive) {
    m_archives.push_back(archive);
    return true;
  }
  name = wxEmptyString;
  const GOArchiveFile *a = m_config.GetArchiveByID(ID);
  if (a)
    name = a->GetName();
  else if (parentID != wxEmptyString) {
    a = m_config.GetArchiveByID(parentID);
    for (unsigned i = 0; i < a->GetDependencies().size(); i++)
      if (a->GetDependencies()[i] == ID)
        name = a->GetDependencyTitles()[i];
  }
  return false;
}

wxString GODefinitionFile::Load(
  GOProgressDialog *dlg, const GOOrgan &organ, const wxString &file2) {
  GOFilename odf_name;

  if (organ.GetArchiveID() != wxEmptyString) {
    dlg->Setup(1, _("Loading sample set"), _("Parsing organ packages"));
    wxString name;
    m_archives.clear();

    if (!LoadArchive(organ.GetArchiveID(), name))
      return wxString::Format(
        _("Failed to open organ package '%s' (%s)"),
        name.c_str(),
        organ.GetArchiveID().c_str());
    GOArchive *main = m_archives[0];
    m_ArchiveID = main->GetArchiveID();
    m_ArchivePath = main->GetPath();

    for (unsigned i = 0; i < main->GetDependencies().size(); i++) {
      if (!LoadArchive(main->GetDependencies()[i], name))
        return wxString::Format(
          _("Failed to open organ package '%s' (%s)"),
          name.c_str(),
          organ.GetArchiveID().c_str());
    }
    m_odf = organ.GetODFPath();
    m_path = "";
    odf_name.Assign(m_odf, this);
  } else {
    wxString file = organ.GetODFPath();
    m_odf = GONormalizePath(file);
    m_path = GOGetPath(m_odf);
    odf_name.AssignAbsolute(m_odf);
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
      if (!useArchives()) {
        if (wxFileExists(bundledSettingsFile)) {
          setting_file = bundledSettingsFile;
          m_b_customized = true;
        }
      } else {
        if (findArchive(m_odf)->containsFile(bundledSettingsFile)) {
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
      if (!extra_odf_config.Read(findArchive(m_odf)->OpenFile(setting_file))) {
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
    dlg->Reset(GetCacheObjectCount());
    /* Load pipes */
    atomic_uint nb_loaded_obj(0);

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
            GOCacheObject *obj = GetCacheObject(nb_loaded_obj);
            if (!obj)
              break;
            if (!obj->LoadCache(reader)) {
              cache_ok = false;
              wxLogError(
                _("Cache load failure: Failed to read %s from cache."),
                obj->GetLoadTitle().c_str());
              break;
            }
            nb_loaded_obj.fetch_add(1);
            if (!dlg->Update(nb_loaded_obj, obj->GetLoadTitle())) {
              dummy.free();
              SetTemperament(m_Temperament);
              GOMessageBox(
                _("Load aborted by the user - only parts of the "
                  "organ are loaded."),
                _("Load error"),
                wxOK | wxICON_ERROR,
                NULL);
              CloseArchives();
              return wxEmptyString;
            }
          }
          if (nb_loaded_obj >= GetCacheObjectCount())
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
        threads.push_back(new GOLoadThread(*this, m_pool, nb_loaded_obj));

      for (unsigned i = 0; i < threads.size(); i++)
        threads[i]->Run();

      for (unsigned pos = nb_loaded_obj.fetch_add(1); true;
           pos = nb_loaded_obj.fetch_add(1)) {
        GOCacheObject *obj = GetCacheObject(pos);
        if (!obj)
          break;
        obj->LoadData();
        if (!dlg->Update(nb_loaded_obj, obj->GetLoadTitle())) {
          dummy.free();
          SetTemperament(m_Temperament);
          GOMessageBox(
            _("Load aborted by the user - only parts of the organ "
              "are loaded."),
            _("Load error"),
            wxOK | wxICON_ERROR,
            NULL);
          CloseArchives();
          return wxEmptyString;
        }
      }

      for (unsigned i = 0; i < threads.size(); i++)
        threads[i]->checkResult();

      if (nb_loaded_obj >= GetCacheObjectCount())
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
    CloseArchives();
    return wxEmptyString;
  } catch (wxString error_) {
    dummy.free();
    return error_;
  }
  dummy.free();

  CloseArchives();

  return wxEmptyString;
}

void GODefinitionFile::LoadCombination(const wxString &file) {
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

bool GODefinitionFile::CachePresent() { return wxFileExists(m_CacheFilename); }

bool GODefinitionFile::UpdateCache(GOProgressDialog *dlg, bool compress) {
  DeleteCache();
  /* Figure out the list of pipes to save */
  unsigned nb_saved_objs = 0;

  dlg->Setup(GetCacheObjectCount(), _("Creating sample cache"));

  wxFileOutputStream file(m_CacheFilename);
  GOCacheWriter writer(file, compress);

  /* Save pipes to cache */
  bool cache_save_ok = writer.WriteHeader();

  GOHashType hash = GenerateCacheHash();
  if (!writer.Write(&hash, sizeof(hash)))
    cache_save_ok = false;

  for (unsigned i = 0; cache_save_ok; i++) {
    GOCacheObject *obj = GetCacheObject(i);
    if (!obj)
      break;
    if (!obj->SaveCache(writer)) {
      cache_save_ok = false;
      wxLogError(
        _("Save of %s to the cache failed"), obj->GetLoadTitle().c_str());
    }
    nb_saved_objs++;

    if (!dlg->Update(nb_saved_objs, obj->GetLoadTitle())) {
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

void GODefinitionFile::DeleteCache() {
  if (CachePresent())
    wxRemoveFile(m_CacheFilename);
}

GODefinitionFile::~GODefinitionFile(void) {
  CloseArchives();
  Cleanup();
  // Just to be sure, that the sound providers are freed before the pool
  m_manual.clear();
  m_tremulant.clear();
  m_ranks.clear();
}

void GODefinitionFile::CloseArchives() {
  for (unsigned i = 0; i < m_archives.size(); i++)
    m_archives[i]->Close();
}

void GODefinitionFile::DeleteSettings() { wxRemoveFile(m_SettingFilename); }

bool GODefinitionFile::Save() {
  if (!Export(m_SettingFilename))
    return false;
  m_doc->Modify(false);
  m_setter->UpdateModified(false);
  return true;
}

bool GODefinitionFile::Export(const wxString &cmb) {
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

GOEnclosure *GODefinitionFile::GetEnclosure(
  const wxString &name, bool is_panel) {
  for (unsigned i = 0; i < m_elementcreators.size(); i++) {
    GOEnclosure *c = m_elementcreators[i]->GetEnclosure(name, is_panel);
    if (c)
      return c;
  }
  return NULL;
}

GOLabel *GODefinitionFile::GetLabel(const wxString &name, bool is_panel) {
  for (unsigned i = 0; i < m_elementcreators.size(); i++) {
    GOLabel *c = m_elementcreators[i]->GetLabel(name, is_panel);
    if (c)
      return c;
  }
  return NULL;
}

GOButton *GODefinitionFile::GetButton(const wxString &name, bool is_panel) {
  for (unsigned i = 0; i < m_elementcreators.size(); i++) {
    GOButton *c = m_elementcreators[i]->GetButton(name, is_panel);
    if (c)
      return c;
  }
  return NULL;
}

GODocument *GODefinitionFile::GetDocument() { return m_doc; }

void GODefinitionFile::SetVolume(int volume) { m_volume = volume; }

int GODefinitionFile::GetVolume() { return m_volume; }

void GODefinitionFile::SetIgnorePitch(bool ignore) { m_IgnorePitch = ignore; }

bool GODefinitionFile::GetIgnorePitch() { return m_IgnorePitch; }

void GODefinitionFile::SetReleaseTail(unsigned releaseTail) {
  m_releaseTail = releaseTail;
  Modified();
}

bool GODefinitionFile::DivisionalsStoreIntermanualCouplers() {
  return m_DivisionalsStoreIntermanualCouplers;
}

bool GODefinitionFile::DivisionalsStoreIntramanualCouplers() {
  return m_DivisionalsStoreIntramanualCouplers;
}

bool GODefinitionFile::DivisionalsStoreTremulants() {
  return m_DivisionalsStoreTremulants;
}

bool GODefinitionFile::CombinationsStoreNonDisplayedDrawstops() {
  return m_CombinationsStoreNonDisplayedDrawstops;
}

bool GODefinitionFile::GeneralsStoreDivisionalCouplers() {
  return m_GeneralsStoreDivisionalCouplers;
}

GOSetter *GODefinitionFile::GetSetter() { return m_setter; }

GOGUIPanel *GODefinitionFile::GetPanel(unsigned index) {
  return m_panels[index];
}

unsigned GODefinitionFile::GetPanelCount() { return m_panels.size(); }

void GODefinitionFile::AddPanel(GOGUIPanel *panel) {
  m_panels.push_back(panel);
}

const wxString &GODefinitionFile::GetChurchName() { return m_ChurchName; }

const wxString &GODefinitionFile::GetChurchAddress() { return m_ChurchAddress; }

const wxString &GODefinitionFile::GetOrganBuilder() { return m_OrganBuilder; }

const wxString &GODefinitionFile::GetOrganBuildDate() {
  return m_OrganBuildDate;
}

const wxString &GODefinitionFile::GetOrganComments() { return m_OrganComments; }

const wxString &GODefinitionFile::GetRecordingDetails() {
  return m_RecordingDetails;
}

const wxString &GODefinitionFile::GetInfoFilename() { return m_InfoFilename; }

bool GODefinitionFile::useArchives() { return m_archives.size() > 0; }

GOArchive *GODefinitionFile::findArchive(const wxString &name) {
  for (unsigned i = 0; i < m_archives.size(); i++) {
    if (m_archives[i]->containsFile(name))
      return m_archives[i];
  }
  return NULL;
}

GOPipeConfigNode &GODefinitionFile::GetPipeConfig() { return m_PipeConfig; }

void GODefinitionFile::UpdateAmplitude() {}

void GODefinitionFile::UpdateTuning() {
  m_PitchLabel.SetContent(
    wxString::Format(_("%f cent"), m_PipeConfig.GetPipeConfig().GetTuning()));
}

void GODefinitionFile::UpdateAudioGroup() {}

bool GODefinitionFile::IsCustomized() { return m_b_customized; }

const wxString GODefinitionFile::GetODFFilename() { return m_odf; }

const wxString GODefinitionFile::GetODFPath() { return m_path.c_str(); }

const wxString GODefinitionFile::GetOrganPathInfo() {
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

GOOrgan GODefinitionFile::GetOrganInfo() {
  return GOOrgan(
    GetODFFilename(),
    m_ArchiveID,
    m_ArchivePath,
    GetChurchName(),
    GetOrganBuilder(),
    GetRecordingDetails());
}

const wxString GODefinitionFile::GetSettingFilename() {
  return m_SettingFilename;
}

const wxString GODefinitionFile::GetCacheFilename() { return m_CacheFilename; }

GOMemoryPool &GODefinitionFile::GetMemoryPool() { return m_pool; }

GOConfig &GODefinitionFile::GetSettings() { return m_config; }

GOGUIMouseStateTracker &GODefinitionFile::GetMouseStateTracker() {
  return m_MouseState;
}

GOBitmapCache &GODefinitionFile::GetBitmapCache() { return m_bitmaps; }

GOSoundSampler *GODefinitionFile::StartSample(
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

uint64_t GODefinitionFile::StopSample(
  const GOSoundProvider *pipe, GOSoundSampler *handle) {
  if (m_soundengine)
    return m_soundengine->StopSample(pipe, handle);
  return 0;
}

void GODefinitionFile::SwitchSample(
  const GOSoundProvider *pipe, GOSoundSampler *handle) {
  if (m_soundengine)
    m_soundengine->SwitchSample(pipe, handle);
}

void GODefinitionFile::UpdateVelocity(
  const GOSoundProvider *pipe, GOSoundSampler *handle, unsigned velocity) {
  if (m_soundengine)
    m_soundengine->UpdateVelocity(pipe, handle, velocity);
}

void GODefinitionFile::SendMidiMessage(GOMidiEvent &e) {
  if (m_midi)
    m_midi->Send(e);
}

void GODefinitionFile::SendMidiRecorderMessage(GOMidiEvent &e) {
  if (m_MidiRecorder)
    m_MidiRecorder->SendMidiRecorderMessage(e);
}

GOMidi *GODefinitionFile::GetMidi() { return m_midi; }

void GODefinitionFile::LoadMIDIFile(wxString const &filename) {
  m_MidiPlayer->LoadFile(
    filename, GetODFManualCount() - 1, GetFirstManualIndex() == 0);
}

void GODefinitionFile::Abort() {
  m_soundengine = NULL;

  GOEventDistributor::AbortPlayback();

  m_MidiPlayer->StopPlaying();
  m_MidiRecorder->StopRecording();
  m_AudioRecorder->StopRecording();
  m_AudioRecorder->SetAudioRecorder(NULL);
  m_midi = NULL;
}

void GODefinitionFile::PreconfigRecorder() {
  for (unsigned i = GetFirstManualIndex(); i <= GetManualAndPedalCount(); i++) {
    wxString id = wxString::Format(wxT("M%d"), i);
    m_MidiRecorder->PreconfigureMapping(id, false);
  }
}

void GODefinitionFile::PreparePlayback(
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

  m_setter->UpdateModified(m_doc->IsModified());

  GOEventDistributor::StartPlayback();
  GOEventDistributor::PrepareRecording();
}

void GODefinitionFile::PrepareRecording() {
  m_MidiRecorder->Clear();
  PreconfigRecorder();
  m_MidiRecorder->SetSamplesetId(m_SampleSetId1, m_SampleSetId2);
  PreconfigRecorder();

  GOEventDistributor::PrepareRecording();
}

void GODefinitionFile::Update() {
  for (unsigned i = 0; i < m_switches.size(); i++)
    m_switches[i]->Update();

  for (unsigned i = m_FirstManual; i < m_manual.size(); i++)
    m_manual[i]->Update();

  for (unsigned i = 0; i < m_tremulant.size(); i++)
    m_tremulant[i]->Update();

  for (unsigned i = 0; i < m_divisionalcoupler.size(); i++)
    m_divisionalcoupler[i]->Update();

  m_setter->Update();
}

void GODefinitionFile::ProcessMidi(const GOMidiEvent &event) {
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

void GODefinitionFile::Reset() {
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

void GODefinitionFile::SetTemperament(const GOTemperament &temperament) {
  m_TemperamentLabel.SetContent(wxGetTranslation(temperament.GetName()));
  for (unsigned k = 0; k < m_ranks.size(); k++)
    m_ranks[k]->SetTemperament(temperament);
}

void GODefinitionFile::SetTemperament(wxString name) {
  const GOTemperament &temperament
    = m_config.GetTemperaments().GetTemperament(name);
  m_Temperament = temperament.GetName();
  SetTemperament(temperament);
}

wxString GODefinitionFile::GetTemperament() { return m_Temperament; }

void GODefinitionFile::AllNotesOff() {
  for (unsigned k = GetFirstManualIndex(); k <= GetManualAndPedalCount(); k++)
    GetManual(k)->AllNotesOff();
}

void GODefinitionFile::Modified() {
  m_doc->Modify(true);
  m_setter->UpdateModified(true);
}

int GODefinitionFile::GetRecorderElementID(wxString name) {
  return m_config.GetMidiMap().GetElementByString(name);
}

GOCombinationDefinition &GODefinitionFile::GetGeneralTemplate() {
  return m_GeneralTemplate;
}

GOLabel *GODefinitionFile::GetPitchLabel() { return &m_PitchLabel; }

GOLabel *GODefinitionFile::GetTemperamentLabel() { return &m_TemperamentLabel; }

GOMainWindowData *GODefinitionFile::GetMainWindowData() {
  return &m_MainWindowData;
}

void GODefinitionFile::MarkSectionInUse(wxString name) {
  if (m_UsedSections[name])
    throw wxString::Format(_("Section %s already in use"), name.c_str());
  m_UsedSections[name] = true;
}

void GODefinitionFile::SetODFPath(wxString path) { m_path = path; }
