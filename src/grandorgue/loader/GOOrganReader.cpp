/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOOrganReader.h"

#include <wx/filename.h>
#include <wx/log.h>
#include <wx/msgdlg.h>

#include "archive/GOArchive.h"
#include "config/GOConfig.h"
#include "config/GOConfigFileReader.h"
#include "files/GOOpenedFile.h"
#include "files/GOStdFileName.h"

#include "GOFileStore.h"
#include "GOLoaderFilename.h"
#include "GOOrgan.h"
#include "GOProgressMonitor.h"
#include "go_path.h"

static const wxString WX_ORGAN = wxT("Organ");

GOOrganReader::GOOrganReader(
  GOConfig &config,
  const GOOrgan &organ,
  const wxString &settingsPath,
  GOFileStore &fileStore,
  GOProgressMonitor &monitor)
  : m_ConfigDB(config.ODFCheck()),
    m_ConfigReader(m_ConfigDB, config.ODFCheck(), config.ODFHw1Check()) {
  GOLoaderFilename odfName;
  const wxString &archiveID = organ.GetArchiveID();

  if (!archiveID.IsEmpty()) {
    monitor.Setup(1, _("Loading sample set"), _("Parsing organ packages"));

    wxString archiveErrMsg;

    if (!fileStore.LoadArchives(
          config,
          config.OrganCachePath(),
          archiveID,
          organ.GetArchivePath(),
          archiveErrMsg))
      throw archiveErrMsg;
    odfName.Assign(organ.GetODFPath());
  } else {
    odfName.AssignAbsolute(organ.GetODFPath());
    fileStore.SetDirectory(go_get_path(organ.GetODFPath()));
  }
  m_Hash = organ.GetOrganHash();
  monitor.Setup(
    1, _("Loading sample set"), _("Parsing sample set definition file"));
  m_LoadedOrganInfo.settingsFilePath = config.OrganSettingsPath()
    + wxFileName::GetPathSeparator()
    + GOStdFileName::composeSettingFileName(m_Hash, config.Preset());
  m_LoadedOrganInfo.cacheFilePath = config.OrganCachePath()
    + wxFileName::GetPathSeparator()
    + GOStdFileName::composeCacheFileName(m_Hash, config.Preset());

  GOConfigFileReader odfIniFile;

  if (!odfIniFile.Read(odfName.Open(fileStore).get()))
    throw wxString::Format(_("Unable to read '%s'"), odfName.GetPath());

  m_LoadedOrganInfo.odfHash = odfIniFile.GetHash();
  m_LoadedOrganInfo.isCustomized = false;
  m_ConfigDB.ReadData(odfIniFile, ODFSetting, false);

  wxString effectiveSettingsPath = settingsPath;
  bool canReadSettingsDirectly = true;

  if (effectiveSettingsPath.IsEmpty()) {
    if (wxFileExists(m_LoadedOrganInfo.settingsFilePath)) {
      effectiveSettingsPath = m_LoadedOrganInfo.settingsFilePath;
      m_LoadedOrganInfo.isCustomized = true;
    } else {
      wxString bundledSettingsPath
        = organ.GetODFPath().BeforeLast('.') + wxT(".cmb");

      if (!fileStore.AreArchivesUsed()) {
        if (wxFileExists(bundledSettingsPath)) {
          effectiveSettingsPath = bundledSettingsPath;
          m_LoadedOrganInfo.isCustomized = true;
        }
      } else {
        if (fileStore.FindArchiveContaining(organ.GetODFPath())
              ->containsFile(bundledSettingsPath)) {
          effectiveSettingsPath = bundledSettingsPath;
          m_LoadedOrganInfo.isCustomized = true;
          canReadSettingsDirectly = false;
        }
      }
    }
  }

  if (!effectiveSettingsPath.IsEmpty()) {
    GOConfigFileReader settingsConfig;

    if (canReadSettingsDirectly) {
      if (!settingsConfig.Read(effectiveSettingsPath))
        throw wxString::Format(_("Unable to read '%s'"), effectiveSettingsPath);
    } else {
      if (!settingsConfig.Read(
            fileStore.FindArchiveContaining(organ.GetODFPath())
              ->OpenFile(effectiveSettingsPath)))
        throw wxString::Format(_("Unable to read '%s'"), effectiveSettingsPath);
    }

    if (
      odfIniFile.getEntry(WX_ORGAN, wxT("ChurchName")).Trim()
      != settingsConfig.getEntry(WX_ORGAN, wxT("ChurchName")).Trim())
      wxLogWarning(
        _("This .cmb file was originally created for:\n%s"),
        settingsConfig.getEntry(WX_ORGAN, wxT("ChurchName")).c_str());

    m_ConfigDB.ReadData(settingsConfig, CMBSetting, false);
    wxString settingsOdfHash
      = settingsConfig.getEntry(WX_ORGAN, wxT("ODFHash"));

    if (settingsOdfHash != wxEmptyString)
      if (settingsOdfHash != m_LoadedOrganInfo.odfHash) {
        if (
          wxMessageBox(
            _("The .cmb file does not exactly match the current "
              "ODF. Importing it can cause various problems. "
              "Should it really be imported?"),
            _("Import"),
            wxYES_NO,
            NULL)
          == wxNO) {
          m_ConfigDB.ClearCMB();
        }
      }
  } else {
    bool hasOldGoSettings = m_ConfigDB.ReadData(odfIniFile, CMBSetting, true);

    if (hasOldGoSettings)
      if (
        wxMessageBox(
          _("The ODF contains GrandOrgue 0.2 styled saved "
            "settings. Should they be imported?"),
          _("Import"),
          wxYES_NO,
          NULL)
        == wxNO) {
        m_ConfigDB.ClearCMB();
      }
  }

  /* skip informational items */
  m_ConfigReader.ReadString(CMBSetting, WX_ORGAN, wxT("ChurchName"), false);
  m_ConfigReader.ReadString(CMBSetting, WX_ORGAN, wxT("ChurchAddress"), false);
  m_ConfigReader.ReadString(CMBSetting, WX_ORGAN, wxT("ODFPath"), false);
  m_ConfigReader.ReadString(CMBSetting, WX_ORGAN, wxT("ODFHash"), false);
  m_ConfigReader.ReadString(CMBSetting, WX_ORGAN, wxT("ArchiveID"), false);
}
