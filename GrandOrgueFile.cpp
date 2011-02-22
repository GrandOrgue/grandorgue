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
#include "GOrgueWindchest.h"
#include "OrganDocument.h"
#include "OrganFile.h"
#include "zlib.h"

extern GOrgueSound* g_sound;
GrandOrgueFile* organfile = 0;
extern const unsigned char* ImageLoader_Stops[];
extern int c_ImageLoader_Stops[];

GrandOrgueFile::GrandOrgueFile() :
	m_pipe_filenames(),
	m_pipe_filesizes(),
	m_pipe_files(),
	m_pipe_ptrs(),
	m_pipe_windchests(),
	m_pipe_percussive(),
	m_pipe_amplitudes(),
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
	m_NumberOfStops(0),
	m_NumberOfPipes(0),
	m_AmplitudeLevel(0),
    m_HauptwerkOrganFileFormatVersion(),
	m_ChurchName(),
	m_ChurchAddress(),
	m_OrganBuilder(),
	m_OrganBuildDate(),
	m_OrganComments(),
	m_RecordingDetails(),
    m_InfoFilename(),
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
		m_images[i]=wxBitmap();
	}
	for (int i = 0; i < 7; ++i)
	{
		m_manual[i]=GOrgueManual();
	}
}



void GrandOrgueFile::CompressWAV(char*& compress, short* fv, short* ptr, int count, int channels, int stage)
{
	int f[4] = {0, 0, 0, 0}, v[4] = {0, 0, 0, 0}, a[4] = {0, 0, 0, 0};
	int size = 0, index;
	char* origlength;
	int i, j, count2;

	// not recommended, but if they want to, reduce stereo to mono
	if (channels == 2 && !(g_sound->IsStereo()))
	{
		for (int i = 0, j = 0; i < count; i += 2)
			ptr[j++] = ((int)ptr[i] + (int)ptr[i + 1]) >> 1;
		count >>= 1;
		channels = 1;
	}

	int maxsearch = 2206 * channels;	// ~20Hz maximum search width for phase alignment table
	if (maxsearch > count)
		maxsearch = count - (count & channels);

	origlength = compress;
	m_compress_p->types[stage] = (channels - 1) << 1;

	for (i = 0; i < 4; i++)
	{
		j = i >> (2 - channels);
		if (count > channels * 2)
            v[i] = fv[i + 12] = ptr[j + channels * 2] - ptr[j];
		else
			v[i] = fv[i + 12] = 0;
		f[i] = fv[i] = ptr[j] - v[i];
	}

	bool flags[PHASE_ALIGN_RES_VA];
	if (stage == 2)
	{
		for (j = 0; j < PHASE_ALIGN_RES_VA; j++)
		{
			flags[j] = false;
			m_compress_p->ra_offset[j] = 0;
			for (i = 0; i < 4; i++)
			{
				m_compress_p->ra_f[j][i] = f[i];
				m_compress_p->ra_v[j][i] = v[i];
			}
		}

		int prev_index = m_compress_p->ra_getindex( f, v), prev_i = 0;
		for (i = 0, j = 0; i < maxsearch; )
		{
			v[j] = ptr[i] - f[j];
			f[j] = ptr[i];
			j++;
			if (channels == 1)
			{
				v[j] = v[j-1];
				f[j] = f[j-1];
				j++;
			}
			j &= 3;
			i++;
			if (!j)
			{
				index = m_compress_p->ra_getindex( f, v);
				if(index>=PHASE_ALIGN_RES_VA)
				  std::cout << index << ":index out of bound\n";
				if (index != prev_index)
				{
					if (!m_compress_p->ra_offset[prev_index])
						m_compress_p->ra_offset[prev_index] = ((prev_i + i) >> (channels + 1)) << (channels + 1);
					prev_i = i;
					prev_index = index;
				}
			}
		}

		for (i = 0; i < 4; i++)
		{
			f[i] = fv[i];
			v[i] = fv[i + 12];
		}
	}

	if (!m_b_squash)
		goto CantCompress;

	count2 = count + (count & channels);
	if (count & channels)
		for (j = 0; j < channels; j++)
			*(ptr + count + j) = 2 * (int)*(ptr + count + j - 2 * channels) - (int)*(ptr + count + j - 4 * channels);

	if (channels == 2)
	{
		for (i = 0; i < count2; )
		{
			int value;
			value = ptr[i] - f[0];
			a[0] = value - v[0];
			v[0] = value;
			f[0] = ptr[i++];
			value = ptr[i] - f[1];
			a[1] = value - v[1];
			v[1] = value;
			f[1] = ptr[i++];
			value = ptr[i] - f[2];
			a[2] = value - v[2];
			v[2] = value;
			f[2] = ptr[i++];
			value = ptr[i] - f[3];
			a[3] = value - v[3];
			v[3] = value;
			f[3] = ptr[i++];

            if (((((v[0] + 32768) | (v[1] + 32768) | (v[2] + 32768) | (v[3] + 32768)) >> 16) | (((a[0] + 16384) | (a[1] + 16384) | (a[2] + 16384) | (a[3] + 16384)) >> 15)) && i < count)
                goto CantCompress;
            if ( (((a[0] + 128) | (a[1] + 128) | (a[2] + 128)) >> 8) | ((a[3] + 64) >> 7))
            {
                *(int*)(compress     ) = ((a[3] & 0xFF00) << 17) | ((a[2] & 0xFF00) <<  9) | ((a[1] & 0xFF00) <<  1) | ((a[0] & 0xFF00) >>  7);
                *(int*)(compress += 4) = ((a[3] & 0x00FF) << 24) | ((a[2] & 0x00FF) << 16) | ((a[1] & 0x00FF) <<  8) | ((a[0] & 0x00FF)      );
                size += i <= count ? 8 : 4;
            }
            else
            {
                *(int*)(compress     ) = ((a[3] & 0x00FF) << 25) | ((a[2] & 0x00FF) << 17) | ((a[1] & 0x00FF) <<  9) | ((a[0] & 0x00FF) <<  1) | 1;
                size += i <= count ? 4 : 2;
            }
            compress += 4;

            if (i < maxsearch && stage == 2)
            {
                index = m_compress_p->ra_getindex( f, v);
                if (!flags[index] && m_compress_p->ra_offset[index] == i << 1)
                {
                    flags[index] = true;
                    m_compress_p->ra_offset[index] = size;
                    for (j = 0; j < 4; j++)
                    {
                        m_compress_p->ra_f[index][j] = f[j];
                        m_compress_p->ra_v[index][j] = v[j];
                    }
                }
            }
		}
	}
	else
	{
		for (i = 0; i < count2; )
		{
			int value;
			value = ptr[i] - f[0];
			a[0] = value - v[0];
			v[0] = value;
			f[0] = ptr[i++];
			value = ptr[i] - f[2];
			a[2] = value - v[2];
			v[2] = value;
			f[2] = ptr[i++];

            if (((((v[0] + 32768) | (v[2] + 32768)) >> 16) | (((a[0] + 16384) | (a[2] + 16384)) >> 15)) && i < count)
                goto CantCompress;

            if (((a[0] + 128) >> 8) | ((a[2] + 64) >> 7))
            {
                *(short*)(compress     ) = ((a[2] & 0xFF00) <<  1) | ((a[0] & 0xFF00) >>  7);
                *(short*)(compress += 2) = ((a[2] & 0x00FF) <<  8) | ((a[0] & 0x00FF)      );
                size += i <= count ? 4 : 2;
            }
            else
            {
                *(short*)(compress     ) = ((a[2] & 0x00FF) <<  9) | ((a[0] & 0x00FF) <<  1) | 1;
                size += i <= count ? 2 : 1;
            }
            compress += 2;

            if (i < maxsearch && stage == 2)
            {
                f[1] = f[0];
                f[3] = f[2];
                v[1] = v[0];
                v[3] = v[2];
                index = m_compress_p->ra_getindex( f, v);
                if (!flags[index] && m_compress_p->ra_offset[index] == i << 1)
                {
                    flags[index] = true;
                    m_compress_p->ra_offset[index] = size;
                    for (j = 0; j < 4; j++)
                    {
                        m_compress_p->ra_f[index][j] = f[j];
                        m_compress_p->ra_v[index][j] = v[j];
                    }
                }
            }
		}
	}
	compress += channels << 1;

	goto Done;

CantCompress:
	m_compress_p->types[stage] |= 1;

	compress = origlength;
	memcpy(compress, ptr, sizeof(short) * count);
	memcpy(compress+(sizeof(short) * count),ptr, sizeof(short) * (count & channels));
	compress=compress+(sizeof(short)*((count&channels) + count));

Done:
	m_compress_p->ptr[stage]    = (wxByte*)((wxByte*)(origlength + size) - (wxByte*)m_compress_p);
	m_compress_p->offset[stage] = -size;
	if (stage == 2)
		for (j = 0; j < PHASE_ALIGN_RES_VA; j++)
			m_compress_p->ra_offset[j] -= size;
}

void GrandOrgueFile::readOrganFile()
{
	IniFileConfig ini(m_cfg);
	const char group[] = "Organ";

	/* load all GUI display metrics */
	m_DisplayMetrics = new GOrgueDisplayMetrics(ini);

	/* load church info */
	m_HauptwerkOrganFileFormatVersion = ini.ReadString( group,"HauptwerkOrganFileFormatVersion",  256, false);
	m_ChurchName = ini.ReadString( group,"ChurchName",  128);
	m_ChurchAddress = ini.ReadString( group,"ChurchAddress",  128);
	m_OrganBuilder = ini.ReadString( group,"OrganBuilder",  128, false);
	m_OrganBuildDate = ini.ReadString( group,"OrganBuildDate",  128, false);
	m_OrganComments = ini.ReadString( group,"OrganComments",  256, false);
	m_RecordingDetails = ini.ReadString( group,"RecordingDetails",  256, false);
	m_InfoFilename = ini.ReadString( group,"InfoFilename",  256, false);
	wxFileName fn = m_filename;
	if (m_InfoFilename.IsEmpty())
		fn.SetExt(".html");
	else
		fn.SetFullName(m_InfoFilename);
	if (fn.FileExists())
		m_InfoFilename = fn.GetFullPath();
	else
		m_InfoFilename = wxEmptyString;

	/* load basic organ information */
	m_NumberOfManuals = ini.ReadInteger(group, "NumberOfManuals", 1, 6);
	m_FirstManual = ini.ReadBoolean(group, "HasPedals") ? 0 : 1;
	m_NumberOfEnclosures = ini.ReadInteger(group, "NumberOfEnclosures", 0, 6);
	m_NumberOfTremulants = ini.ReadInteger(group, "NumberOfTremulants", 0, 10);
	m_NumberOfWindchestGroups = ini.ReadInteger(group, "NumberOfWindchestGroups", 1, 12);
	m_NumberOfReversiblePistons = ini.ReadInteger(group, "NumberOfReversiblePistons", 0, 32);
	m_NumberOfLabels = ini.ReadInteger(group, "NumberOfLabels", 0, 16);
	m_NumberOfGenerals = ini.ReadInteger(group, "NumberOfGenerals", 0, 99);
	m_NumberOfFrameGenerals = 512;	// we never want this to change, what's the point?
	m_NumberOfDivisionalCouplers = ini.ReadInteger(group, "NumberOfDivisionalCouplers", 0, 8);
	m_AmplitudeLevel = ini.ReadInteger(group, "AmplitudeLevel", 0, 1000);
	m_DivisionalsStoreIntermanualCouplers = ini.ReadBoolean(group, "DivisionalsStoreIntermanualCouplers");
	m_DivisionalsStoreIntramanualCouplers = ini.ReadBoolean(group, "DivisionalsStoreIntramanualCouplers");
	m_DivisionalsStoreTremulants = ini.ReadBoolean(group, "DivisionalsStoreTremulants");
	m_GeneralsStoreDivisionalCouplers = ini.ReadBoolean(group, "GeneralsStoreDivisionalCouplers");
	m_CombinationsStoreNonDisplayedDrawstops = ini.ReadBoolean(group, "CombinationsStoreNonDisplayedDrawstops");

	char buffer[64];

	for (int i = m_FirstManual; i <= m_NumberOfManuals; i++)
	{
		sprintf(buffer, "Manual%03d", i);
		m_manual[i].Load(ini, buffer, m_DisplayMetrics, i);
	}

	m_enclosure = new GOrgueEnclosure[m_NumberOfEnclosures];
	for (int i = 0; i < m_NumberOfEnclosures; i++)
	{
		sprintf(buffer, "Enclosure%03d", i + 1);
		m_enclosure[i].Load(ini, buffer, m_DisplayMetrics);
	}

	m_windchest = new GOrgueWindchest[m_NumberOfTremulants + 1 + m_NumberOfWindchestGroups];
	for (int i = 0; i < m_NumberOfWindchestGroups; i++)
	{
		sprintf(buffer, "WindchestGroup%03d", i + 1);
		m_windchest[m_NumberOfTremulants + 1 + i].Load(ini, buffer);
	}

	m_tremulant = new GOrgueTremulant[m_NumberOfTremulants];
	for (int i = 0; i < m_NumberOfTremulants; i++)
	{
		sprintf(buffer, "Tremulant%03d", i + 1);
		m_tremulant[i].Load(ini, buffer, m_DisplayMetrics);
	}

	m_piston = new GOrguePiston[m_NumberOfReversiblePistons];
	for (int i = 0; i < m_NumberOfReversiblePistons; i++)
	{
		sprintf(buffer, "ReversiblePiston%03d", i + 1);
		m_piston[i].Load(ini, buffer, m_DisplayMetrics);
	}

	m_label = new GOrgueLabel[m_NumberOfLabels];
	for (int i = 0; i < m_NumberOfLabels; i++)
	{
		sprintf(buffer, "Label%03d", i + 1);
		m_label[i].Load(ini, buffer, m_DisplayMetrics);
	}

	m_general = new GOrgueGeneral[m_NumberOfGenerals];
	for (int i = 0; i < m_NumberOfGenerals; i++)
	{
		sprintf(buffer, "General%03d", i + 1);
		m_general[i].Load(ini, buffer, m_DisplayMetrics);
	}

	m_framegeneral = new GOrgueFrameGeneral[m_NumberOfFrameGenerals];
	for (int i = 0; i < m_NumberOfFrameGenerals; i++)
	{
		sprintf(buffer, "General%03d", i + 100);
		m_framegeneral[i].Load(ini, buffer);
		m_framegeneral[i].ObjectNumber = i + 100;
	}

	m_divisionalcoupler = new GOrgueDivisionalCoupler[m_NumberOfDivisionalCouplers];
	for (int i = 0; i < m_NumberOfDivisionalCouplers; i++)
	{
		sprintf(buffer, "DivisionalCoupler%03d", i + 1);
		m_divisionalcoupler[i].Load(ini, buffer, m_FirstManual, m_NumberOfManuals, m_DisplayMetrics);
	}

}

wxString GrandOrgueFile::Load(const wxString& file, const wxString& file2)
{
	wxFileConfig *cfg = 0, *extra_cfg = 0;

	wxProgressDialog dlg(_("Loading sample set"), _("Parsing sample set definition file"), 32768, 0, wxPD_AUTO_HIDE | wxPD_CAN_ABORT | wxPD_APP_MODAL | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME);

	m_filename = file;
	m_b_squash = wxConfigBase::Get()->Read("LosslessCompression", 1);

	wxString key, key2, error = "!";

  	// NOTICE: unfortunately, the format is not adhered to well at all. with logging enabled, most sample sets generate warnings.
    wxLog::EnableLogging(false);
    m_cfg = cfg = new wxFileConfig(wxEmptyString, wxEmptyString, wxEmptyString, file, wxCONFIG_USE_GLOBAL_FILE | wxCONFIG_USE_NO_ESCAPE_CHARACTERS);
    wxLog::EnableLogging(true);

    if (!cfg->GetNumberOfGroups())
	{
		error.Printf("Unable to read '%s'", file.c_str());
		return error;
	}

	m_b_customized = false;
    long cookie;
    bool bCont = cfg->GetFirstGroup(key, cookie);
    while (bCont)
    {
        if (key.StartsWith("_"))
        {
            m_b_customized = true;
            cfg->SetPath('/' + key);
            long cookie2;
            bool bCont2 = cfg->GetFirstEntry(key2, cookie2);
            while (bCont2)
            {
                cfg->Write('/' + key.Mid(1) + '/' + key2, cfg->Read(key2));
                bCont2 = cfg->GetNextEntry(key2, cookie2);
            }
            cfg->SetPath("/");
        }
        bCont = cfg->GetNextGroup(key, cookie);
    }

	if (!file2.IsEmpty())
	{
	    // NOTICE: unfortunately, the format is not adhered to well at all. with logging enabled, most sample sets generate warnings.
        wxLog::EnableLogging(false);
		extra_cfg = new wxFileConfig(wxEmptyString, wxEmptyString, wxEmptyString, file2, wxCONFIG_USE_GLOBAL_FILE | wxCONFIG_USE_NO_ESCAPE_CHARACTERS);
        wxLog::EnableLogging(true);

		key = "/Organ/ChurchName";
		if (cfg->Read(key).Trim() == extra_cfg->Read(key).Trim())
		{
			long cookie;
			bool bCont = extra_cfg->GetFirstGroup(key, cookie);
			while (bCont)
			{
				extra_cfg->SetPath('/' + key);
				long cookie2;
				bool bCont2 = extra_cfg->GetFirstEntry(key2, cookie2);
				while (bCont2)
				{
					cfg->Write(key + '/' + key2, extra_cfg->Read(key2));
					bCont2 = extra_cfg->GetNextEntry(key2, cookie2);
				}
				extra_cfg->SetPath("/");
				bCont = extra_cfg->GetNextGroup(key, cookie);
			}
		}
		else
		{
			if (!extra_cfg->GetNumberOfGroups())
			{
				error.Printf("Unable to read '%s'", file2.c_str());
				return error;
			}
			::wxLogWarning("This combination file is only compatible with:\n%s", extra_cfg->Read(key).c_str());
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
	m_pipe = new GOrguePipe*[m_NumberOfPipes + m_NumberOfTremulants];
	memset(m_pipe, 0, sizeof(GOrguePipe*) * (m_NumberOfPipes + m_NumberOfTremulants));

	m_path = file;
	m_path.MakeAbsolute();
	wxString temp;
	m_path.SetCwd(m_path.GetPath());
	int j, progress = 0;
	char *mbuffer_p = 0, *ptr, *compress;
	short *s_ptr;
	unsigned mbuffer_s = 0, compress_s = 0;
	m_compress_p = 0;
	int ffile = -1;

	/* The wave filenames stored in m_pipe_files are relative paths. This code
	 * takes each filename, converts it to it's absolute path and then stores
	 * it into the pipe_keys vector (FIXME: bad variable name). Reference pipe
	 * filenames are converted to an empty string so for each m_pipe_files[x],
	 * pipe_keys[x] is it's absolute path. */
	std::vector<wxString> pipe_keys;
	for (std::vector<wxString>::const_iterator aFileIter = m_pipe_files.begin();
		 aFileIter != m_pipe_files.end();
		 ++aFileIter)
	{

		if (aFileIter->StartsWith("REF:"))
		{
			pipe_keys.push_back(wxEmptyString);
			continue;
		}

		m_path = *aFileIter;
		m_path.MakeAbsolute();

		// FIXME: breaks an eventual translation to unicode
		key.Printf("%s", m_path.GetFullPath().c_str());
		pipe_keys.push_back(key);

	}

	/* Load pipes */
	for (int i = 0; i < m_pipe_files.size(); i++)
	{

		wxLogDebug("Loading file %s", m_pipe_files[i].c_str());

		/* If this pipe filename is a reference to another pipe, skip. We
		 * load these pipes later... */
		if (m_pipe_files[i].StartsWith("REF:"))
			continue;

		/* Update the progress dialog */
		if (!dlg.Update(((progress + 1) << 15) / (int)(m_NumberOfPipes + 1), m_pipe_files[i]))
		{
			error = "!"; // FIXME: what is this? how can a progress dialog fail to update?
			break;
		}

		// 327: max parameter to progress dialog divided by 100 to calculate percentage
#ifdef __VFD__
		int n=(((progress + 1) << 15) / (int)(organfile->NumberOfPipes + organfile->NumberOfTremulants ))/327;
		GOrgueLCD_WriteLineTwo(wxString::Format("Loading %d%%", n));
#endif

		/* FIXME: bad variable name. "pipe_filename" suggestion */
		key = pipe_keys[i];

		{
			try
			{
				unsigned wavestart = 0;
				unsigned wavesize = 0;
				unsigned loopstart = 0;
				unsigned loopend = 0;
				unsigned release = 0;
				unsigned q;
				unsigned r;
				unsigned length = 0;
				int channels = 0;
				int peak = 0;
				int amp = 10000;
				int k;
				int jj;

				temp = key.c_str();

				/* Open file, because of efficiency wxFile is not used
				 * FIXME: ^^^ the above comment was here when I started working
				 * on the code... I'm not sure what it means? Has the efficency
				 * of wxFile been tested? I think we should put it back in and
				 * make use of the cross-platform benefits. */
#ifdef linux
				temp.Replace("\\", "/");
				ffile = open(temp, O_RDONLY);
				struct stat ffile_info;
				fstat(ffile, &ffile_info);
				length = ffile_info.st_size;
#endif
#ifdef _WIN32
				ffile = _open(temp, _O_BINARY | _O_RDONLY | _O_SEQUENTIAL);
				length = _filelength(ffile);
#endif

				/* FIXME: better errorhandling (linux) < this causes GrandOrgue
				 * to quit ungracefully! very bad. */
				if (ffile == -1)
				{
					char message[1024];
					const char* x = temp.mb_str();
					sprintf(message, "Failed to open file '%s'\n", x);
					throw message;
				}

				/* FIXME: this is pointless. This is the load code. Delays of
				 * microseconds are unimportant. The temporary buffer should be
				 * malloced for each file and freed on cleanup. */
				if (!mbuffer_p)
					ptr = (char*)malloc(mbuffer_s = length + 8);
				else if (length + 8 > mbuffer_s)
					ptr = (char*)realloc(mbuffer_p, mbuffer_s = length + 8);
				else
					ptr = (char*)mbuffer_p;

				if (!ptr)
					throw (char*)_(" Out of memory loading");

				mbuffer_p = ptr;
				length=readOneFile(ffile, mbuffer_p, length);

				{

					RIFF riff;

					/* Check that this is a valid RIFF file */
					if (length >= 12)
					{
						riff.Analyze(ptr, 0);
						if (!(riff == "RIFF"&&
								(riff.GetSize() == length - 8 || riff.GetSize() == length)
								&& !strncmp(ptr + 8, "WAVE", 4)))
							throw (char*)"<Invalid WAV file";

					}
					else
						throw (char*)"<Invalid WAV file";

					/* Find required chunks... */
					for (j = 12; j + 8 <= (int)length;)
					{

						riff.Analyze(ptr, j);
						j += 8;

						/* Is this the data chunk? */
						if (riff == "data")
						{
								wavestart = j;
								wavesize = riff.GetSize() / 2;
						}

						jj = j; // TODO: Figure what this is for...

						/* Move J forward to the offset of the next chunk */
						j += (riff.GetSize() + 1) & 0xFFFFFFFE;
						if (j > (int)length)
							throw (char*)"<Invalid WAV file";

						/* Is this the format chunk?? */
						if (riff == "fmt ")
						{

							/* FIXME: This could be done much more elequently */
							/* Ensure format chunk size is 16 (basic wave
							 * format chunk... no extensible data... and
							 * that the format tag is 1 */
							if (riff.GetSize() < 16 || wxINT16_SWAP_ON_BE(*(short*)(ptr + j - riff.GetSize())) != 1)
								throw (char*)"<Not PCM data in";

							/* get channels and ensure only mono or stereo */
							channels = wxUINT16_SWAP_ON_BE(*(unsigned short*)(ptr + j - riff.GetSize() + 2));
							if (channels < 1 || channels > 2)
								throw (char*)"<More than 2 channels in";

							/* get sample rate and ensure only 44.1 kHz */
							if (wxUINT32_SWAP_ON_BE(*(unsigned*)(ptr + j - riff.GetSize() + 4)) != 44100)
								throw (char*)"<Not 44.1kHz sampling rate in";

						}

						/* Is this the cue chunk? */
						else if (riff == "cue " && !m_pipe_percussive[i])
						{

							if (riff.GetSize() < 4)
								throw (char*)"<Invalid CUE chunk in";

							/* k is the number of cue points. Ensure that the
							 * chunk has enough room to store 'k' cue points */
							k = wxUINT32_SWAP_ON_BE(*(unsigned*)(ptr + j - riff.GetSize()));
							if (riff.GetSize() < 4 + 24 * (unsigned)k)
								throw (char*)"<Invalid CUE chunk in";

							jj += 4; /* jj now points to the position of the first cue point */

							/* read each cue points position value. If the
							 * position is greater than the currently stored
							 * release position, update it. */
							for (; k>0; k--)
							{
								q = wxUINT32_SWAP_ON_BE(*(unsigned*)(ptr + jj + 20)) * channels;
								if (q > release)
									release = q;
								jj += 24;
							}

						}

						/* Is this the sampler chunk (contains loop/pitch information */
						else if (riff == "smpl" && !m_pipe_percussive[i])
						{

							/* TODO: we could support multiple sample loops? */
							if (riff.GetSize() < 36)
								throw (char*)"<Invalid SMPL chunk in";

							/* Read the number of loops and ensure that the
							 * chunk is large enough to store them all. */
							k = wxUINT32_SWAP_ON_BE(*(unsigned*)(ptr + j - riff.GetSize() + 28));
							if (riff.GetSize() < 36 + 24 * (unsigned)k)
								throw (char*)"<Invalid SMPL chunk in";

							jj += 36; /* jj now points to the offset of the first sampler loop */

							/* NOTE: for now, we just get the *longest* loop. */
							for (; k>0; k--)
							{

								q = wxUINT32_SWAP_ON_BE(*(unsigned*)(ptr + jj +  8)) * channels;
								r = wxUINT32_SWAP_ON_BE(*(unsigned*)(ptr + jj + 12)) * channels + channels;
								if (r - q > loopend - loopstart)
								{
									loopend   = r;
									loopstart = q;
								}
								jj += 24;

							}

						}

					} /* for */

					/* We have looked at all the chunks, j should now equal
					 * the wave file main chunk's length (as we have read
					 * all chunks. */
					if (!length || j != (int)length)
						throw (char*)"<Invalid WAV file";

					/* Find the wave's peak samples */
					int peaktemp;
					s_ptr = (short*)(ptr + wavestart) + wavesize;
					if (channels == 1)
					{
						for (k = -(int)wavesize; k; k++)
						{
							/* FIXME: if sample < 0 sample++ ?? what
							 * this cannot be good... what is going on here?
							 *
							 * peaktemp = s_ptr[k] xor (s_ptr[k] >> 15)
							 * ... which basically means if s_ptr[k] < 0,
							 * flip the LSB of s_ptr[k]. */
							/*
							if (s_ptr[k] & 0x8000)
								s_ptr[k]++;
							peaktemp  = s_ptr[k];
							peaktemp ^= (peaktemp >> 15);
							*/
							peaktemp = abs(peak);
							peak = std::max(peaktemp,peak);
						}
					}
					else
					{
						for (k = -(int)wavesize; k; k+=2)
						{
							/*
							if (s_ptr[k] & 0x8000)
								s_ptr[k]++;
							peaktemp  = s_ptr[k++];
							if (s_ptr[k] & 0x8000)
								s_ptr[k]++;
							peaktemp += s_ptr[k++];
							peaktemp ^= (peaktemp >> 15);
							*/
							peaktemp = abs(s_ptr[k] + s_ptr[k+1]);
							peak = std::max(peaktemp,peak);
						}
						peak = peak / 2;
					}

					// learning lesson: never ever trust the range values of outside sources to be correct!
					if (loopstart >= loopend || loopstart >= wavesize ||
						loopend >= wavesize || release >= wavesize ||
						!loopend || !release)
						loopstart = loopend = release = 0;

				}

				amp = m_pipe_amplitudes[progress];

				k = offsetof(GOrguePipe, data) + sizeof(short) * (12 + loopstart + (loopend - loopstart) + (wavesize - release));

				/* FIXME: for the same reason as previously, this should be
				 * allocated on a per pipe basis... */
				if (!m_compress_p)
					compress = (char*)malloc(compress_s = k);
				else if (k > (int)compress_s)
					compress = (char*)realloc(m_compress_p, compress_s = k);
				else
					compress = (char*)m_compress_p;
				if (!compress)
					throw (char*)_("<Out of memory loading");
				m_compress_p = (GOrguePipe*)compress;

				compress = (char*)m_compress_p->data;
				*(int*)(ptr + wavestart - 4) = 0;
				*(int*)(ptr + wavestart + sizeof(short) * wavesize) = 0;

				m_compress_p->ra_volume = peak;
				m_compress_p->ra_factor = (PHASE_ALIGN_RES << 25) / (peak + 1);

				if (release)
				{
					if (release + channels < wavesize)
						release += channels;
					if (loopstart & channels)
					{
						wavestart -= channels << 1;
						loopstart += channels;
						loopend   += channels;
						release   += channels;
						wavesize  += channels;
					}
					if ((wavesize - release) & channels)
						wavesize   += channels;

					CompressWAV(compress, m_compress_p->f[0], (short*)(ptr + wavestart), loopstart, channels, 0);
					CompressWAV(compress, m_compress_p->f[1], (short*)(ptr + wavestart) + loopstart, loopend - loopstart, channels, 1);
					CompressWAV(compress, m_compress_p->f[2], (short*)(ptr + wavestart) + release, wavesize - release, channels, 2);
					m_compress_p->instances = 0;
				}
				else
				{
					if (wavesize & channels)
						wavesize += channels;
					CompressWAV(compress, m_compress_p->f[0], (short*)(ptr + wavestart), wavesize, channels, 0);
					m_compress_p->instances = -1;
				}

				k = compress - (char*)m_compress_p;
				m_pipe[progress] = (GOrguePipe*)malloc(k);
				if (!m_pipe[progress])
					throw (char*)_("<Out of memory loading");
				memcpy(m_pipe[progress], m_compress_p, k);

				m_pipe[progress]->ra_shift = 7;
				while (amp > 10000)
				{
					m_pipe[progress]->ra_shift--;
					amp >>= 1;
				}
				m_pipe[progress]->ra_amp = (amp << 15) / -10000;
				m_pipe[progress]->sampler = 0;
				for (k = 0; k < 3; k++)
					m_pipe[progress]->ptr[k] += (unsigned)m_pipe[progress];
			}
			catch (char* str)
			{
			  wxLogError("%s",str);
			  error.Printf("%s '%s'", str + 1, key.c_str());
			  break;
			}
		}

		*m_pipe_ptrs[i] = (short)progress;
		m_pipe[progress++]->WindchestGroup = m_pipe_windchests[i];

	}

	/* Load tremulants */
	for (int i = 0; i < m_NumberOfTremulants; i++)
	{

		wxLogDebug("Loading tremulant #%d", i);

		{
			try
			{
				unsigned wavestart = 0;
				unsigned wavesize = 0;
				unsigned loopstart = 0;
				unsigned loopend = 0;
				unsigned release = 0, q, r, length = 0;
				int channels = 0, peak = 0, amp = 10000, k, jj;

				channels = 1;
				wavestart = 4;
				loopstart = 441000 / m_tremulant[i].StartRate;
				release = loopend = loopstart + 441 * m_tremulant[i].Period / 10;
				wavesize = release + 441000 / m_tremulant[i].StopRate;
				length = 8 + sizeof(short) * wavesize;

				if (!mbuffer_p)
					ptr = (char*)malloc(mbuffer_s = length + 8);
				else if (length + 8 > mbuffer_s)
					ptr = (char*)realloc(mbuffer_p, mbuffer_s = length + 8);
				else
					ptr = (char*)mbuffer_p;

				if (!ptr)
				{
					throw (char*)_(" Out of memory loading");
				}
				mbuffer_p = ptr;

				ptr = mbuffer_p;

				fillBufferWithTremulant(m_tremulant[i],(short*)ptr);
				peak =  (16384 * m_tremulant[i].AmpModDepth / 100);

				k = offsetof(GOrguePipe, data) + sizeof(short) * (12 + loopstart + (loopend - loopstart) + (wavesize - release));

				if (!m_compress_p)
					compress = (char*)malloc(compress_s = k);
				else if (k > (int)compress_s)
					compress = (char*)realloc(m_compress_p, compress_s = k);
				else
					compress = (char*)m_compress_p;

				if (!compress)
					throw (char*)_("<Out of memory loading");
				m_compress_p = (GOrguePipe*)compress;

				compress = (char*)m_compress_p->data;
				*(int*)(ptr + wavestart - 4) = 0;
				*(int*)(ptr + wavestart + sizeof(short) * wavesize) = 0;

				m_compress_p->ra_volume = peak;
				m_compress_p->ra_factor = (PHASE_ALIGN_RES << 25) / (peak + 1);

				if (release)
				{
					if (release + channels < wavesize)
						release += channels;
					if (loopstart & channels)
					{
						wavestart -= channels << 1;
						loopstart += channels;
						loopend   += channels;
						release   += channels;
						wavesize  += channels;
					}
					if ((wavesize - release) & channels)
						wavesize   += channels;

					CompressWAV(compress, m_compress_p->f[0], (short*)(ptr + wavestart), loopstart, channels, 0);
					CompressWAV(compress, m_compress_p->f[1], (short*)(ptr + wavestart) + loopstart, loopend - loopstart, channels, 1);
					CompressWAV(compress, m_compress_p->f[2], (short*)(ptr + wavestart) + release, wavesize - release, channels, 2);
					m_compress_p->instances = 0;
				}
				else
				{
					if (wavesize & channels)
						wavesize += channels;
					CompressWAV(compress, m_compress_p->f[0], (short*)(ptr + wavestart), wavesize, channels, 0);
					m_compress_p->instances = -1;
				}

				k = compress - (char*)m_compress_p;
				m_pipe[progress] = (GOrguePipe*)malloc(k);
				if (!m_pipe[progress])
					throw (char*)_("<Out of memory loading");
				memcpy(m_pipe[progress], m_compress_p, k);

				m_pipe[progress]->ra_shift = 7;
				while (amp > 10000)
				{
					m_pipe[progress]->ra_shift--;
					amp >>= 1;
				}
				m_pipe[progress]->ra_amp = (amp << 15) / -10000;
				m_pipe[progress]->sampler = 0;
				for (k = 0; k < 3; k++)
					m_pipe[progress]->ptr[k] += (unsigned)m_pipe[progress];
			}
			catch (char* str)
			{
			  wxLogError("%s",str);
			  error.Printf("%s '%s'", str + 1, "Tremulant");
			  break;
			}
		}

		m_tremulant[i].pipe = m_pipe[progress];
		m_pipe[progress++]->WindchestGroup = i;

	}

	/*
	int ii=m_pipe_files.size();
	int max=ii+m_NumberOfTremulants;
	for (int i = 0; i < max; i++)
	{
		wxLogDebug("Loading file %s",m_pipe_files[i].c_str());
		if (i < ii)
		{
			if (m_pipe_files[i].StartsWith("REF:"))
				continue;
			if (!dlg.Update(((progress + 1) << 15) / (int)(m_NumberOfPipes + m_NumberOfTremulants + 1), m_pipe_files[i]))
			{
				error = "!";
				break;
			}
			// 327: max parameter to progress dialog divided by 100 to calculate percentage
#ifdef __VFD__
			int n=(((progress + 1) << 15) / (int)(organfile->NumberOfPipes + organfile->NumberOfTremulants ))/327;
			GOrgueLCD_WriteLineTwo(wxString::Format("Loading %d%%", n));
#endif
			key = pipe_keys[i];

		}
		else if (!dlg.Update(((progress + 1) << 15) / (int)(m_NumberOfPipes + m_NumberOfTremulants + 1), wxString("Tremulant ") << m_tremulant[i - ii].ObjectNumber))
		{
			error = "!";
			break;
		}

		{
			try
			{
				unsigned wavestart = 0;
				unsigned wavesize = 0;
				unsigned loopstart = 0;
				unsigned loopend = 0;
				unsigned release = 0, q, r, length = 0;
				int channels = 0, peak = 0, amp = 10000, k, jj;

				if (i < ii)
				{
					temp = key.c_str();
					// Open file, because of efficiency wxFile is not used
#ifdef linux
					temp.Replace("\\", "/");
					ffile = open(temp, O_RDONLY);
					struct stat ffile_info;
					fstat(ffile, &ffile_info);
					length = ffile_info.st_size;
#endif
#ifdef _WIN32
					ffile = _open(temp, _O_BINARY | _O_RDONLY | _O_SEQUENTIAL);
					length = _filelength(ffile);
#endif
					if (ffile == -1) // TODO: better errorhandling (linux)
					{
						char message[1024];
						const char* x = temp.mb_str();
						sprintf(message, "Failed to open file '%s'\n", x);
						throw message;
					}

				}
				else if (i >= ii)
				{
					channels = 1;
					wavestart = 4;
					loopstart = 441000 / m_tremulant[i - ii].StartRate;
					release = loopend = loopstart + 441 * m_tremulant[i - ii].Period / 10;
					wavesize = release + 441000 / m_tremulant[i - ii].StopRate;
					length = 8 + sizeof(short) * wavesize;
				}

				if (!mbuffer_p)
					ptr = (char*)malloc(mbuffer_s = length + 8);
				else if (length + 8 > mbuffer_s)
					ptr = (char*)realloc(mbuffer_p, mbuffer_s = length + 8);
				else
					ptr = (char*)mbuffer_p;

				if (!ptr)
				{
					throw (char*)_(" Out of memory loading");
				}
				mbuffer_p = ptr;

				if (i < ii)
				{
					length=readOneFile(ffile, mbuffer_p, length);
				}

				ptr = mbuffer_p;

				if (i >= ii)
				{

					fillBufferWithTremulant(m_tremulant[i-ii],(short*)ptr);
					peak =  (16384 * m_tremulant[i-ii].AmpModDepth / 100);
				}
				else
				{
					{
						RIFF riff;
						if (length >= 12)
						{
							riff.Analyze(ptr, 0);
							if (!(riff == "RIFF"&&
									(riff.GetSize() == length - 8 || riff.GetSize() == length)
									&& !strncmp(ptr + 8, "WAVE", 4)))
							{
								throw (char*)"<Invalid WAV file";
							}
						}
						else
						{
							throw (char*)"<Invalid WAV file";
						}

						for (j = 12; j + 8 <= (int)length; )
						{
							riff.Analyze(ptr, j);
							j += 8;
							if (riff == "data")
							{
								wavestart = j;
								wavesize = riff.GetSize() / 2;
							}
							jj = j;
							j += (riff.GetSize() + 1) & 0xFFFFFFFE;
							if (j > (int)length)
							{
								throw (char*)"<Invalid WAV file";
							}
							if (riff == "fmt ")
							{
								if (riff.GetSize() < 16 || wxINT16_SWAP_ON_BE(*(short*)(ptr + j - riff.GetSize())) != 1)
								{
									throw (char*)"<Not PCM data in";
								}
								channels = wxUINT16_SWAP_ON_BE(*(unsigned short*)(ptr + j - riff.GetSize() + 2));
								if (channels < 1 || channels > 2)
									throw (char*)"<More than 2 channels in";
								if (wxUINT32_SWAP_ON_BE(*(unsigned*)(ptr + j - riff.GetSize() + 4)) != 44100)
									throw (char*)"<Not 44.1kHz sampling rate in";
							}
							else if (riff == "cue " && !m_pipe_percussive[i])
							{
								if (riff.GetSize() < 4)
									throw (char*)"<Invalid CUE chunk in";
								k = wxUINT32_SWAP_ON_BE(*(unsigned*)(ptr + j - riff.GetSize()));
								if (riff.GetSize() < 4 + 24 * (unsigned)k)
									throw (char*)"<Invalid CUE chunk in";
								jj += 4;
								while (k)
								{
									q = wxUINT32_SWAP_ON_BE(*(unsigned*)(ptr + jj + 20)) * channels;
									if (q > release)
										release = q;
									k--;
									jj += 24;
								}
							}
							else if (riff == "smpl" && !m_pipe_percussive[i])
							{
								// TODO: we could support multiple sample loops?
								if (riff.GetSize() < 36)
									throw (char*)"<Invalid SMPL chunk in";
								k = wxUINT32_SWAP_ON_BE(*(unsigned*)(ptr + j - riff.GetSize() + 28));
								if (riff.GetSize() < 36 + 24 * (unsigned)k)
									throw (char*)"<Invalid SMPL chunk in";

								jj += 36;
								// NOTE: for now, we just get the *longest* loop.
								while (k)
								{
									q = wxUINT32_SWAP_ON_BE(*(unsigned*)(ptr + jj +  8)) * channels;
									r = wxUINT32_SWAP_ON_BE(*(unsigned*)(ptr + jj + 12)) * channels + channels;
									if (r - q > loopend - loopstart)
									{
										loopend   = r;
										loopstart = q;
									}
									k--;
									jj += 24;
								}
							}
						}
						if (!length || j != (int)length)
							throw (char*)"<Invalid WAV file";

						int peaktemp;
						s_ptr = (short*)(ptr + wavestart) + wavesize;
						if (channels == 1)
						{
							for (k = -(int)wavesize; k; )
							{
								if (s_ptr[k] & 0x8000)
									s_ptr[k]++;
								peaktemp  = s_ptr[k++];
								peaktemp ^= (peaktemp >> 15);
								peak = std::max(peaktemp,peak);
							}
						}
						else
						{
							for (k = -(int)wavesize; k; )
							{
								if (s_ptr[k] & 0x8000)
									s_ptr[k]++;
								peaktemp  = s_ptr[k++];
								if (s_ptr[k] & 0x8000)
									s_ptr[k]++;
								peaktemp += s_ptr[k++];
								peaktemp ^= (peaktemp >> 15);
								peak = std::max(peaktemp,peak);
							}
							peak >>= 1;
						}

						// learning lesson: never ever trust the range values of outside sources to be correct!
						if (loopstart >= loopend || loopstart >= wavesize || loopend >= wavesize || release >= wavesize || !loopend || !release)
							loopstart = loopend = release = 0;
					}
					amp = m_pipe_amplitudes[progress];
				}

				k = offsetof(GOrguePipe, data) + sizeof(short) * (12 + loopstart + (loopend - loopstart) + (wavesize - release));
				if (!m_compress_p)
					compress = (char*)malloc(compress_s = k);
				else if (k > (int)compress_s)
					compress = (char*)realloc(m_compress_p, compress_s = k);
				else
					compress = (char*)m_compress_p;
				if (!compress)
					throw (char*)_("<Out of memory loading");
				m_compress_p = (GOrguePipe*)compress;

				compress = (char*)m_compress_p->data;
				*(int*)(ptr + wavestart - 4) = 0;
				*(int*)(ptr + wavestart + sizeof(short) * wavesize) = 0;

				m_compress_p->ra_volume = peak;
				m_compress_p->ra_factor = (PHASE_ALIGN_RES << 25) / (peak + 1);

				if (release)
				{
					if (release + channels < wavesize)
						release += channels;
					if (loopstart & channels)
					{
						wavestart -= channels << 1;
						loopstart += channels;
						loopend   += channels;
						release   += channels;
						wavesize  += channels;
					}
					if ((wavesize - release) & channels)
						wavesize   += channels;

					CompressWAV(compress, m_compress_p->f[0], (short*)(ptr + wavestart), loopstart, channels, 0);
					CompressWAV(compress, m_compress_p->f[1], (short*)(ptr + wavestart) + loopstart, loopend - loopstart, channels, 1);
					CompressWAV(compress, m_compress_p->f[2], (short*)(ptr + wavestart) + release, wavesize - release, channels, 2);
					m_compress_p->instances = 0;
				}
				else
				{
					if (wavesize & channels)
						wavesize += channels;
					CompressWAV(compress, m_compress_p->f[0], (short*)(ptr + wavestart), wavesize, channels, 0);
					m_compress_p->instances = -1;
				}

				k = compress - (char*)m_compress_p;
				m_pipe[progress] = (GOrguePipe*)malloc(k);
				if (!m_pipe[progress])
					throw (char*)_("<Out of memory loading");
				memcpy(m_pipe[progress], m_compress_p, k);

				m_pipe[progress]->ra_shift = 7;
				while (amp > 10000)
				{
					m_pipe[progress]->ra_shift--;
					amp >>= 1;
				}
				m_pipe[progress]->ra_amp = (amp << 15) / -10000;
				m_pipe[progress]->sampler = 0;
				for (k = 0; k < 3; k++)
					m_pipe[progress]->ptr[k] += (unsigned)m_pipe[progress];
			}
			catch (char* str)
			{
			  wxLogError("%s",str);
			  error.Printf("%s '%s'", str + 1, i < ii ?  key.c_str()  : "Tremulant");
			  break;
			}
		}

		if (i >= ii)
		{
			m_tremulant[i - ii].pipe = m_pipe[progress];
			m_pipe[progress++]->WindchestGroup = i - ii;
		}
		else
		{
			*m_pipe_ptrs[i] = (short)progress;
			m_pipe[progress++]->WindchestGroup = m_pipe_windchests[i];
		}
	}*/


	if (mbuffer_p)
		free(mbuffer_p);
	if (m_compress_p)
		free(m_compress_p);

	/* Resolve references in m_pipe_files */
	/* This code goes through the list of all pipe filenames
	 * searching for filenames starting with the "REF:" identifier,
	 * if a pipe is found, it will try to resolve the pipe.	*/
	for (int i = 0; i < m_pipe_files.size(); i++)
	{

		if (!m_pipe_files[i].StartsWith("REF:"))
			continue;

		int manual, stop, pipe;
		sscanf(m_pipe_files[i].c_str() + 4, "%d:%d:%d", &manual, &stop, &pipe);
		if ((manual < m_FirstManual) || (manual > m_NumberOfManuals) ||
			(stop <= 0) || (stop > m_manual[manual].GetStopCount()) ||
			(pipe <= 0) || (pipe > m_manual[manual].GetStop(stop-1)->NumberOfLogicalPipes))
			return "Invalid reference " + m_pipe_files[i];

		*m_pipe_ptrs[i] = m_manual[manual].GetStop(stop-1)->pipe[pipe-1];

	}

	/* TODO: ? check for correctness ? */
	/* Load the images for the stops */
	for (int i = 0; i < 9; i++)
	{

		wxMemoryInputStream mem((const char*)ImageLoader_Stops[i], c_ImageLoader_Stops[i]);
		wxImage img(mem, wxBITMAP_TYPE_PNG);
		m_images[i] = wxBitmap(img);

	}

	m_pipe_files.clear();
	m_pipe_ptrs.clear();
	m_pipe_amplitudes.clear();
	m_pipe_windchests.clear();
	m_pipe_percussive.clear();

	if (m_cfg)
	{
		::wxGetApp().frame->m_meters[0]->SetValue(m_cfg->Read("/Organ/Volume", g_sound->GetVolume()));
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
	for (int i = 0; i < m_NumberOfTremulants; i++)
        if (m_tremulant && m_tremulant[i].pipe)
            free(m_tremulant[i].pipe);
	if (m_pipe)
		delete[] m_pipe;
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
        if (key.StartsWith("_"))
            to_drop.Add(key);
        bCont = cfg.GetNextGroup(key, cookie);
    }
    for (unsigned i = 0; i < to_drop.Count(); i++)
        cfg.DeleteGroup(to_drop[i]);
}

void GrandOrgueFile::Save(const wxString& file)
{
    wxFileName fn(file);
    bool prefix = true;

    if (fn.GetExt().CmpNoCase("organ"))
    {
        if (::wxFileExists(file) && !::wxRemoveFile(file))
        {
            ::wxLogError("Could not write to '%s'", file.c_str());
            return;
        }
        prefix = false;
    }

    wxLog::EnableLogging(false);
    wxFileConfig cfg(wxEmptyString, wxEmptyString, file, wxEmptyString, wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_NO_ESCAPE_CHARACTERS);
    wxLog::EnableLogging(true);
    if (prefix)
        Revert(cfg);
	m_b_customized = true;

	IniFileConfig aIni(&cfg);
    aIni.SaveHelper(prefix, "Organ", "ChurchName", m_ChurchName);
    aIni.SaveHelper(prefix, "Organ", "ChurchAddress", m_ChurchAddress);
    aIni.SaveHelper(prefix, "Organ", "HauptwerkOrganFileFormatVersion", m_HauptwerkOrganFileFormatVersion);
    aIni.SaveHelper(prefix, "Organ", "NumberOfFrameGenerals", m_NumberOfFrameGenerals);
    aIni.SaveHelper(prefix, "Organ", "Volume", g_sound->GetVolume());

    int i, j;

	for (i = m_FirstManual; i <= m_NumberOfManuals; i++)
	{
		for (j = 0; j < m_manual[i].GetStopCount(); j++)
			m_manual[i].GetStop(j)->Save(aIni, prefix);
		for (j = 0; j < m_manual[i].GetCouplerCount(); j++)
			m_manual[i].GetCoupler(j)->Save(aIni, prefix);
		for (j = 0; j < m_manual[i].GetDivisionalCount(); j++)
			m_manual[i].GetDivisional(j)->Save(aIni, prefix);
	}
	for (j = 0; j < m_NumberOfTremulants; j++)
		m_tremulant[j].Save(aIni, prefix);
	for (j = 0; j < m_NumberOfDivisionalCouplers; j++)
		m_divisionalcoupler[j].Save(aIni, prefix);
	for (j = 0; j < m_NumberOfGenerals; j++)
		m_general[j].Save(aIni, prefix);
	for (j = 0; j < m_NumberOfFrameGenerals; j++)
		m_framegeneral[j].Save(aIni, prefix);
	for (j = 0; j < m_NumberOfReversiblePistons; j++)
		m_piston[j].Save(aIni, prefix);

}


int GrandOrgueFile::readOneFile(int file, char* buffer, unsigned length)
{
  unsigned remain = length;
  for(;;)
	{
#ifdef _WIN32
	  remain -= _read(file, buffer, remain >= BUFSIZ ? BUFSIZ : remain);
#endif
#ifdef linux
	  remain -= read(file, buffer, remain >= BUFSIZ ? BUFSIZ : remain); // TODO better error handling
#endif
	  if (!remain)
		break;
	  buffer += BUFSIZ;
	}
#ifdef _WIN32
  _close(file);
#endif
#ifdef linux
  close(file);
#endif
  return length;
}


void GrandOrgueFile::fillBufferWithTremulant(const GOrgueTremulant& tremulant,short* buffer)
{
  unsigned wavestart = 4;
  unsigned loopstart = 441000 / tremulant.StartRate;
  unsigned release = loopstart + 441 * tremulant.Period / 10;
  unsigned loopend = release;
  unsigned wavesize = release + 441000 / tremulant.StopRate;
  short *s_ptr;
  double trem_amp   =  (16384 * tremulant.AmpModDepth / 100);
  double trem_param = 0.14247585730565955729989312395825 / (double)tremulant.Period;
  double trem_fade, trem_inc, trem_angle;
  s_ptr = (short*)(buffer + wavestart);

  trem_inc = 1.0 / (double)loopstart;
  trem_fade = trem_angle = 0.0;
  int j = -(int)loopstart;
  s_ptr -= j;
  do
	{
	  s_ptr[j] = SynthTrem(trem_amp, trem_angle, trem_fade);
	  trem_angle += trem_param * trem_fade;
	  trem_fade += trem_inc;
	}
  while (++j);

  j = (int)loopstart - (int)loopend;
  s_ptr -= j;
  do
	{
	  s_ptr[j] = SynthTrem(trem_amp, trem_angle);
	  trem_angle += trem_param;
	}
  while (++j);

  trem_inc = 1.0 / (double)(wavesize - release);
  trem_fade = 1.0 - trem_inc;
  j = (int)release - (int)wavesize;
  s_ptr -= j;
  do
	{
	  s_ptr[j] = SynthTrem(trem_amp, trem_angle, trem_fade);
	  trem_angle += trem_param * trem_fade;
	  trem_fade -= trem_inc;
	}
  while (++j);
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

int GrandOrgueFile::GetEnclosureCount()
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

