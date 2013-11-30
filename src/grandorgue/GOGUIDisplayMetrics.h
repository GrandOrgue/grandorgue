/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2013 GrandOrgue contributors (see AUTHORS)
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

#ifndef GOGUIDISPLAYMETRICS_H
#define GOGUIDISPLAYMETRICS_H

#include <wx/wx.h>
#include <vector>

class GOGUIEnclosure;
class GrandOrgueFile;

class GOGUIDisplayMetrics
{

public:

	typedef struct
	{
		unsigned width;
		unsigned height;
		int y;
		int x;
		int keys_y;
		int piston_y;
	} MANUAL_RENDER_INFO;

protected:
	wxString m_group;
	GrandOrgueFile* m_organfile;

	typedef struct
	{
		bool displayed;
		unsigned nb_accessible_keys;
		int first_accessible_key_midi_note_nb;
		MANUAL_RENDER_INFO render_info;
	} MANUAL_INFO;

	/* Values loaded in constructor */
	unsigned m_nb_enclosures;
	unsigned m_nb_manuals;
	unsigned m_first_manual;
	std::vector<MANUAL_INFO> m_manual_info;

	/* Values loaded from ODF */
	unsigned m_DispScreenSizeHoriz;
	unsigned m_DispScreenSizeVert;
	unsigned m_DispDrawstopBackgroundImageNum;
	unsigned m_DispConsoleBackgroundImageNum;
	unsigned m_DispKeyHorizBackgroundImageNum;
	unsigned m_DispKeyVertBackgroundImageNum;
	unsigned m_DispDrawstopInsetBackgroundImageNum;
	wxString m_DispControlLabelFont;
	wxString m_DispShortcutKeyLabelFont;
	wxColour m_DispShortcutKeyLabelColour;
	wxString m_DispGroupLabelFont;
	unsigned m_DispDrawstopCols;
	unsigned m_DispDrawstopRows;
	bool m_DispDrawstopColsOffset;
	bool m_DispDrawstopOuterColOffsetUp;
	bool m_DispPairDrawstopCols;
	unsigned m_DispExtraDrawstopRows;
	unsigned m_DispExtraDrawstopCols;
	unsigned m_DispButtonCols;
	unsigned m_DispExtraButtonRows;
	bool m_DispExtraPedalButtonRow;
	bool m_DispExtraPedalButtonRowOffset;
	bool m_DispExtraPedalButtonRowOffsetRight;
	bool m_DispButtonsAboveManuals;
	bool m_DispTrimAboveManuals;
	bool m_DispTrimBelowManuals;
	bool m_DispTrimAboveExtraRows;
	bool m_DispExtraDrawstopRowsAboveExtraButtonRows;
	unsigned m_DrawStopWidth;
	unsigned m_DrawStopHeight;
	unsigned m_ButtonWidth;
	unsigned m_ButtonHeight;
	unsigned m_EnclosureWidth;
	unsigned m_EnclosureHeight;
	unsigned m_PedalHeight;
	unsigned m_PedalKeyWidth;
	unsigned m_ManualHeight;
	unsigned m_ManualKeyWidth;

	/* values computed on screen update */
	int m_HackY;
	int m_EnclosureY;
	int m_CenterY;
	int m_CenterWidth;

	virtual void Update();

public:

	GOGUIDisplayMetrics(GrandOrgueFile* organfile, wxString group);
	virtual ~GOGUIDisplayMetrics();

	virtual unsigned NumberOfExtraDrawstopRowsToDisplay();
	virtual unsigned NumberOfExtraDrawstopColsToDisplay();
	virtual unsigned NumberOfDrawstopColsToDisplay();
	virtual unsigned NumberOfButtonCols();
	virtual unsigned NumberOfExtraButtonRows();

	virtual void GetDrawstopBlitPosition(const int drawstopRow, const int drawstopCol, int* blitX, int* blitY);
	virtual void GetPushbuttonBlitPosition(const int buttonRow, const int buttonCol, int* blitX, int* blitY);
	virtual wxFont GetControlLabelFont();
	virtual wxFont GetGroupLabelFont();

	virtual unsigned GetEnclosureWidth();
	virtual int GetEnclosureY();
	virtual int GetEnclosureX(const GOGUIEnclosure* enclosure);

	virtual unsigned GetScreenWidth();
	virtual unsigned GetScreenHeight();

	virtual int GetJambLeftRightWidth();
	virtual unsigned GetJambLeftRightHeight();
	virtual int GetJambLeftRightY();
	virtual int GetJambLeftX();
	virtual int GetJambRightX();
	virtual int GetJambTopDrawstop();
	virtual int GetJambTopPiston();
	virtual unsigned GetJambTopHeight();
	virtual unsigned GetJambTopWidth();
	virtual int GetJambTopX();
	virtual int GetJambTopY();
	virtual unsigned GetPistonTopHeight();
	virtual unsigned GetPistonWidth();
	virtual int GetPistonX();
	virtual int GetCenterWidth();
	virtual int GetCenterY();
	virtual int GetCenterX();

	virtual unsigned GetDrawstopBackgroundImageNum();
	virtual unsigned GetConsoleBackgroundImageNum();
	virtual unsigned GetDrawstopInsetBackgroundImageNum();
	virtual unsigned GetKeyVertBackgroundImageNum();
	virtual unsigned GetKeyHorizBackgroundImageNum();

	virtual bool HasPairDrawstopCols();
	virtual bool HasTrimAboveExtraRows();
	virtual bool HasExtraPedalButtonRow();

	virtual unsigned NewEnclosure();

	virtual const MANUAL_RENDER_INFO& GetManualRenderInfo(const unsigned manual_nb) const;

	virtual unsigned GetDrawstopWidth();
	virtual unsigned GetDrawstopHeight();
	virtual unsigned GetButtonWidth();
	virtual unsigned GetButtonHeight();
};

#endif
