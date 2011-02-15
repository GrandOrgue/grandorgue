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

#ifndef GRANDORGUEFILE_H
#define GRANDORGUEFILE_H

#include <vector>
#include <wx/wx.h>
#include <wx/fileconf.h>
#include <wx/filename.h>
#include "GOrguePipe.h"
#include "OrganFile.h"
#include "GOrgueManual.h"
#include "GOrguePushbutton.h"
#include "GOrgueEnclosure.h"
#include "GOrgueWindchest.h"
#include "GOrgueLabel.h"
#include "GOrgueFrameGeneral.h"

class GrandOrgueFile 
{
public:
  GrandOrgueFile();
	
	wxString Load(const wxString& file, const wxString& file2 = wxEmptyString);
	void Save(const wxString& file);
	void Revert(wxFileConfig& cfg);
	~GrandOrgueFile(void);
  short SynthTrem(double amp, double angle)
  {
	return (short)(amp * sin(angle));
  }

  short SynthTrem(double amp, double angle, double fade)
  {
	return (short)(fade * amp * sin(angle));
  }

	void CompressWAV(char*& compress, short* fv, short* ptr, int count, int channels, int stage);

	std::vector<wxString> m_pipe_filenames;
	std::vector<int> m_pipe_filesizes;

	std::vector<wxString> m_pipe_files;
	std::vector<wxInt16*> m_pipe_ptrs;
	std::vector<wxInt16> m_pipe_windchests;
	std::vector<wxInt16> m_pipe_percussive;
	std::vector<int> m_pipe_amplitudes;
	wxBitmap m_images[9];
	wxFileName m_path;
	int m_b_squash;
	GOrguePipe* m_compress_p;
	wxFileConfig *m_cfg;
	wxString m_filename;
	long m_elapsed;

 	bool m_opening : 1;
	bool m_b_customized : 1;

	bool m_DivisionalsStoreIntermanualCouplers : 1;
	bool m_DivisionalsStoreIntramanualCouplers : 1;
	bool m_DivisionalsStoreTremulants : 1;
	bool m_GeneralsStoreDivisionalCouplers : 1;
	bool m_CombinationsStoreNonDisplayedDrawstops : 1;
	bool m_DispDrawstopColsOffset : 1;
	bool m_DispDrawstopOuterColOffsetUp : 1;
	bool m_DispPairDrawstopCols : 1;
	bool m_DispExtraPedalButtonRow : 1;
	bool m_DispExtraPedalButtonRowOffset : 1;
	bool m_DispExtraPedalButtonRowOffsetRight : 1;
	bool m_DispButtonsAboveManuals : 1;
	bool m_DispTrimAboveManuals : 1;
	bool m_DispTrimBelowManuals : 1;
	bool m_DispTrimAboveExtraRows : 1;
	bool m_DispExtraDrawstopRowsAboveExtraButtonRows : 1;

	wxInt16 m_JambLeftRightWidth, m_JambLeftRightHeight, m_JambLeftRightY;
	wxInt16 m_JambLeftX, m_JambRightX;
	wxInt16 m_JambTopWidth, m_JambTopHeight;
	wxInt16 m_JambTopX, m_JambTopY, m_HackY;
	wxInt16 m_JambTopPiston, m_JambTopDrawstop;
	wxInt16 m_CenterX, m_CenterY, m_CenterWidth;
	wxInt16 m_PistonX, m_PistonWidth, m_PistonTopHeight;
	wxInt16 m_EnclosureWidth, m_EnclosureY;

	wxInt16 m_HighestSampleFormat;
	wxInt16 m_FirstManual;
	wxInt16 m_NumberOfManuals;
	wxInt16 m_NumberOfEnclosures;
	wxInt16 m_NumberOfTremulants;
	wxInt16 m_NumberOfWindchestGroups;
	wxInt16 m_NumberOfReversiblePistons;
	wxInt16 m_NumberOfLabels;
	wxInt16 m_NumberOfGenerals;
	wxInt16 m_NumberOfFrameGenerals;
	wxInt16 m_NumberOfDivisionalCouplers;
	wxInt16 m_NumberOfStops;
	wxInt16 m_NumberOfPipes;
	wxInt16 m_AmplitudeLevel;

	wxInt16 m_DispScreenSizeHoriz;
	wxInt16 m_DispScreenSizeVert;
	wxInt16 m_DispDrawstopBackgroundImageNum;
	wxInt16 m_DispConsoleBackgroundImageNum;
	wxInt16 m_DispKeyHorizBackgroundImageNum;
	wxInt16 m_DispKeyVertBackgroundImageNum;
	wxInt16 m_DispDrawstopInsetBackgroundImageNum;

	wxInt16 m_DispDrawstopCols;
	wxInt16 m_DispDrawstopRows;
	wxInt16 m_DispExtraDrawstopRows;
	wxInt16 m_DispExtraDrawstopCols;
	wxInt16 m_DispButtonCols;
	wxInt16 m_DispExtraButtonRows;

	wxColour m_DispShortcutKeyLabelColour;

    wxString m_HauptwerkOrganFileFormatVersion;
	wxString m_ChurchName;
	wxString m_ChurchAddress;
	wxString m_OrganBuilder;
	wxString m_OrganBuildDate;
	wxString m_OrganComments;
	wxString m_RecordingDetails;
    wxString m_InfoFilename;

	wxString m_DispControlLabelFont;
	wxString m_DispShortcutKeyLabelFont;
	wxString m_DispGroupLabelFont;

	GOrgueManual m_manual[7];
	GOrgueEnclosure* m_enclosure;
	GOrgueTremulant* m_tremulant;
	GOrgueWindchest* m_windchest;
	GOrguePiston* m_piston;
	GOrgueLabel* m_label;
	GOrgueGeneral* m_general;
	GOrgueFrameGeneral* m_framegeneral;
	GOrgueDivisionalCoupler* m_divisionalcoupler;
	GOrguePipe** m_pipe;

private:
  void readOrganFile();
  int readOneFile(int file, char* buffer, unsigned length);
  void fillBufferWithTremulant(const GOrgueTremulant& tremulant,short* buffer);
};

#endif
