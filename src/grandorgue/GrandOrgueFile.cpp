/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "GrandOrgueFile.h"

#include <math.h>
#include <wx/progdlg.h>
#include <wx/stream.h>
#include <wx/wfstream.h>
#include <wx/zstream.h>

#include "IniFileConfig.h"
#include "GrandOrgue.h"
#include "GrandOrgueFrame.h"
#include "GOrgueCoupler.h"
#include "GOrgueDivisional.h"
#include "GOrgueDivisionalCoupler.h"
#include "GOrgueEnclosure.h"
#include "GOrgueGeneral.h"
#include "GOrgueLCD.h"
#include "GOrgueManual.h"
#include "GOrgueMeter.h"
#include "GOrguePipe.h"
#include "GOrguePiston.h"
#include "GOrguePushbutton.h"
#include "GOrgueReleaseAlignTable.h"
#include "GOrgueSetter.h"
#include "GOrgueSound.h"
#include "GOrgueStop.h"
#include "GOrgueTremulant.h"
#include "GOrgueWindchest.h"
#include "OrganDocument.h"
#include "GOGUIPanel.h"
#include "contrib/sha1.h"

extern GOrgueSound* g_sound;

GrandOrgueFile::GrandOrgueFile(OrganDocument* doc) :
	m_doc(doc),
	m_path(),
	m_b_squash(0),
	m_filename(),
	m_elapsed(0),
	m_setter(0),
	m_b_customized(false),
	m_DivisionalsStoreIntermanualCouplers(false),
	m_DivisionalsStoreIntramanualCouplers(false),
	m_DivisionalsStoreTremulants(false),
	m_GeneralsStoreDivisionalCouplers(false),
	m_CombinationsStoreNonDisplayedDrawstops(false),
	m_HighestSampleFormat(0),
	m_FirstManual(0),
	m_AmplitudeLevel(0),
	m_HauptwerkOrganFileFormatVersion(),
	m_ChurchName(),
	m_ChurchAddress(),
	m_OrganBuilder(),
	m_OrganBuildDate(),
	m_OrganComments(),
	m_RecordingDetails(),
	m_InfoFilename(),
	m_enclosure(0),
	m_tremulant(0),
	m_windchest(0),
	m_piston(0),
	m_general(0),
	m_divisionalcoupler(0),
	m_manual(0),
	m_panels(0)
{
}

void GrandOrgueFile::GenerateCacheHash(unsigned char hash[20])
{
	SHA_CTX ctx;
	int len;
	SHA1_Init(&ctx);
	for (unsigned i = m_FirstManual; i < m_manual.size(); i++)
		for (unsigned j = 0; j < m_manual[i]->GetStopCount(); j++)
		{
			unsigned stop_amplitude = m_manual[i]->GetStop(j)->GetAmplitude();
			SHA1_Update(&ctx, &stop_amplitude, sizeof(stop_amplitude));
			for (unsigned k = 0; k < m_manual[i]->GetStop(j)->GetPipeCount(); k++)
			{
				wxString filename = m_manual[i]->GetStop(j)->GetPipe(k)->GetFilename();
				SHA1_Update(&ctx, filename.c_str(), (filename.Length() + 1) * sizeof(wxChar));
			}
		}

	SHA1_Update(&ctx, &m_AmplitudeLevel, sizeof(m_AmplitudeLevel));

	len = sizeof(AUDIO_SECTION);
	SHA1_Update(&ctx, &len, sizeof(len));
	len = sizeof(GOrguePipe);
	SHA1_Update(&ctx, &len, sizeof(len));
	len = sizeof(GOrgueReleaseAlignTable);
	SHA1_Update(&ctx, &len, sizeof(len));
	len = BLOCKS_PER_FRAME;
	SHA1_Update(&ctx, &len, sizeof(len));
	len = BLOCK_HISTORY;
	SHA1_Update(&ctx, &len, sizeof(len));

	SHA1_Final(hash, &ctx);
}


bool GrandOrgueFile::TryLoad
	(wxInputStream* cache
	,wxProgressDialog& dlg
	,wxString& error
	)
{

	bool success = false;

	try
	{

		/* Figure out how many pipes there are */
		unsigned nb_pipes = 0;
		for (unsigned i = m_FirstManual; i < m_manual.size(); i++)
			for (unsigned j = 0; j < m_manual[i]->GetStopCount(); j++)
				nb_pipes += m_manual[i]->GetStop(j)->GetPipeCount();

		/* Load pipes */
		unsigned nb_loaded_pipes = 0;
		for (unsigned i = m_FirstManual; i < m_manual.size(); i++)
			for (unsigned j = 0; j < m_manual[i]->GetStopCount(); j++)
				for (unsigned k = 0; k < m_manual[i]->GetStop(j)->GetPipeCount(); k++)
				{
					GOrguePipe* pipe = m_manual[i]->GetStop(j)->GetPipe(k);
					if (cache != NULL)
					{
						if (!pipe->LoadCache(cache))
						{
							error = wxString::Format
								(_("Failed to read %s from cache.")
								,pipe->GetFilename().c_str()
								);
							return false;
						}
					}
					else
					{
						pipe->LoadData();
					}
					nb_loaded_pipes++;
					if (!dlg.Update	((nb_loaded_pipes << 15) / (nb_pipes + 1), pipe->GetFilename()))
					{
						error = _("Load aborted by the user");
						return false;
					}
					GOrgueLCD_WriteLineTwo
						(wxString::Format
							(_("Loading %d%%")
							,(nb_loaded_pipes * 100) / (nb_pipes + 1))
						);
				}

		success = true;

	}
	catch (wxString msg)
	{
		error = msg;
	}

	return success;

}

void GrandOrgueFile::ReadOrganFile(wxFileConfig& odf_ini_file)
{

	IniFileConfig ini(odf_ini_file);
	wxString group = wxT("Organ");

	/* load church info */
	m_HauptwerkOrganFileFormatVersion = ini.ReadString(group, wxT("HauptwerkOrganFileFormatVersion"),  256, false);
	m_ChurchName = ini.ReadString(group, wxT("ChurchName"),  128);
	m_ChurchAddress = ini.ReadString(group, wxT("ChurchAddress"),  128);
	m_OrganBuilder = ini.ReadString(group, wxT("OrganBuilder"),  128, false);
	m_OrganBuildDate = ini.ReadString(group, wxT("OrganBuildDate"),  128, false);
	m_OrganComments = ini.ReadString(group, wxT("OrganComments"),  256, false);
	m_RecordingDetails = ini.ReadString(group, wxT("RecordingDetails"),  256, false);
	m_InfoFilename = ini.ReadString(group, wxT("InfoFilename"),  256, false);
	wxFileName fn = m_filename;
	if (m_InfoFilename.IsEmpty())
		fn.SetExt(wxT(".html"));
	else
		fn.SetFullName(m_InfoFilename);
	if (fn.FileExists())
		m_InfoFilename = fn.GetFullPath();
	else
		m_InfoFilename = wxEmptyString;

	/* load basic organ information */
	unsigned m_NumberOfManuals = ini.ReadInteger(group, wxT("NumberOfManuals"), 1, 6);
	m_FirstManual = ini.ReadBoolean(group, wxT("HasPedals")) ? 0 : 1;
	unsigned m_NumberOfEnclosures = ini.ReadInteger(group, wxT("NumberOfEnclosures"), 0, 6);
	unsigned m_NumberOfTremulants = ini.ReadInteger(group, wxT("NumberOfTremulants"), 0, 10);
	unsigned m_NumberOfWindchestGroups = ini.ReadInteger(group, wxT("NumberOfWindchestGroups"), 1, 12);
	unsigned m_NumberOfReversiblePistons = ini.ReadInteger(group, wxT("NumberOfReversiblePistons"), 0, 32);
	unsigned m_NumberOfGenerals = ini.ReadInteger(group, wxT("NumberOfGenerals"), 0, 99);
	unsigned m_NumberOfDivisionalCouplers = ini.ReadInteger(group, wxT("NumberOfDivisionalCouplers"), 0, 8);
	m_AmplitudeLevel = ini.ReadInteger(group, wxT("AmplitudeLevel"), 0, 1000);
	m_DivisionalsStoreIntermanualCouplers = ini.ReadBoolean(group, wxT("DivisionalsStoreIntermanualCouplers"));
	m_DivisionalsStoreIntramanualCouplers = ini.ReadBoolean(group, wxT("DivisionalsStoreIntramanualCouplers"));
	m_DivisionalsStoreTremulants = ini.ReadBoolean(group, wxT("DivisionalsStoreTremulants"));
	m_GeneralsStoreDivisionalCouplers = ini.ReadBoolean(group, wxT("GeneralsStoreDivisionalCouplers"));
	m_CombinationsStoreNonDisplayedDrawstops = ini.ReadBoolean(group, wxT("CombinationsStoreNonDisplayedDrawstops"));

	wxString buffer;

	m_windchest.resize(0);
	for (unsigned i = 0; i < m_NumberOfWindchestGroups; i++)
		m_windchest.push_back(new GOrgueWindchest(this));

	m_manual.resize(0);
	m_manual.resize(m_FirstManual); // Add empty slot for pedal, if necessary
	for (unsigned int i = m_FirstManual; i <= m_NumberOfManuals; i++)
		m_manual.push_back(new GOrgueManual(this));

	m_enclosure.resize(0);
	for (unsigned i = 0; i < m_NumberOfEnclosures; i++)
	{
		m_enclosure.push_back(new GOrgueEnclosure(this));
		buffer.Printf(wxT("Enclosure%03u"), i + 1);
		m_enclosure[i]->Load(ini, buffer, i);
	}

	m_tremulant.resize(0);
	for (unsigned i = 0; i < m_NumberOfTremulants; i++)
	{
		m_tremulant.push_back(new GOrgueTremulant(this));
		buffer.Printf(wxT("Tremulant%03d"), i + 1);
		m_tremulant[i]->Load(ini, buffer, i + 1);
	}

	for (unsigned  i = 0; i < m_NumberOfWindchestGroups; i++)
	{
		buffer.Printf(wxT("WindchestGroup%03d"), i + 1);
		m_windchest[i]->Load(ini, buffer);
	}

	for (unsigned int i = m_FirstManual; i <= m_NumberOfManuals; i++)
	{
		buffer.Printf(wxT("Manual%03d"), i);
		m_manual[i]->Load(ini, buffer, i);
	}

	m_piston.resize(0);
	for (unsigned i = 0; i < m_NumberOfReversiblePistons; i++)
	{
		m_piston.push_back(new GOrguePiston(this));
		buffer.Printf(wxT("ReversiblePiston%03d"), i + 1);
		m_piston[i]->Load(ini, buffer);
	}

	m_divisionalcoupler.resize(0);
	for (unsigned i = 0; i < m_NumberOfDivisionalCouplers; i++)
	{
		m_divisionalcoupler.push_back(new GOrgueDivisionalCoupler(this));
		buffer.Printf(wxT("DivisionalCoupler%03d"), i + 1);
		m_divisionalcoupler[i]->Load(ini, buffer);
	}

	m_general.resize(0);
	for (unsigned i = 0; i < m_NumberOfGenerals; i++)
	{
		m_general.push_back(new GOrgueGeneral(this));
		buffer.Printf(wxT("General%03d"), i + 1);
		m_general[i]->Load(ini, buffer);
	}

	m_setter = new GOrgueSetter(this);
	m_setter->Load(ini);

	m_panels.resize(0);
	m_panels.push_back(new GOGUIPanel(this));
	m_panels[0]->Load(ini, wxT("Organ"));

	m_panels.push_back(m_setter->CreateGeneralsPanel(ini));
	m_panels.push_back(m_setter->CreateSetterPanel(ini));
}

wxString GrandOrgueFile::Load(const wxString& file, const wxString& file2)
{

	wxProgressDialog dlg
		(_("Loading sample set")
		,_("Parsing sample set definition file")
		,32768
		,0
		,wxPD_AUTO_HIDE | wxPD_CAN_ABORT | wxPD_APP_MODAL | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME
		);

	m_filename = file;
	m_b_squash = wxConfigBase::Get()->Read(wxT("LosslessCompression"), 1);

	// NOTICE: unfortunately, the format is not adhered to well at all. With
	// logging enabled, most sample sets generate warnings.
	wxLog::EnableLogging(false);
	wxFileConfig odf_ini_file
		(wxEmptyString
		,wxEmptyString
		,wxEmptyString
		,file
		,wxCONFIG_USE_GLOBAL_FILE | wxCONFIG_USE_NO_ESCAPE_CHARACTERS
		,wxCSConv(wxT("ISO-8859-1"))
		);
	wxLog::EnableLogging(true);

	if (!odf_ini_file.GetNumberOfGroups())
	{
		wxString error;
		error.Printf(_("Unable to read '%s'"), file.c_str());
		return error;
	}
	wxString key, key2, error = wxT("!");

	m_b_customized = false;
    long cookie;
    bool bCont = odf_ini_file.GetFirstGroup(key, cookie);
    while (bCont)
    {
        if (key.StartsWith(wxT("_")))
        {
            m_b_customized = true;
            odf_ini_file.SetPath(wxT('/') + key);
            long cookie2;
            bool bCont2 = odf_ini_file.GetFirstEntry(key2, cookie2);
            while (bCont2)
            {
                odf_ini_file.Write(wxT('/') + key.Mid(1) + wxT('/') + key2, odf_ini_file.Read(key2));
                bCont2 = odf_ini_file.GetNextEntry(key2, cookie2);
            }
            odf_ini_file.SetPath(wxT("/"));
        }
        bCont = odf_ini_file.GetNextGroup(key, cookie);
    }

	if (!file2.IsEmpty())
	{

		// NOTICE: unfortunately, the format is not adhered to well at all.
		// with logging enabled, most sample sets generate warnings.
		wxLog::EnableLogging(false);
		wxFileConfig extra_odf_config
			(wxEmptyString
			,wxEmptyString
			,wxEmptyString
			,file2
			,wxCONFIG_USE_GLOBAL_FILE | wxCONFIG_USE_NO_ESCAPE_CHARACTERS
			,wxCSConv(wxT("ISO-8859-1"))
			);
		wxLog::EnableLogging(true);

		key = wxT("/Organ/ChurchName");
		if (odf_ini_file.Read(key).Trim() == extra_odf_config.Read(key).Trim())
		{
			long cookie;
			bool bCont = extra_odf_config.GetFirstGroup(key, cookie);
			while (bCont)
			{
				extra_odf_config.SetPath(wxT('/') + key);
				long cookie2;
				bool bCont2 = extra_odf_config.GetFirstEntry(key2, cookie2);
				while (bCont2)
				{
					odf_ini_file.Write(key + wxT('/') + key2, extra_odf_config.Read(key2));
					bCont2 = extra_odf_config.GetNextEntry(key2, cookie2);
				}
				extra_odf_config.SetPath(wxT("/"));
				bCont = extra_odf_config.GetNextGroup(key, cookie);
			}
		}
		else
		{
			if (!extra_odf_config.GetNumberOfGroups())
			{
				error.Printf(_("Unable to read '%s'"), file2.c_str());
				return error;
			}
			::wxLogWarning(_("This combination file is only compatible with:\n%s"), extra_odf_config.Read(key).c_str());
		}
	}

	try
	{
		ReadOrganFile(odf_ini_file);
	}
	catch (wxString error_)
	{
		return error_;
	}

	GOrgueLCD_WriteLineOne(m_ChurchName + wxT(" ") + m_OrganBuilder);
	GOrgueLCD_WriteLineTwo(_("Loading..."));

	m_path = file;
	m_path.MakeAbsolute();
	wxString temp;
	m_path.SetCwd(m_path.GetPath());

	try
	{

		wxString cache_filename = file + wxT(".cache");
		wxString load_error;
		bool cache_ok = false;

		if (wxFileExists(cache_filename))
		{

			wxFileInputStream cache(file + wxT(".cache"));
			wxZlibInputStream zin(cache);

			cache_ok = cache.IsOk() && zin.IsOk();

			if (cache_ok)
			{
				int magic;
				unsigned char hash1[20], hash2[20];
				zin.Read(&magic, sizeof(magic));
				if (
						(zin.LastRead() != sizeof(magic))
						||
						(magic != GRANDORGUE_CACHE_MAGIC)
					)
				{
					cache_ok = false;
					wxLogWarning
						(_("Cache file had bad magic (expected %#.8x got %#.8x) bypassing cache.")
						,GRANDORGUE_CACHE_MAGIC
						,magic
						);
				}
				GenerateCacheHash(hash1);
				zin.Read(hash2, sizeof(hash2));
				if (zin.LastRead() != sizeof(hash2) ||
				    memcmp(hash1, hash2, sizeof(hash1)))
				{
					cache_ok = false;
					wxLogWarning (_("Cache file had diffent hash bypassing cache."));
				}
			}

			if (cache_ok)
			{
				if (!TryLoad(&zin, dlg, load_error))
				{
					cache_ok = false;
					wxLogError(_("Cache load failure: %s"), load_error.c_str());
				}
			}

			if (!cache_ok)
			{
				wxMessageBox(_("The cache for this organ is outdated. Please update or delete it."), _("Warning"), wxOK | wxICON_WARNING, NULL);
			}
		}

		if (!cache_ok)
		{
			if (!TryLoad(NULL, dlg, load_error))
				return load_error;
		}

	}
	catch (wxString error_)
	{
		return error_;
	}

	::wxGetApp().frame->m_meters[0]->SetValue(odf_ini_file.Read(wxT("/Organ/Volume"), g_sound->GetVolume()));

	GOrgueLCD_WriteLineTwo(_("Ready!"));

	return wxEmptyString;

}
bool GrandOrgueFile::CachePresent()
{
	wxString cache_filename = m_filename + wxT(".cache");
	return wxFileExists(cache_filename);
}
bool GrandOrgueFile::UpdateCache()
{
	wxString cache_filename = m_filename + wxT(".cache");

	/* Figure out how many pipes there are */
	unsigned nb_pipes = 0;
	unsigned nb_saved_pipes = 0;
	for (unsigned i = GetFirstManualIndex(); i <= GetManualAndPedalCount(); i++)
		for (unsigned j = 0; j < GetManual(i)->GetStopCount(); j++)
			nb_pipes += GetManual(i)->GetStop(j)->GetPipeCount();

	wxProgressDialog dlg(_("Creating sample cache"), wxEmptyString, 32768, 0, wxPD_AUTO_HIDE | wxPD_CAN_ABORT | wxPD_APP_MODAL | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME);
	wxFileOutputStream file(cache_filename);
	wxZlibOutputStream zout(file);

	/* Save pipes to cache */
	bool cache_save_ok = true;
	int magic = GRANDORGUE_CACHE_MAGIC;
	zout.Write(&magic, sizeof(magic));
	if (zout.LastWrite() != sizeof(magic))
		cache_save_ok = false;
	
	unsigned char hash[20];
	GenerateCacheHash(hash);
	zout.Write(hash, sizeof(hash));
	if (zout.LastWrite() != sizeof(hash))
		cache_save_ok = false;

	for (unsigned i = GetFirstManualIndex(); cache_save_ok && i <= GetManualAndPedalCount(); i++)
		for (unsigned j = 0; cache_save_ok && j < GetManual(i)->GetStopCount(); j++)
			for (unsigned k = 0; cache_save_ok && k < GetManual(i)->GetStop(j)->GetPipeCount(); k++)
			{
				GOrguePipe* pipe = GetManual(i)->GetStop(j)->GetPipe(k);
				if (!pipe->SaveCache(&zout))
				{
					cache_save_ok = false;
					wxLogError(_("Save of %s to the cache failed"), pipe->GetFilename().c_str());
				}
				nb_saved_pipes++;
				if (!dlg.Update ((nb_saved_pipes << 15) / (nb_pipes + 1), pipe->GetFilename()))
				{
					zout.Close();
					file.Close();
					DeleteCache();
					return false;
				}
			}
	zout.Close();
	file.Close();
	if (!cache_save_ok)
	{
		DeleteCache();
		return false;
	}
	return true;
}

void GrandOrgueFile::DeleteCache()
{
	wxString cache_filename = m_filename + wxT(".cache");
	wxRemoveFile(cache_filename);
}

GrandOrgueFile::~GrandOrgueFile(void)
{
	if (m_setter)
		delete m_setter;
}

void GrandOrgueFile::Revert(wxFileConfig& cfg)
{
    if (!m_b_customized)
        return;
    long cookie;
    wxString key;
    bool bCont = cfg.GetFirstGroup(key, cookie);
    wxArrayString to_drop;
    while (bCont)
    {
        if (key.StartsWith(wxT("_")))
            to_drop.Add(key);
        bCont = cfg.GetNextGroup(key, cookie);
    }
    for (unsigned i = 0; i < to_drop.Count(); i++)
        cfg.DeleteGroup(to_drop[i]);
}

void GrandOrgueFile::Save(const wxString& file)
{

	wxFileName fn(file);
	wxString buffer;
	bool prefix = true;

	if (fn.GetExt().CmpNoCase(wxT("organ")))
	{
		if (::wxFileExists(file) && !::wxRemoveFile(file))
		{
			::wxLogError(_("Could not write to '%s'"), file.c_str());
			return;
		}
		prefix = false;
	}

	wxLog::EnableLogging(false);
	wxFileConfig cfg
		(wxEmptyString
		,wxEmptyString
		,file
		,wxEmptyString
		,wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_NO_ESCAPE_CHARACTERS
		,wxCSConv(wxT("ISO-8859-1"))
		);
	wxLog::EnableLogging(true);

	if (prefix)
		Revert(cfg);
	m_b_customized = true;

	IniFileConfig aIni(cfg);
	aIni.SaveHelper(prefix, wxT("Organ"), wxT("ChurchName"), m_ChurchName);
	aIni.SaveHelper(prefix, wxT("Organ"), wxT("ChurchAddress"), m_ChurchAddress);
	aIni.SaveHelper(prefix, wxT("Organ"), wxT("HauptwerkOrganFileFormatVersion"), m_HauptwerkOrganFileFormatVersion);
	aIni.SaveHelper(prefix, wxT("Organ"), wxT("Volume"), g_sound->GetVolume());

	for (unsigned i = m_FirstManual; i < m_manual.size(); i++)
		m_manual[i]->Save(aIni, prefix);

	for (unsigned j = 0; j < m_tremulant.size(); j++)
		m_tremulant[j]->Save(aIni, prefix);

	for (unsigned j = 0; j < m_divisionalcoupler.size(); j++)
		m_divisionalcoupler[j]->Save(aIni, prefix);

	for (unsigned j = 0; j < m_general.size(); j++)
		m_general[j]->Save(aIni, prefix);

	if (m_setter)
		m_setter->Save(aIni, prefix);

	for (unsigned j = 0; j < m_piston.size(); j++)
		m_piston[j]->Save(aIni, prefix);

	for (unsigned j = 0; j < m_enclosure.size(); j++)
		m_enclosure[j]->Save(aIni, prefix);

	for(unsigned i = 0; i < m_panels.size(); i++)
		m_panels[i]->Save(aIni, prefix);
}

unsigned GrandOrgueFile::GetFirstManualIndex()
{
	return m_FirstManual;
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

long GrandOrgueFile::GetElapsedTime()
{
	return m_elapsed;
}

void GrandOrgueFile::SetElapsedTime(long elapsed)
{
	m_elapsed = elapsed;
}

GOrgueWindchest* GrandOrgueFile::GetWindchest(unsigned index)
{
	return m_windchest[index];
}

unsigned GrandOrgueFile::GetWinchestGroupCount()
{
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

int GrandOrgueFile::GetAmplitude()
{
	return m_AmplitudeLevel;
}

bool GrandOrgueFile::IsCustomized()
{
	return m_b_customized;
}

const wxString& GrandOrgueFile::GetODFFilename()
{
	return m_filename;
}

void GrandOrgueFile::Abort()
{
	for (unsigned i = m_FirstManual; i < m_manual.size(); i++)
		m_manual[i]->Abort();

	for (unsigned i = 0; i < m_tremulant.size(); i++)
		m_tremulant[i]->Abort();
}

void GrandOrgueFile::PreparePlayback()
{
	for (unsigned i = m_FirstManual; i < m_manual.size(); i++)
		m_manual[i]->PreparePlayback();

	for (unsigned j = 0; j < m_tremulant.size(); j++)
		m_tremulant[j]->PreparePlayback();

	m_setter->PreparePlayback();
}

void GrandOrgueFile::ProcessMidi(const GOrgueMidiEvent& event)
{
	for(unsigned i = 0; i < m_enclosure.size(); i++)
		m_enclosure[i]->ProcessMidi(event);

	for(unsigned i = 0; i < m_tremulant.size(); i++)
		m_tremulant[i]->ProcessMidi(event);

	for(unsigned i = 0; i < m_piston.size(); i++)
		m_piston[i]->ProcessMidi(event);

	for(unsigned i = 0; i < m_general.size(); i++)
		m_general[i]->ProcessMidi(event);

	for(unsigned i = 0; i < m_divisionalcoupler.size(); i++)
		m_divisionalcoupler[i]->ProcessMidi(event);

	for(unsigned i = m_FirstManual; i < m_manual.size(); i++)
		m_manual[i]->ProcessMidi(event);
	
	m_setter->ProcessMidi(event);
}

void GrandOrgueFile::Reset()
{
        for (unsigned k = GetFirstManualIndex(); k <= GetManualAndPedalCount(); k++)
		GetManual(k)->Reset();
        for (unsigned l = 0; l < GetTremulantCount(); l++)
		GetTremulant(l)->Set(false);
        for (unsigned j = 0; j < GetDivisionalCouplerCount(); j++)
		     GetDivisionalCoupler(j)->Set(false);
        for (unsigned k = 0; k < GetGeneralCount(); k++)
		GetGeneral(k)->Display(false);
}

void GrandOrgueFile::ControlChanged(void* control)
{
	if (!control)
		return;
	for(unsigned i = 0; i < m_panels.size(); i++)
		m_panels[i]->ControlChanged(control);
	for(unsigned i = 0; i < m_piston.size(); i++)
		m_piston[i]->ControlChanged(control);
}

void GrandOrgueFile::Modified()
{
	m_doc->Modify(true);
}
