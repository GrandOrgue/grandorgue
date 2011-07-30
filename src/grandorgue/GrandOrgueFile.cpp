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
#include <wx/image.h>
#include <wx/mstream.h>
#include <wx/stream.h>
#include <wx/wfstream.h>
#include <wx/zstream.h>

#include "IniFileConfig.h"
#include "GrandOrgue.h"
#include "GrandOrgueFrame.h"
#include "GOrgueCoupler.h"
#include "GOrgueDisplayMetrics.h"
#include "GOrgueDivisional.h"
#include "GOrgueDivisionalCoupler.h"
#include "GOrgueEnclosure.h"
#include "GOrgueFrameGeneral.h"
#include "GOrgueGeneral.h"
#include "GOrgueLabel.h"
#include "GOrgueMeter.h"
#include "GOrguePipe.h"
#include "GOrguePiston.h"
#include "GOrguePushbutton.h"
#include "GOrgueSound.h"
#include "GOrgueStop.h"
#include "GOrgueThread.h"
#include "GOrgueTremulant.h"
#include "GOrgueWave.h"
#include "GOrgueWindchest.h"
#include "OrganDocument.h"

extern GOrgueSound* g_sound;
GrandOrgueFile* organfile = 0;
extern const unsigned char* ImageLoader_Stops[];
extern int c_ImageLoader_Stops[];

GrandOrgueFile::GrandOrgueFile() :
	m_path(),
	m_b_squash(0),
	m_compress_p(NULL),
	m_cfg(NULL),
	m_filename(),
	m_elapsed(0),
	m_b_customized(false),
	m_DivisionalsStoreIntermanualCouplers(false),
	m_DivisionalsStoreIntramanualCouplers(false),
	m_DivisionalsStoreTremulants(false),
	m_GeneralsStoreDivisionalCouplers(false),
	m_CombinationsStoreNonDisplayedDrawstops(false),
	m_HighestSampleFormat(0),
	m_FirstManual(0),
	m_NumberOfManuals(0),
	m_NumberOfEnclosures(0),
	m_NumberOfTremulants(0),
	m_NumberOfWindchestGroups(0),
	m_NumberOfReversiblePistons(0),
	m_NumberOfLabels(0),
	m_NumberOfGenerals(0),
	m_NumberOfFrameGenerals(0),
	m_NumberOfDivisionalCouplers(0),
	m_AmplitudeLevel(0),
    m_HauptwerkOrganFileFormatVersion(),
	m_ChurchName(),
	m_ChurchAddress(),
	m_OrganBuilder(),
	m_OrganBuildDate(),
	m_OrganComments(),
	m_RecordingDetails(),
    m_InfoFilename(),
    m_DisplayMetrics(NULL),
	m_enclosure(NULL),
	m_tremulant(NULL),
	m_windchest(NULL),
	m_piston(NULL),
	m_label(NULL),
	m_general(NULL),
	m_framegeneral(NULL),
	m_divisionalcoupler(NULL),
	m_pipe(NULL)
{
	for (int i = 0; i < 9; ++i)
	{
		m_images[i] = wxBitmap();
	}
	for (int i = 0; i < 7; ++i)
	{
		m_manual[i] = GOrgueManual();
	}
}

bool GrandOrgueFile::TryLoad
	(wxInputStream* cache
	,wxProgressDialog& dlg
	,wxString& error
	)
{

	bool success = true;

	try
	{

		/* Figure out how many pipes there are */
		unsigned nb_pipes = 0;
		for (int i = m_FirstManual; i <= m_NumberOfManuals; i++)
			for (int j = 0; j < m_manual[i].GetStopCount(); j++)
				nb_pipes += m_manual[i].GetStop(j)->GetPipeCount();

		/* Load pipes */
		unsigned nb_loaded_pipes = 0;
		for (int i = m_FirstManual; i <= m_NumberOfManuals; i++)
			for (int j = 0; j < m_manual[i].GetStopCount(); j++)
				for (unsigned k = 0; k < m_manual[i].GetStop(j)->GetPipeCount(); k++)
				{
					GOrguePipe* pipe = m_manual[i].GetStop(j)->GetPipe(k);
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
					dlg.Update
						((nb_loaded_pipes << 15) / (nb_pipes + 1)
						,pipe->GetFilename()
						);
	#ifdef __VFD__
					GOrgueLCD_WriteLineTwo
						(wxString::Format
							("Loading %d%%"
							,(nb_loaded_pipes * 100) / (nb_pipes + 1);
							)
						);
	#endif
				}

	}
	catch (wxString msg)
	{
		error = msg;
		success = false;
	}

	return success;

}

void GrandOrgueFile::readOrganFile()
{
	IniFileConfig ini(m_cfg);
	wxString group = wxT("Organ");

	/* load all GUI display metrics */
	m_DisplayMetrics = new GOrgueDisplayMetrics(ini);

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
	m_NumberOfManuals = ini.ReadInteger(group, wxT("NumberOfManuals"), 1, 6);
	m_FirstManual = ini.ReadBoolean(group, wxT("HasPedals")) ? 0 : 1;
	m_NumberOfEnclosures = ini.ReadInteger(group, wxT("NumberOfEnclosures"), 0, 6);
	m_NumberOfTremulants = ini.ReadInteger(group, wxT("NumberOfTremulants"), 0, 10);
	m_NumberOfWindchestGroups = ini.ReadInteger(group, wxT("NumberOfWindchestGroups"), 1, 12);
	m_NumberOfReversiblePistons = ini.ReadInteger(group, wxT("NumberOfReversiblePistons"), 0, 32);
	m_NumberOfLabels = ini.ReadInteger(group, wxT("NumberOfLabels"), 0, 16);
	m_NumberOfGenerals = ini.ReadInteger(group, wxT("NumberOfGenerals"), 0, 99);
	m_NumberOfFrameGenerals = 999;	// we never want this to change, what's the point?
	m_NumberOfDivisionalCouplers = ini.ReadInteger(group, wxT("NumberOfDivisionalCouplers"), 0, 8);
	m_AmplitudeLevel = ini.ReadInteger(group, wxT("AmplitudeLevel"), 0, 1000);
	m_DivisionalsStoreIntermanualCouplers = ini.ReadBoolean(group, wxT("DivisionalsStoreIntermanualCouplers"));
	m_DivisionalsStoreIntramanualCouplers = ini.ReadBoolean(group, wxT("DivisionalsStoreIntramanualCouplers"));
	m_DivisionalsStoreTremulants = ini.ReadBoolean(group, wxT("DivisionalsStoreTremulants"));
	m_GeneralsStoreDivisionalCouplers = ini.ReadBoolean(group, wxT("GeneralsStoreDivisionalCouplers"));
	m_CombinationsStoreNonDisplayedDrawstops = ini.ReadBoolean(group, wxT("CombinationsStoreNonDisplayedDrawstops"));

	wxString buffer;

	for (int i = m_FirstManual; i <= m_NumberOfManuals; i++)
	{
		buffer.Printf(wxT("Manual%03d"), i);
		m_manual[i].Load(ini, buffer, m_DisplayMetrics, i);
	}

	m_enclosure = new GOrgueEnclosure[m_NumberOfEnclosures];
	for (int i = 0; i < m_NumberOfEnclosures; i++)
		m_enclosure[i].Load(ini, i, m_DisplayMetrics);

	m_windchest = new GOrgueWindchest[m_NumberOfTremulants + 1 + m_NumberOfWindchestGroups];
	for (int i = 0; i < m_NumberOfWindchestGroups; i++)
	{
		buffer.Printf(wxT("WindchestGroup%03d"), i + 1);
		m_windchest[m_NumberOfTremulants + 1 + i].Load(ini, buffer);
	}

	m_tremulant = new GOrgueTremulant[m_NumberOfTremulants];
	for (int i = 0; i < m_NumberOfTremulants; i++)
	{
		buffer.Printf(wxT("Tremulant%03d"), i + 1);
		m_tremulant[i].Load(ini, buffer, m_DisplayMetrics);
	}

	m_piston = new GOrguePiston[m_NumberOfReversiblePistons];
	for (int i = 0; i < m_NumberOfReversiblePistons; i++)
	{
		buffer.Printf(wxT("ReversiblePiston%03d"), i + 1);
		m_piston[i].Load(ini, buffer, m_DisplayMetrics);
	}

	m_label = new GOrgueLabel[m_NumberOfLabels];
	for (int i = 0; i < m_NumberOfLabels; i++)
	{
		buffer.Printf(wxT("Label%03d"), i + 1);
		m_label[i].Load(ini, buffer, m_DisplayMetrics);
	}

	m_general = new GOrgueGeneral[m_NumberOfGenerals];
	for (int i = 0; i < m_NumberOfGenerals; i++)
	{
		buffer.Printf(wxT("General%03d"), i + 1);
		m_general[i].Load(ini, buffer, m_DisplayMetrics);
	}

	m_framegeneral = new GOrgueFrameGeneral[m_NumberOfFrameGenerals];
	for (int i = 0; i < m_NumberOfFrameGenerals; i++)
	{
		buffer.Printf(wxT("FrameGeneral%03d"), i + 1);
		m_framegeneral[i].Load(ini, buffer);
	}

	m_divisionalcoupler = new GOrgueDivisionalCoupler[m_NumberOfDivisionalCouplers];
	for (int i = 0; i < m_NumberOfDivisionalCouplers; i++)
	{
		buffer.Printf(wxT("DivisionalCoupler%03d"), i + 1);
		m_divisionalcoupler[i].Load(ini, buffer, m_FirstManual, m_NumberOfManuals, m_DisplayMetrics);
	}

}

wxString GrandOrgueFile::Load(const wxString& file, const wxString& file2)
{
	wxFileConfig *cfg = 0, *extra_cfg = 0;

	wxProgressDialog dlg(_("Loading sample set"), _("Parsing sample set definition file"), 32768, 0, wxPD_AUTO_HIDE | wxPD_CAN_ABORT | wxPD_APP_MODAL | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME);

	m_filename = file;
	m_b_squash = wxConfigBase::Get()->Read(wxT("LosslessCompression"), 1);

	wxString key, key2, error = wxT("!");

  	// NOTICE: unfortunately, the format is not adhered to well at all. with logging enabled, most sample sets generate warnings.
    wxLog::EnableLogging(false);
    m_cfg = cfg = new wxFileConfig(wxEmptyString, wxEmptyString, wxEmptyString, file, wxCONFIG_USE_GLOBAL_FILE | wxCONFIG_USE_NO_ESCAPE_CHARACTERS, wxCSConv(wxT("ISO-8859-1")));
    wxLog::EnableLogging(true);

    if (!cfg->GetNumberOfGroups())
	{
		error.Printf(_("Unable to read '%s'"), file.c_str());
		return error;
	}

	m_b_customized = false;
    long cookie;
    bool bCont = cfg->GetFirstGroup(key, cookie);
    while (bCont)
    {
        if (key.StartsWith(wxT("_")))
        {
            m_b_customized = true;
            cfg->SetPath(wxT('/') + key);
            long cookie2;
            bool bCont2 = cfg->GetFirstEntry(key2, cookie2);
            while (bCont2)
            {
                cfg->Write(wxT('/') + key.Mid(1) + wxT('/') + key2, cfg->Read(key2));
                bCont2 = cfg->GetNextEntry(key2, cookie2);
            }
            cfg->SetPath(wxT("/"));
        }
        bCont = cfg->GetNextGroup(key, cookie);
    }

	if (!file2.IsEmpty())
	{
	    // NOTICE: unfortunately, the format is not adhered to well at all. with logging enabled, most sample sets generate warnings.
        wxLog::EnableLogging(false);
		extra_cfg = new wxFileConfig(wxEmptyString, wxEmptyString, wxEmptyString, file2, wxCONFIG_USE_GLOBAL_FILE | wxCONFIG_USE_NO_ESCAPE_CHARACTERS, wxCSConv(wxT("ISO-8859-1")));
        wxLog::EnableLogging(true);

		key = wxT("/Organ/ChurchName");
		if (cfg->Read(key).Trim() == extra_cfg->Read(key).Trim())
		{
			long cookie;
			bool bCont = extra_cfg->GetFirstGroup(key, cookie);
			while (bCont)
			{
				extra_cfg->SetPath(wxT('/') + key);
				long cookie2;
				bool bCont2 = extra_cfg->GetFirstEntry(key2, cookie2);
				while (bCont2)
				{
					cfg->Write(key + wxT('/') + key2, extra_cfg->Read(key2));
					bCont2 = extra_cfg->GetNextEntry(key2, cookie2);
				}
				extra_cfg->SetPath(wxT("/"));
				bCont = extra_cfg->GetNextGroup(key, cookie);
			}
		}
		else
		{
			if (!extra_cfg->GetNumberOfGroups())
			{
				error.Printf(_("Unable to read '%s'"), file2.c_str());
				return error;
			}
			::wxLogWarning(_("This combination file is only compatible with:\n%s"), extra_cfg->Read(key).c_str());
		}
		delete extra_cfg;
	}

	try
	{
		readOrganFile();
	}
	catch (wxString error_)
	{
		return error_;
	}

#ifdef __VFD__
    GOrgueLCD_WriteLineOne(ChurchName+" "+OrganBuilder);
    GOrgueLCD_WriteLineTwo("Loading...");
#endif
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
			}

			if (cache_ok)
			{
				if (!TryLoad(&zin, dlg, load_error))
				{
					cache_ok = false;
					wxLogError(_("Cache load failure: %s"), load_error.c_str());
				}
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

	/* FIXME: Load tremulants */
/*	for (int i = 0; i < m_NumberOfTremulants; i++)
	{

		wxLogDebug(_("Loading tremulant #%d"), i);

		m_pipe[progress]->CreateFromTremulant(&m_tremulant[i]);
		m_tremulant[i].pipe = m_pipe[progress];
		m_pipe[progress]->SetWindchestGroup(i);
		progress++;

	}*/

	/* TODO: ? check for correctness ? */
	/* Load the images for the stops */
	for (int i = 0; i < 9; i++)
	{

		wxMemoryInputStream mem((const char*)ImageLoader_Stops[i], c_ImageLoader_Stops[i]);
		wxImage img(mem, wxBITMAP_TYPE_PNG);
		m_images[i] = wxBitmap(img);

	}

	if (m_cfg)
	{
		::wxGetApp().frame->m_meters[0]->SetValue(m_cfg->Read(wxT("/Organ/Volume"), g_sound->GetVolume()));
		delete m_cfg;
		m_cfg = 0;
	}

#ifdef __VFD__
	GOrgueLCD_WriteLineTwo("Ready!");
#endif
	return wxEmptyString;

}

GrandOrgueFile::~GrandOrgueFile(void)
{

	/*
	 * This code used to delete the tremulants, but because of the code below
	 * and because they are stored in pipe objects, it is not necessary... but
	 * should come back at some point, because tremulants should not be stored
	 * in the pipes array... it doesn't make sense.
	 *
	for (int i = 0; i < m_NumberOfTremulants; i++)
        if (m_tremulant && m_tremulant[i].pipe)
            free(m_tremulant[i].pipe);
	*/

	if (m_divisionalcoupler)
		delete[] m_divisionalcoupler;
	if (m_framegeneral)
		delete[] m_framegeneral;
	if (m_general)
		delete[] m_general;
	if (m_label)
		delete[] m_label;
	if (m_piston)
		delete[] m_piston;
	if (m_windchest)
		delete[] m_windchest;
	if (m_tremulant)
		delete[] m_tremulant;
	if (m_enclosure)
		delete[] m_enclosure;
	if (m_cfg)
		delete m_cfg;
	if (m_DisplayMetrics)
		delete m_DisplayMetrics;

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
	wxFileConfig cfg(wxEmptyString, wxEmptyString, file, wxEmptyString, wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_NO_ESCAPE_CHARACTERS, wxCSConv(wxT("ISO-8859-1")));
	wxLog::EnableLogging(true);
	if (prefix)
		Revert(cfg);
	m_b_customized = true;

	IniFileConfig aIni(&cfg);
	aIni.SaveHelper(prefix, wxT("Organ"), wxT("ChurchName"), m_ChurchName);
	aIni.SaveHelper(prefix, wxT("Organ"), wxT("ChurchAddress"), m_ChurchAddress);
	aIni.SaveHelper(prefix, wxT("Organ"), wxT("HauptwerkOrganFileFormatVersion"), m_HauptwerkOrganFileFormatVersion);
	aIni.SaveHelper(prefix, wxT("Organ"), wxT("NumberOfFrameGenerals"), m_NumberOfFrameGenerals);
	aIni.SaveHelper(prefix, wxT("Organ"), wxT("Volume"), g_sound->GetVolume());

    int i, j;

	for (i = m_FirstManual; i <= m_NumberOfManuals; i++)
	{
		buffer.Printf(wxT("Manual%03d"), i);
		m_manual[i].Save(aIni, prefix, buffer);
	}
	for (j = 0; j < m_NumberOfTremulants; j++)
	{
		buffer.Printf(wxT("Tremulant%03d"), j + 1);
		m_tremulant[j].Save(aIni, prefix, buffer);
	}
	for (j = 0; j < m_NumberOfDivisionalCouplers; j++)
	{
		buffer.Printf(wxT("DivisionalCoupler%03d"), j + 1);
		m_divisionalcoupler[j].Save(aIni, prefix, buffer);
	}
	for (j = 0; j < m_NumberOfGenerals; j++)
	{
		buffer.Printf(wxT("General%03d"), j + 1);
		m_general[j].Save(aIni, prefix, buffer);
	}
	for (j = 0; j < m_NumberOfFrameGenerals; j++)
	{
		buffer.Printf(wxT("FrameGeneral%03d"), j + 1);
		m_framegeneral[j].Save(aIni, prefix, buffer);
	}
	for (j = 0; j < m_NumberOfReversiblePistons; j++)
	{
		buffer.Printf(wxT("ReversiblePiston%03d"), j + 1);
		m_piston[j].Save(aIni, prefix, buffer);
	}

}

int GrandOrgueFile::GetFirstManualIndex()
{
	return m_FirstManual;
}

int GrandOrgueFile::GetManualAndPedalCount()
{
	return m_NumberOfManuals;
}

GOrgueManual* GrandOrgueFile::GetManual(unsigned index)
{
	return &m_manual[index];
}

int GrandOrgueFile::GetTremulantCount()
{
	return m_NumberOfTremulants;
}

GOrgueTremulant* GrandOrgueFile::GetTremulant(unsigned index)
{
	return &m_tremulant[index];
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

int GrandOrgueFile::GetDivisionalCouplerCount()
{
	return m_NumberOfDivisionalCouplers;
}

GOrgueDivisionalCoupler* GrandOrgueFile::GetDivisionalCoupler(unsigned index)
{
	return &m_divisionalcoupler[index];
}

bool GrandOrgueFile::CombinationsStoreNonDisplayedDrawstops()
{
	return m_CombinationsStoreNonDisplayedDrawstops;
}

GOrgueDisplayMetrics* GrandOrgueFile::GetDisplayMetrics()
{
	return m_DisplayMetrics;
}

wxBitmap* GrandOrgueFile::GetImage(unsigned index)
{
	return &m_images[index];
}

int GrandOrgueFile::GetNumberOfReversiblePistons()
{
	return m_NumberOfReversiblePistons;
}

GOrguePiston* GrandOrgueFile::GetPiston(unsigned index)
{
	return &m_piston[index];
}

bool GrandOrgueFile::GeneralsStoreDivisionalCouplers()
{
	return m_GeneralsStoreDivisionalCouplers;
}

int GrandOrgueFile::GetGeneralCount()
{
	return m_NumberOfGenerals;
}

GOrgueGeneral* GrandOrgueFile::GetGeneral(unsigned index)
{
	return &m_general[index];
}

GOrguePipe* GrandOrgueFile::GetPipe(unsigned index)
{
	return m_pipe[index];
}

GOrgueFrameGeneral* GrandOrgueFile::GetFrameGeneral(unsigned index)
{
	return &m_framegeneral[index];
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
	return &m_windchest[index];
}

int GrandOrgueFile::GetWinchestGroupCount()
{
	return m_NumberOfWindchestGroups;
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
	return &m_enclosure[index];
}

unsigned GrandOrgueFile::GetEnclosureCount()
{
	return m_NumberOfEnclosures;
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

GOrgueLabel* GrandOrgueFile::GetLabel(unsigned index)
{
	return &m_label[index];
}

int GrandOrgueFile::GetLabelCount()
{
	return m_NumberOfLabels;
}

void GrandOrgueFile::MIDIPretend(bool on)
{
	for (int i = m_FirstManual; i <= m_NumberOfManuals; i++)
		m_manual[i].MIDIPretend(on);
}

