/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2014 GrandOrgue contributors (see AUTHORS)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "GrandOrgueFile.h"

#include "GOGUIPanel.h"
#include "GOSoundEngine.h"
#include "GOrgueCache.h"
#include "GOrgueCacheWriter.h"
#include "GOrgueConfigFileReader.h"
#include "GOrgueConfigFileWriter.h"
#include "GOrgueConfigReader.h"
#include "GOrgueConfigReaderDB.h"
#include "GOrgueConfigWriter.h"
#include "GOrgueCoupler.h"
#include "GOrgueDivisional.h"
#include "GOrgueDivisionalCoupler.h"
#include "GOrgueDocument.h"
#include "GOrgueEnclosure.h"
#include "GOrgueEvent.h"
#include "GOrgueEventHandler.h"
#include "GOrgueGeneral.h"
#include "GOrgueLCD.h"
#include "GOrgueLoadThread.h"
#include "GOrgueManual.h"
#include "GOrgueMidi.h"
#include "GOrgueMidiEvent.h"
#include "GOrguePath.h"
#include "GOrguePipe.h"
#include "GOrguePiston.h"
#include "GOrgueProgressDialog.h"
#include "GOrguePushbutton.h"
#include "GOrgueReleaseAlignTable.h"
#include "GOrgueSetter.h"
#include "GOrgueSettings.h"
#include "GOrgueSwitch.h"
#include "GOrgueRank.h"
#include "GOrgueTemperament.h"
#include "GOrgueTremulant.h"
#include "GOrgueWindchest.h"
#include "contrib/sha1.h"
#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <wx/stream.h>
#include <wx/wfstream.h>
#include <math.h>


GrandOrgueFile::GrandOrgueFile(GOrgueDocument* doc, GOrgueSettings& settings) :
	m_doc(doc),
	m_odf(),
	m_path(),
	m_CacheFilename(),
	m_SettingFilename(),
	m_ODFHash(),
	m_Cacheable(false),
	m_setter(0),
	m_volume(0),
	m_IgnorePitch(false),
	m_b_customized(false),
	m_DivisionalsStoreIntermanualCouplers(false),
	m_DivisionalsStoreIntramanualCouplers(false),
	m_DivisionalsStoreTremulants(false),
	m_GeneralsStoreDivisionalCouplers(false),
	m_CombinationsStoreNonDisplayedDrawstops(false),
	m_FirstManual(0),
	m_ChurchName(),
	m_ChurchAddress(),
	m_OrganBuilder(),
	m_OrganBuildDate(),
	m_OrganComments(),
	m_RecordingDetails(),
	m_InfoFilename(),
	m_RankCount(0),
	m_ODFManualCount(0),
	m_enclosure(0),
	m_tremulant(0),
	m_windchest(0),
	m_piston(0),
	m_general(0),
	m_divisionalcoupler(0),
	m_switches(0),
	m_ranks(0),
	m_manual(0),
	m_panels(0),
	m_handler(0),
	m_CacheObjects(0),
	m_UsedSections(),
	m_soundengine(0),
	m_midi(0),
	m_bitmaps(this),
	m_PipeConfig(this, this),
	m_Settings(settings),
	m_GeneralTemplate(this),
	m_PitchLabel(this),
	m_TemperamentLabel(this)
{
	m_pool.SetMemoryLimit(m_Settings.GetMemoryLimit());
}

bool GrandOrgueFile::IsCacheable()
{
	return m_Cacheable;
}

void GrandOrgueFile::ResolveReferences()
{
	for(unsigned i = 0; i < m_CacheObjects.size(); i++)
		m_CacheObjects[i]->Initialize();
}


void GrandOrgueFile::GenerateCacheHash(unsigned char hash[20])
{
	SHA_CTX ctx;
	int len;
	SHA1_Init(&ctx);
	for (unsigned i = 0; i < m_CacheObjects.size(); i++)
		m_CacheObjects[i]->UpdateHash(ctx);

	len = sizeof(GOAudioSection);
	SHA1_Update(&ctx, &len, sizeof(len));
	len = sizeof(GOrguePipe);
	SHA1_Update(&ctx, &len, sizeof(len));
	len = sizeof(GOrgueReleaseAlignTable);
	SHA1_Update(&ctx, &len, sizeof(len));
	len = BLOCK_HISTORY;
	SHA1_Update(&ctx, &len, sizeof(len));
	len = MAX_READAHEAD;
	SHA1_Update(&ctx, &len, sizeof(len));
	len = SHORT_LOOP_LENGTH;
	SHA1_Update(&ctx, &len, sizeof(len));
	len = sizeof(attack_section_info);
	SHA1_Update(&ctx, &len, sizeof(len));
	len = sizeof(release_section_info);
	SHA1_Update(&ctx, &len, sizeof(len));
	len = sizeof(audio_start_data_segment);
	SHA1_Update(&ctx, &len, sizeof(len));
	len = sizeof(audio_end_data_segment);
	SHA1_Update(&ctx, &len, sizeof(len));

	SHA1_Final(hash, &ctx);
}

#define FREE_AND_NULL(x) do { if (x) { free(x); x = NULL; } } while (0)

void GrandOrgueFile::ReadOrganFile(GOrgueConfigReader& cfg)
{
	wxString group = wxT("Organ");

	/* load church info */
	cfg.ReadString(ODFSetting, group, wxT("HauptwerkOrganFileFormatVersion"),  false);
	m_ChurchName = cfg.ReadStringTrim(ODFSetting, group, wxT("ChurchName"));
	m_ChurchAddress = cfg.ReadString(ODFSetting, group, wxT("ChurchAddress"));
	m_OrganBuilder = cfg.ReadString(ODFSetting, group, wxT("OrganBuilder"),  false);
	m_OrganBuildDate = cfg.ReadString(ODFSetting, group, wxT("OrganBuildDate"),  false);
	m_OrganComments = cfg.ReadString(ODFSetting, group, wxT("OrganComments"),  false);
	m_RecordingDetails = cfg.ReadString(ODFSetting, group, wxT("RecordingDetails"),  false);
	m_InfoFilename = cfg.ReadStringTrim(ODFSetting, group, wxT("InfoFilename"), false);
	wxFileName fn;
	if (m_InfoFilename.IsEmpty())
	{
		/* Resolve organ file path */
		fn = GetODFFilename();
		fn.SetExt(wxT(".html"));
	}
	else
		fn = GOCreateFilename(m_path, m_InfoFilename);
	if (fn.FileExists())
		m_InfoFilename = fn.GetFullPath();
	else
		m_InfoFilename = wxEmptyString;

	/* load basic organ information */
	unsigned NumberOfManuals = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfManuals"), 1, 16);
	m_FirstManual = cfg.ReadBoolean(ODFSetting, group, wxT("HasPedals")) ? 0 : 1;
	unsigned NumberOfEnclosures = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfEnclosures"), 0, 50);
	unsigned NumberOfTremulants = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfTremulants"), 0, 10);
	unsigned NumberOfWindchestGroups = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfWindchestGroups"), 1, 50);
	unsigned NumberOfReversiblePistons = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfReversiblePistons"), 0, 32);
	unsigned NumberOfGenerals = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfGenerals"), 0, 99);
	unsigned NumberOfDivisionalCouplers = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfDivisionalCouplers"), 0, 8);
	unsigned NumberOfPanels = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfPanels"), 0, 100, false);
	unsigned NumberOfSwitches = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfSwitches"), 0, 999, 0);
	m_RankCount = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfRanks"), 0, 400, false);
	m_PipeConfig.Load(cfg, group, wxEmptyString);
	m_DivisionalsStoreIntermanualCouplers = cfg.ReadBoolean(ODFSetting, group, wxT("DivisionalsStoreIntermanualCouplers"));
	m_DivisionalsStoreIntramanualCouplers = cfg.ReadBoolean(ODFSetting, group, wxT("DivisionalsStoreIntramanualCouplers"));
	m_DivisionalsStoreTremulants = cfg.ReadBoolean(ODFSetting, group, wxT("DivisionalsStoreTremulants"));
	m_GeneralsStoreDivisionalCouplers = cfg.ReadBoolean(ODFSetting, group, wxT("GeneralsStoreDivisionalCouplers"));
	m_CombinationsStoreNonDisplayedDrawstops = cfg.ReadBoolean(ODFSetting, group, wxT("CombinationsStoreNonDisplayedDrawstops"));
	m_volume = cfg.ReadInteger(CMBSetting, group, wxT("Volume"), -121, 100, false, -121);
	if (m_volume > 20)
		m_volume = -121;
	m_Temperament = cfg.ReadString(CMBSetting, group, wxT("Temperament"), false);
	m_IgnorePitch = cfg.ReadBoolean(CMBSetting, group, wxT("IgnorePitch"), false, false);

	wxString buffer;

	m_windchest.resize(0);
	for (unsigned i = 0; i < NumberOfWindchestGroups; i++)
		m_windchest.push_back(new GOrgueWindchest(this));

	m_manual.resize(0);
	m_manual.resize(m_FirstManual); // Add empty slot for pedal, if necessary
	for (unsigned int i = m_FirstManual; i <= NumberOfManuals; i++)
		m_manual.push_back(new GOrgueManual(this));
	m_ODFManualCount = NumberOfManuals + 1;
	for(unsigned int i = 0; i < 4; i++)
		m_manual.push_back(new GOrgueManual(this));

	m_enclosure.resize(0);
	for (unsigned i = 0; i < NumberOfEnclosures; i++)
	{
		m_enclosure.push_back(new GOrgueEnclosure(this));
		buffer.Printf(wxT("Enclosure%03u"), i + 1);
		m_enclosure[i]->Load(cfg, buffer, i);
		m_enclosure[i]->SetElementID(GetRecorderElementID(wxString::Format(wxT("E%d"), i)));
	}

	m_switches.resize(0);
	for (unsigned i = 0; i < NumberOfSwitches; i++)
	{
		m_switches.push_back(new GOrgueSwitch(this));
		buffer.Printf(wxT("Switch%03d"), i + 1);
		m_switches[i]->Load(cfg, buffer);
		m_switches[i]->SetElementID(GetRecorderElementID(wxString::Format(wxT("S%d"), i)));
	}

	m_tremulant.resize(0);
	for (unsigned i = 0; i < NumberOfTremulants; i++)
	{
		m_tremulant.push_back(new GOrgueTremulant(this));
		buffer.Printf(wxT("Tremulant%03d"), i + 1);
		m_tremulant[i]->Load(cfg, buffer, -((int)(i + 1)));
		m_tremulant[i]->SetElementID(GetRecorderElementID(wxString::Format(wxT("T%d"), i)));
	}

	for (unsigned  i = 0; i < NumberOfWindchestGroups; i++)
	{
		buffer.Printf(wxT("WindchestGroup%03d"), i + 1);
		m_windchest[i]->Load(cfg, buffer, i);
	}

	for (unsigned  i = 0; i < m_RankCount; i++)
	{
		m_ranks.push_back(new GOrgueRank(this));
		buffer.Printf(wxT("Rank%03d"), i + 1);
		m_ranks[i]->Load(cfg, buffer, -1);
	}

	for (unsigned int i = m_FirstManual; i <= NumberOfManuals; i++)
	{
		buffer.Printf(wxT("Manual%03d"), i);
		m_manual[i]->Load(cfg, buffer, i);
	}

	unsigned min_key = 0xff, max_key = 0;
	for(unsigned i = GetFirstManualIndex(); i < GetODFManualCount(); i++)
	{
		GOrgueManual* manual = GetManual(i);
		if ((unsigned)manual->GetFirstLogicalKeyMIDINoteNumber() < min_key)
			min_key = manual->GetFirstLogicalKeyMIDINoteNumber();
		if (manual->GetFirstLogicalKeyMIDINoteNumber() + manual->GetLogicalKeyCount() > max_key)
			max_key = manual->GetFirstLogicalKeyMIDINoteNumber() + manual->GetLogicalKeyCount();
	}
	for (unsigned i = GetODFManualCount(); i <= GetManualAndPedalCount(); i++)
		GetManual(i)->Init(cfg, wxString::Format(wxT("SetterFloating%03d"), i - GetODFManualCount() + 1), i, min_key, max_key - min_key);

	m_piston.resize(0);
	for (unsigned i = 0; i < NumberOfReversiblePistons; i++)
	{
		m_piston.push_back(new GOrguePiston(this));
		buffer.Printf(wxT("ReversiblePiston%03d"), i + 1);
		m_piston[i]->Load(cfg, buffer);
	}

	m_divisionalcoupler.resize(0);
	for (unsigned i = 0; i < NumberOfDivisionalCouplers; i++)
	{
		m_divisionalcoupler.push_back(new GOrgueDivisionalCoupler(this));
		buffer.Printf(wxT("DivisionalCoupler%03d"), i + 1);
		m_divisionalcoupler[i]->Load(cfg, buffer);
	}

	m_general.resize(0);
	m_GeneralTemplate.InitGeneral();
	for (unsigned i = 0; i < NumberOfGenerals; i++)
	{
		m_general.push_back(new GOrgueGeneral(this->GetGeneralTemplate(), this, false));
		buffer.Printf(wxT("General%03d"), i + 1);
		m_general[i]->Load(cfg, buffer);
	}

	m_setter = new GOrgueSetter(this);
	m_setter->Load(cfg);

	m_PitchLabel.Load(cfg, wxT("SetterMasterPitch"));
	m_TemperamentLabel.Load(cfg, wxT("SetterMasterTemperament"));

	m_panels.resize(0);
	m_panels.push_back(new GOGUIPanel(this));
	m_panels[0]->Load(cfg, wxT(""));

	for (unsigned i = 0; i < NumberOfPanels; i++)
	{
		buffer.Printf(wxT("Panel%03d"), i + 1);
		m_panels.push_back(new GOGUIPanel(this));
		m_panels[i + 1]->Load(cfg, buffer);
	}

	for (unsigned i = m_FirstManual; i <= NumberOfManuals; i++)
		m_panels.push_back(m_setter->CreateCouplerPanel(cfg, i));
	m_panels.push_back(m_setter->CreateCrescendoPanel(cfg));
	m_panels.push_back(m_setter->CreateDivisionalPanel(cfg));
	m_panels.push_back(m_setter->CreateGeneralsPanel(cfg));
	m_panels.push_back(m_setter->CreateSetterPanel(cfg));

	for (unsigned i = m_ODFManualCount; i < m_manual.size(); i++)
		m_panels.push_back(m_setter->CreateCouplerPanel(cfg, i));
	m_panels.push_back(m_setter->CreateFloatingPanel(cfg));

	m_panels.push_back(m_setter->CreateMasterPanel(cfg));

	m_GeneralTemplate.InitGeneral();
	for (unsigned i = m_FirstManual; i < m_manual.size(); i++)
		m_manual[i]->GetDivisionalTemplate().InitDivisional(i);

	ReadCombinations(cfg);
}

void GrandOrgueFile::ReadCombinations(GOrgueConfigReader& cfg)
{
	for (unsigned i = m_FirstManual; i < m_manual.size(); i++)
		m_manual[i]->LoadCombination(cfg);

	for (unsigned j = 0; j < m_general.size(); j++)
		m_general[j]->LoadCombination(cfg);

	if (m_setter)
		m_setter->LoadCombination(cfg);
}

wxString GrandOrgueFile::GenerateSettingFileName()
{
	SHA_CTX ctx;
	unsigned char hash[20];
	wxFileName odf(m_odf);
	odf.Normalize(wxPATH_NORM_ALL | wxPATH_NORM_CASE);
	wxString filename = odf.GetFullPath();

	SHA1_Init(&ctx);
	SHA1_Update(&ctx, (const wxChar*)filename.c_str(), (filename.Length() + 1) * sizeof(wxChar));
	SHA1_Final(hash, &ctx);
       
	filename = wxEmptyString;
	for(unsigned i = 0; i < 20; i++)
		filename += wxDecToHex(hash[i]);

	filename = m_Settings.GetUserSettingPath()  + wxFileName::GetPathSeparator() + 
		filename + wxString::Format(wxT("-%d.cmb"), m_Settings.GetPreset());

	return filename;
}

wxString GrandOrgueFile::GenerateCacheFileName()
{
	SHA_CTX ctx;
	unsigned char hash[20];
	wxFileName odf(m_odf);
	odf.Normalize(wxPATH_NORM_ALL | wxPATH_NORM_CASE);
	wxString filename = odf.GetFullPath();

	SHA1_Init(&ctx);
	SHA1_Update(&ctx, (const wxChar*)filename.c_str(), (filename.Length() + 1) * sizeof(wxChar));
	SHA1_Final(hash, &ctx);

	filename = wxEmptyString;
	for(unsigned i = 0; i < 20; i++)
		filename += wxDecToHex(hash[i]);

	filename = m_Settings.GetUserCachePath()  + wxFileName::GetPathSeparator() + 
		filename + wxString::Format(wxT("-%d.cache"), m_Settings.GetPreset());

	return filename;
}

wxString GrandOrgueFile::Load(GOrgueProgressDialog* dlg, const wxString& file, const wxString& file2)
{
	dlg->Setup(1, _("Loading sample set") ,_("Parsing sample set definition file"));
	m_odf = GONormalizePath(file);
	m_path = GOGetPath(m_odf);
	m_SettingFilename = GenerateSettingFileName();
	m_CacheFilename = GenerateCacheFileName();
	m_Cacheable = false;

	GOrgueConfigFileReader odf_ini_file;

	if (!odf_ini_file.Read(file))
	{
		wxString error;
		error.Printf(_("Unable to read '%s'"), file.c_str());
		return error;
	}

	m_ODFHash = odf_ini_file.GetHash();
	wxString error = wxT("!");
	m_b_customized = false;
	GOrgueConfigReaderDB ini(m_Settings.GetODFCheck());
	ini.ReadData(odf_ini_file, ODFSetting, false);

	wxString setting_file = file2;

	if (setting_file.IsEmpty())
	{
		if (wxFileExists(m_SettingFilename))
		{
			setting_file = m_SettingFilename;
			m_b_customized = true;
		}
	}

	if (!setting_file.IsEmpty())
	{
		GOrgueConfigFileReader extra_odf_config;
		if (!extra_odf_config.Read(setting_file))
		{
			error.Printf(_("Unable to read '%s'"), setting_file.c_str());
			return error;
		}

		if (odf_ini_file.getEntry(wxT("Organ"), wxT("ChurchName")).Trim() == extra_odf_config.getEntry(wxT("Organ"), wxT("ChurchName")).Trim())
		{
			ini.ReadData(extra_odf_config, CMBSetting, false);
		}
		else
		{
			wxLogWarning(_("This combination file is only compatible with:\n%s"), extra_odf_config.getEntry(wxT("Organ"), wxT("ChurchName")).c_str());
		}
		wxString hash = extra_odf_config.getEntry(wxT("Organ"), wxT("ODFHash"));
		if (hash != wxEmptyString)
			if (hash != m_ODFHash)
			{
				if (wxMessageBox(_("The ODF does not match the combination file. Importing it can cause various problems. Should they really be imported?"), _("Import"), wxYES_NO, NULL) == wxNO)
				{
						ini.ClearCMB();
				}
			}
	}
	else
	{
		bool old_go_settings = ini.ReadData(odf_ini_file, CMBSetting, true);
		if (old_go_settings)
			if (wxMessageBox(_("The ODF contains GrandOrgue 0.2 styled saved settings. Should they be imported?"), _("Import"), wxYES_NO, NULL) == wxNO)
			{
				ini.ClearCMB();
			}
	}

	try
	{
		GOrgueConfigReader cfg(ini, m_Settings.GetODFCheck());
		/* skip informational items */
		cfg.ReadString(CMBSetting, wxT("Organ"), wxT("ChurchName"), false);
		cfg.ReadString(CMBSetting, wxT("Organ"), wxT("ChurchAddress"), false);
		cfg.ReadString(CMBSetting, wxT("Organ"), wxT("ODFPath"), false);
		cfg.ReadString(CMBSetting, wxT("Organ"), wxT("ODFHash"), false);
		ReadOrganFile(cfg);
	}
	catch (wxString error_)
	{
		return error_;
	}
	ini.ReportUnused();

	GOrgueLCD_WriteLineOne(m_ChurchName + wxT(" ") + m_OrganBuilder);
	GOrgueLCD_WriteLineTwo(_("Loading..."));

	void* dummy = NULL;

	try
	{
		dummy = malloc(1024 * 1024 * 50);
		if (!dummy)
			throw GOrgueOutOfMemory();

		wxString load_error;
		bool cache_ok = false;

		ResolveReferences();

		/* Figure out list of pipes to load */
		dlg->Reset(m_CacheObjects.size());
		/* Load pipes */
		std::atomic_uint nb_loaded_obj(0);

		if (wxFileExists(m_CacheFilename))
		{

			wxFile cache_file(m_CacheFilename);
			GOrgueCache reader(cache_file, m_pool);
			cache_ok = cache_file.IsOpened();

			if (cache_ok)
			{
				unsigned char hash1[20], hash2[20];
				if (!reader.ReadHeader())
				{
					cache_ok = false;
					wxLogWarning (_("Cache file had bad magic bypassing cache."));
				}
				GenerateCacheHash(hash1);
				if (!reader.Read(hash2, sizeof(hash2)) || 
				    memcmp(hash1, hash2, sizeof(hash1)))
				{
					cache_ok = false;
					reader.FreeCacheFile();
					wxLogWarning (_("Cache file had diffent hash bypassing cache."));
				}
			}

			if (cache_ok)
			{
				try
				{
					while (nb_loaded_obj < m_CacheObjects.size())
					{
						GOrgueCacheObject* obj = m_CacheObjects[nb_loaded_obj];
						if (!obj->LoadCache(reader))
						{
							cache_ok = false;
							wxLogError(_("Cache load failure: Failed to read %s from cache."), obj->GetLoadTitle().c_str());
							break;
						}
						nb_loaded_obj++;
						if (!dlg->Update (nb_loaded_obj, obj->GetLoadTitle()))
						{
							FREE_AND_NULL(dummy);
							SetTemperament(m_Temperament);
							GOMessageBox(_("Load aborted by the user - only parts of the organ are loaded.") , _("Load error"), wxOK | wxICON_ERROR, NULL);
							return wxEmptyString;
						}
					}
					if (nb_loaded_obj >= m_CacheObjects.size())
						m_Cacheable = true;
				}
				catch (wxString msg)
				{
					cache_ok = false;
					wxLogError(_("Cache load failure: %s"), msg.c_str());
				}
			}

			if (!cache_ok && !m_Settings.GetManageCache())
			{
				GOMessageBox(_("The cache for this organ is outdated. Please update or delete it."), _("Warning"), wxOK | wxICON_WARNING, NULL);
			}

			reader.Close();
		}

		if (!cache_ok)
		{
			ptr_vector<GOrgueLoadThread> threads;
			for(unsigned i = 0; i < m_Settings.GetLoadConcurrency(); i++)
				threads.push_back(new GOrgueLoadThread(m_CacheObjects, m_pool, nb_loaded_obj));

			for(unsigned i = 0; i < threads.size(); i++)
				threads[i]->Run();

			for(unsigned pos = nb_loaded_obj.fetch_add(1); pos < m_CacheObjects.size(); pos = nb_loaded_obj.fetch_add(1))
			{
				GOrgueCacheObject* obj = m_CacheObjects[pos];
				obj->LoadData();
				if (!dlg->Update (nb_loaded_obj, obj->GetLoadTitle()))
				{
					FREE_AND_NULL(dummy);
					SetTemperament(m_Temperament);
					GOMessageBox(_("Load aborted by the user - only parts of the organ are loaded.") , _("Load error"), wxOK | wxICON_ERROR, NULL);
					return wxEmptyString;
				}
			}

			for(unsigned i = 0; i < threads.size(); i++)
				threads[i]->checkResult();

			if (nb_loaded_obj >= m_CacheObjects.size())
				m_Cacheable = true;

			if (m_Settings.GetManageCache() && m_Cacheable)
				UpdateCache(dlg, m_Settings.GetCompressCache());
		}
		SetTemperament(m_Temperament);
	}
	catch (GOrgueOutOfMemory e)
	{
		FREE_AND_NULL(dummy);
		GOMessageBox(_("Out of memory - only parts of the organ are loaded. Please reduce memory footprint via the sample loading settings.") , _("Load error"), wxOK | wxICON_ERROR, NULL);
		return wxEmptyString;
	}
	catch (wxString error_)
	{
		FREE_AND_NULL(dummy);
		return error_;
	}
	FREE_AND_NULL(dummy);

	GOrgueLCD_WriteLineTwo(_("Ready!"));

	return wxEmptyString;

}

void GrandOrgueFile::LoadCombination(const wxString& file)
{
	try
	{
		GOrgueConfigFileReader odf_ini_file;

		if (!odf_ini_file.Read(file))
			throw wxString::Format(_("Unable to read '%s'"), file.c_str());

		GOrgueConfigReaderDB ini;
		ini.ReadData(odf_ini_file, CMBSetting, false);
		GOrgueConfigReader cfg(ini);

		wxString church_name = cfg.ReadString(CMBSetting, wxT("Organ"), wxT("ChurchName"));
		if (church_name != m_ChurchName)
			throw wxString::Format(_("File belongs to a different organ: %s"), church_name.c_str());
		wxString hash = odf_ini_file.getEntry(wxT("Organ"), wxT("ODFHash"));
		if (hash != wxEmptyString)
			if (hash != m_ODFHash)
			{
				wxLogError(_("The ODF does not match the combination file."));
			}
		/* skip informational items */
		cfg.ReadString(CMBSetting, wxT("Organ"), wxT("ChurchAddress"), false);
		cfg.ReadString(CMBSetting, wxT("Organ"), wxT("ODFPath"), false);

		ReadCombinations(cfg);
	}
	catch (wxString error)
	{
		wxLogError(wxT("%s\n"),error.c_str());
		GOMessageBox(error, _("Load error"), wxOK | wxICON_ERROR, NULL);
	}
}

bool GrandOrgueFile::CachePresent()
{
	return wxFileExists(m_CacheFilename);
}

bool GrandOrgueFile::UpdateCache(GOrgueProgressDialog* dlg, bool compress)
{
	DeleteCache();
	/* Figure out the list of pipes to save */
	unsigned nb_saved_objs = 0;

	dlg->Setup(m_CacheObjects.size(), _("Creating sample cache"));

	wxFileOutputStream file(m_CacheFilename);
	GOrgueCacheWriter writer(file, compress);

	/* Save pipes to cache */
	bool cache_save_ok = writer.WriteHeader();
	
	unsigned char hash[20];
	GenerateCacheHash(hash);
	if (!writer.Write(hash, sizeof(hash)))
		cache_save_ok = false;

	for (unsigned i = 0; cache_save_ok && i < m_CacheObjects.size(); i++)
	{
		GOrgueCacheObject* obj = m_CacheObjects[i];
		if (!obj->SaveCache(writer))
		{
			cache_save_ok = false;
			wxLogError(_("Save of %s to the cache failed"), obj->GetLoadTitle().c_str());
		}
		nb_saved_objs++;

		if (!dlg->Update (nb_saved_objs, obj->GetLoadTitle()))
		{
			writer.Close();
			DeleteCache();
			return false;
		}
	}

	writer.Close();
	if (!cache_save_ok)
	{
		DeleteCache();
		return false;
	}
	return true;
}

void GrandOrgueFile::DeleteCache()
{
	wxRemoveFile(m_CacheFilename);
}

GrandOrgueFile::~GrandOrgueFile(void)
{
	if (m_setter)
		delete m_setter;
	// Just to be sure, that the sound providers are freed before the pool
	m_manual.clear();
	m_tremulant.clear();
	m_ranks.clear();
}

void GrandOrgueFile::DeleteSettings()
{
	wxRemoveFile(m_SettingFilename);
}

bool GrandOrgueFile::Save()
{
	if (!Export(m_SettingFilename))
		return false;
	m_doc->Modify(false);
	return true;
}

bool GrandOrgueFile::Export(const wxString& cmb)
{
	wxString fn = cmb;
	wxString tmp_name = fn + wxT(".new");
	wxString buffer;
	bool prefix = false;

	GOrgueConfigFileWriter cfg_file;
	m_b_customized = true;

	GOrgueConfigWriter cfg(cfg_file, prefix);
	cfg.WriteString(wxT("Organ"), wxT("ODFHash"), m_ODFHash);
	cfg.WriteString(wxT("Organ"), wxT("ChurchName"), m_ChurchName);
	cfg.WriteString(wxT("Organ"), wxT("ChurchAddress"), m_ChurchAddress);
	cfg.WriteString(wxT("Organ"), wxT("ODFPath"), GetODFFilename());

	if (m_volume >= -120)
		cfg.WriteInteger(wxT("Organ"), wxT("Volume"), m_volume);

	cfg.WriteString(wxT("Organ"), wxT("Temperament"), m_Temperament);
	cfg.WriteBoolean(wxT("Organ"), wxT("IgnorePitch"), m_IgnorePitch);
	m_PipeConfig.Save(cfg);

	for (unsigned j = 0; j < m_ranks.size(); j++)
		m_ranks[j]->Save(cfg);

	for (unsigned i = m_FirstManual; i < m_manual.size(); i++)
		m_manual[i]->Save(cfg);

	for (unsigned j = 0; j < m_tremulant.size(); j++)
		m_tremulant[j]->Save(cfg);

	for (unsigned j = 0; j < m_divisionalcoupler.size(); j++)
		m_divisionalcoupler[j]->Save(cfg);

	for (unsigned j = 0; j < m_switches.size(); j++)
		m_switches[j]->Save(cfg);

	for (unsigned j = 0; j < m_general.size(); j++)
		m_general[j]->Save(cfg);

	if (m_setter)
		m_setter->Save(cfg);

	for (unsigned j = 0; j < m_piston.size(); j++)
		m_piston[j]->Save(cfg);

	for (unsigned j = 0; j < m_enclosure.size(); j++)
		m_enclosure[j]->Save(cfg);

	for(unsigned i = 0; i < m_panels.size(); i++)
		m_panels[i]->Save(cfg);

	m_PitchLabel.Save(cfg);
	m_TemperamentLabel.Save(cfg);

	if (::wxFileExists(tmp_name) && !::wxRemoveFile(tmp_name))
	{
		wxLogError(_("Could not write to '%s'"), tmp_name.c_str());
		return false;
	}
	if (!cfg_file.Save(tmp_name))
	{
		wxLogError(_("Could not write to '%s'"), tmp_name.c_str());
		return false;
	}
	if (!GORenameFile(tmp_name, fn))
		return false;
	return true;
}

GOrgueDocument* GrandOrgueFile::GetDocument()
{
	return m_doc;
}

void GrandOrgueFile::SetVolume(int volume)
{
	m_volume = volume;
}

int GrandOrgueFile::GetVolume()
{
	return m_volume;
}

void GrandOrgueFile::SetIgnorePitch(bool ignore)
{
	m_IgnorePitch = ignore;
}

bool GrandOrgueFile::GetIgnorePitch()
{
	return m_IgnorePitch;
}

unsigned GrandOrgueFile::GetFirstManualIndex()
{
	return m_FirstManual;
}

unsigned GrandOrgueFile::GetODFManualCount()
{
	return m_ODFManualCount;
}

unsigned GrandOrgueFile::GetManualAndPedalCount()
{
	if (!m_manual.size())
		return 0;
	return m_manual.size() - 1;
}

GOrgueManual* GrandOrgueFile::GetManual(unsigned index)
{
	return m_manual[index];
}

unsigned GrandOrgueFile::GetTremulantCount()
{
	return m_tremulant.size();
}

GOrgueTremulant* GrandOrgueFile::GetTremulant(unsigned index)
{
	return m_tremulant[index];
}

unsigned GrandOrgueFile::GetSwitchCount()
{
	return m_switches.size();
}

GOrgueSwitch* GrandOrgueFile::GetSwitch(unsigned index)
{
	return m_switches[index];
}

GOrgueRank* GrandOrgueFile::GetRank(unsigned index)
{
	return m_ranks[index];
}

unsigned GrandOrgueFile::GetRankCount()
{
	return m_RankCount;
}

void GrandOrgueFile::AddRank(GOrgueRank* rank)
{
	m_ranks.push_back(rank);
}

bool GrandOrgueFile::DivisionalsStoreIntermanualCouplers()
{
	return m_DivisionalsStoreIntermanualCouplers;
}

bool GrandOrgueFile::DivisionalsStoreIntramanualCouplers()
{
	return m_DivisionalsStoreIntramanualCouplers;
}

bool GrandOrgueFile::DivisionalsStoreTremulants()
{
	return m_DivisionalsStoreTremulants;
}

unsigned GrandOrgueFile::GetDivisionalCouplerCount()
{
	return m_divisionalcoupler.size();
}

GOrgueDivisionalCoupler* GrandOrgueFile::GetDivisionalCoupler(unsigned index)
{
	return m_divisionalcoupler[index];
}

bool GrandOrgueFile::CombinationsStoreNonDisplayedDrawstops()
{
	return m_CombinationsStoreNonDisplayedDrawstops;
}

unsigned GrandOrgueFile::GetNumberOfReversiblePistons()
{
	return m_piston.size();
}

GOrguePiston* GrandOrgueFile::GetPiston(unsigned index)
{
	return m_piston[index];
}

bool GrandOrgueFile::GeneralsStoreDivisionalCouplers()
{
	return m_GeneralsStoreDivisionalCouplers;
}

unsigned GrandOrgueFile::GetGeneralCount()
{
	return m_general.size();
}

GOrgueGeneral* GrandOrgueFile::GetGeneral(unsigned index)
{
	return m_general[index];
}

GOrgueSetter* GrandOrgueFile::GetSetter()
{
	return m_setter;
}

GOrgueWindchest* GrandOrgueFile::GetWindchest(unsigned index)
{
	return m_windchest[index];
}

unsigned GrandOrgueFile::GetWindchestGroupCount()
{
	return m_windchest.size();
}

unsigned GrandOrgueFile::AddWindchest(GOrgueWindchest* windchest)
{
	m_windchest.push_back(windchest);
	return m_windchest.size();
}

GOGUIPanel* GrandOrgueFile::GetPanel(unsigned index)
{
	return m_panels[index];
}

unsigned GrandOrgueFile::GetPanelCount()
{
	return m_panels.size();
}

const wxString& GrandOrgueFile::GetChurchName()
{
	return m_ChurchName;
}

const wxString& GrandOrgueFile::GetChurchAddress()
{
	return m_ChurchAddress;
}

const wxString& GrandOrgueFile::GetOrganBuilder()
{
	return m_OrganBuilder;
}

const wxString& GrandOrgueFile::GetOrganBuildDate()
{
	return m_OrganBuildDate;
}

const wxString& GrandOrgueFile::GetOrganComments()
{
	return m_OrganComments;
}

const wxString& GrandOrgueFile::GetRecordingDetails()
{
	return m_RecordingDetails;
}

const wxString& GrandOrgueFile::GetInfoFilename()
{
	return m_InfoFilename;
}

GOrgueEnclosure* GrandOrgueFile::GetEnclosure(unsigned index)
{
	return m_enclosure[index];
}

unsigned GrandOrgueFile::GetEnclosureCount()
{
	return m_enclosure.size();
}

unsigned GrandOrgueFile::AddEnclosure(GOrgueEnclosure* enclosure)
{
	m_enclosure.push_back(enclosure);
	return m_enclosure.size() - 1;
}

GOrguePipeConfig& GrandOrgueFile::GetPipeConfig()
{
	return m_PipeConfig;
}

void GrandOrgueFile::UpdateAmplitude()
{
	for (unsigned i = 0; i < m_ranks.size(); i++)
		m_ranks[i]->UpdateAmplitude();
}

void GrandOrgueFile::UpdateTuning()
{
	m_PitchLabel.SetName(wxString::Format(_("%f cent"), m_PipeConfig.GetTuning()));
	for (unsigned i = 0; i < m_ranks.size(); i++)
		m_ranks[i]->UpdateTuning();
}

void GrandOrgueFile::UpdateAudioGroup()
{
	for (unsigned i = 0; i < m_ranks.size(); i++)
		m_ranks[i]->UpdateAudioGroup();
}

bool GrandOrgueFile::IsCustomized()
{
	return m_b_customized;
}

const wxString GrandOrgueFile::GetODFFilename()
{
	return m_odf;
}

const wxString GrandOrgueFile::GetODFPath()
{
	return m_path.c_str();
}

const wxString GrandOrgueFile::GetSettingFilename()
{
	return m_SettingFilename;
}

const wxString GrandOrgueFile::GetCacheFilename()
{
	return m_CacheFilename;
}

GOrgueMemoryPool& GrandOrgueFile::GetMemoryPool()
{
	return m_pool;
}

GOrgueSettings& GrandOrgueFile::GetSettings()
{
	return m_Settings;
}

GOrgueBitmapCache& GrandOrgueFile::GetBitmapCache()
{
	return m_bitmaps;
}

SAMPLER_HANDLE GrandOrgueFile::StartSample(const GOSoundProvider *pipe, int sampler_group_id, unsigned audio_group, unsigned velocity, unsigned delay)
{
	if (!m_soundengine)
		return NULL;
	return m_soundengine->StartSample(pipe, sampler_group_id, audio_group, velocity, delay);
}

void GrandOrgueFile::StopSample(const GOSoundProvider *pipe, SAMPLER_HANDLE handle)
{
	if (m_soundengine)
		m_soundengine->StopSample(pipe, handle);
}

void GrandOrgueFile::SwitchSample(const GOSoundProvider *pipe, SAMPLER_HANDLE handle)
{
	if (m_soundengine)
		m_soundengine->SwitchSample(pipe, handle);
}

void GrandOrgueFile::UpdateVelocity(SAMPLER_HANDLE handle, unsigned velocity)
{
	if (m_soundengine)
		m_soundengine->UpdateVelocity(handle, velocity);
}

void GrandOrgueFile::SendMidiMessage(GOrgueMidiEvent& e)
{
	if (m_midi)
		m_midi->Send(e);
}

void GrandOrgueFile::SendMidiRecorderMessage(GOrgueMidiEvent& e)
{
	if (m_midi)
		m_midi->GetMidiRecorder().SendMidiRecorderMessage(e);
}

void GrandOrgueFile::AddMidiListener(GOrgueMidiListener* listener)
{
	if (m_midi)
		listener->Register(m_midi);
}

void GrandOrgueFile::Abort()
{
	m_soundengine = NULL;

	for (unsigned i = m_FirstManual; i < m_manual.size(); i++)
		m_manual[i]->Abort();

	for (unsigned i = 0; i < m_enclosure.size(); i++)
		m_enclosure[i]->Abort();

	for (unsigned i = 0; i < m_switches.size(); i++)
		m_switches[i]->Abort();

	for (unsigned i = 0; i < m_piston.size(); i++)
		m_piston[i]->Abort();

	for (unsigned i = 0; i < m_general.size(); i++)
		m_general[i]->Abort();

	for (unsigned i = 0; i < m_divisionalcoupler.size(); i++)
		m_divisionalcoupler[i]->Abort();

	for (unsigned i = 0; i < m_switches.size(); i++)
		m_switches[i]->Abort();

	for (unsigned i = 0; i < m_ranks.size(); i++)
		m_ranks[i]->Abort();

	for (unsigned i = 0; i < m_tremulant.size(); i++)
		m_tremulant[i]->Abort();
	
	m_setter->Abort();
	m_PitchLabel.Abort();
	m_TemperamentLabel.Abort();

	m_midi = NULL;
}

void GrandOrgueFile::PreparePlayback(GOSoundEngine* engine, GOrgueMidi* midi)
{
	m_soundengine = engine;
	m_midi = midi;

	m_midi->GetMidiRecorder().Clear();
	UpdateAudioGroup();

	for (unsigned i = 0; i < m_ranks.size(); i++)
		m_ranks[i]->PreparePlayback();

	for (unsigned i = 0; i < m_switches.size(); i++)
		m_switches[i]->PreparePlayback();

	for (unsigned i = m_FirstManual; i < m_manual.size(); i++)
		m_manual[i]->PreparePlayback();

	for (unsigned i = 0; i < m_enclosure.size(); i++)
		m_enclosure[i]->PreparePlayback();

	for (unsigned i = 0; i < m_tremulant.size(); i++)
		m_tremulant[i]->PreparePlayback();

	for (unsigned i = 0; i < m_piston.size(); i++)
		m_piston[i]->PreparePlayback();

	for (unsigned i = 0; i < m_general.size(); i++)
		m_general[i]->PreparePlayback();

	for (unsigned i = 0; i < m_divisionalcoupler.size(); i++)
		m_divisionalcoupler[i]->PreparePlayback();

	for (unsigned i = 0; i < m_windchest.size(); i++)
		m_windchest[i]->PreparePlayback();

	m_setter->PreparePlayback();
	m_PitchLabel.PreparePlayback();
	m_TemperamentLabel.PreparePlayback();
}

void GrandOrgueFile::PrepareRecording()
{
	m_midi->GetMidiRecorder().Clear();

	for (unsigned i = 0; i < m_switches.size(); i++)
		m_switches[i]->PrepareRecording();

	for (unsigned i = m_FirstManual; i < m_manual.size(); i++)
		m_manual[i]->PrepareRecording();

	for (unsigned i = 0; i < m_enclosure.size(); i++)
		m_enclosure[i]->PrepareRecording();

	for (unsigned i = 0; i < m_tremulant.size(); i++)
		m_tremulant[i]->PrepareRecording();

	m_setter->PrepareRecording();
	m_PitchLabel.PrepareRecording();
	m_TemperamentLabel.PrepareRecording();
}

void GrandOrgueFile::Update()
{
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

void GrandOrgueFile::UpdateVolume()
{
	for (unsigned i = 0; i < m_windchest.size(); i++)
		m_windchest[i]->UpdateVolume();
}

void GrandOrgueFile::ProcessMidi(const GOrgueMidiEvent& event)
{
	if (event.GetMidiType() == MIDI_RESET)
	{
		Reset();
		return;
	}

	for(unsigned i = 0; i < m_handler.size(); i++)
		m_handler[i]->ProcessMidi(event);
}

void GrandOrgueFile::HandleKey(int key)
{
	for(unsigned i = 0; i < m_handler.size(); i++)
		m_handler[i]->HandleKey(key);
}

void GrandOrgueFile::Reset()
{
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

void GrandOrgueFile::SetTemperament(const GOrgueTemperament& temperament)
{
	m_TemperamentLabel.SetName(wxGetTranslation(temperament.GetName()));
        for (unsigned k = 0; k < m_ranks.size(); k++)
		m_ranks[k]->SetTemperament(temperament);
}

void GrandOrgueFile::SetTemperament(wxString name)
{
	const GOrgueTemperament& temperament = GOrgueTemperament::GetTemperament(name);
	m_Temperament = temperament.GetName();
	SetTemperament(temperament);
}

wxString GrandOrgueFile::GetTemperament()
{
	return m_Temperament;
}

void GrandOrgueFile::ControlChanged(void* control)
{
	if (!control)
		return;
	for(unsigned i = 0; i < m_panels.size(); i++)
		m_panels[i]->ControlChanged(control);
	for(unsigned i = 0; i < m_piston.size(); i++)
		m_piston[i]->ControlChanged(control);
	m_setter->ControlChanged(control);
}

void GrandOrgueFile::RegisterEventHandler(GOrgueEventHandler* handler)
{
	m_handler.push_back(handler);
}

void GrandOrgueFile::RegisterCacheObject(GOrgueCacheObject* obj)
{
	m_CacheObjects.push_back(obj);
}

void GrandOrgueFile::UpdateTremulant(GOrgueTremulant* tremulant)
{
	for(unsigned i = 0; i < m_windchest.size(); i++)
		m_windchest[i]->UpdateTremulant(tremulant);
}

void GrandOrgueFile::AllNotesOff()
{
        for (unsigned k = GetFirstManualIndex(); k <= GetManualAndPedalCount(); k++)
		GetManual(k)->AllNotesOff();
}

void GrandOrgueFile::Modified()
{
	m_doc->Modify(true);
}

int GrandOrgueFile::GetRecorderElementID(wxString name)
{
	return m_Settings.GetMidiMap().GetElementByString(name);
}

GOrgueCombinationDefinition& GrandOrgueFile::GetGeneralTemplate()
{
	return m_GeneralTemplate;
}

GOrgueLabel* GrandOrgueFile::GetPitchLabel()
{
	return &m_PitchLabel;
}

GOrgueLabel* GrandOrgueFile::GetTemperamentLabel()
{
	return &m_TemperamentLabel;
}

void GrandOrgueFile::MarkSectionInUse(wxString name)
{
	if (m_UsedSections[name])
		throw wxString::Format(_("Section %s already in use"), name.c_str());
	m_UsedSections[name] = true;
}
