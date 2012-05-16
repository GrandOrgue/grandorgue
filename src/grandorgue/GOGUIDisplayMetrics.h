/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2012 GrandOrgue contributors (see AUTHORS)
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
 * MA 02111-1307, USA.
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
		int width;
		int height;
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
	int m_DispScreenSizeHoriz;
	int m_DispScreenSizeVert;
	int m_DispDrawstopBackgroundImageNum;
	int m_DispConsoleBackgroundImageNum;
	int m_DispKeyHorizBackgroundImageNum;
	int m_DispKeyVertBackgroundImageNum;
	int m_DispDrawstopInsetBackgroundImageNum;
	wxString m_DispControlLabelFont;
	wxString m_DispShortcutKeyLabelFont;
	wxColour m_DispShortcutKeyLabelColour;
	wxString m_DispGroupLabelFont;
	int m_DispDrawstopCols;
	int m_DispDrawstopRows;
	bool m_DispDrawstopColsOffset;
	bool m_DispDrawstopOuterColOffsetUp;
	bool m_DispPairDrawstopCols;
	int m_DispExtraDrawstopRows;
	int m_DispExtraDrawstopCols;
	int m_DispButtonCols;
	int m_DispExtraButtonRows;
	bool m_DispExtraPedalButtonRow;
	bool m_DispExtraPedalButtonRowOffset;
	bool m_DispExtraPedalButtonRowOffsetRight;
	bool m_DispButtonsAboveManuals;
	bool m_DispTrimAboveManuals;
	bool m_DispTrimBelowManuals;
	bool m_DispTrimAboveExtraRows;
	bool m_DispExtraDrawstopRowsAboveExtraButtonRows;
	int m_DrawStopWidth;
	int m_DrawStopHeight;
	int m_ButtonWidth;
	int m_ButtonHeight;
	int m_EnclosureWidth;
	int m_EnclosureHeight;
	int m_PedalHeight;
	int m_PedalKeyWidth;
	int m_ManualHeight;
	int m_ManualKeyWidth;

	/* values computed on screen update */
	int m_HackY;
	int m_EnclosureY;
	int m_CenterY;
	int m_CenterWidth;

	virtual void Update();

public:

	GOGUIDisplayMetrics(GrandOrgueFile* organfile, wxString group);
	virtual ~GOGUIDisplayMetrics();

	virtual int NumberOfExtraDrawstopRowsToDisplay();
	virtual int NumberOfExtraDrawstopColsToDisplay();
	virtual int NumberOfDrawstopColsToDisplay();
	virtual int NumberOfButtonCols();
	virtual int NumberOfExtraButtonRows();

	virtual void GetDrawstopBlitPosition(const int drawstopRow, const int drawstopCol, int* blitX, int* blitY);
	virtual void GetPushbuttonBlitPosition(const int buttonRow, const int buttonCol, int* blitX, int* blitY);
	virtual wxFont GetControlLabelFont();
	virtual wxFont GetGroupLabelFont();

	virtual int GetEnclosureWidth();
	virtual int GetEnclosureY();
	virtual int GetEnclosureX(const GOGUIEnclosure* enclosure);

	virtual int GetScreenWidth();
	virtual int GetScreenHeight();

	virtual int GetJambLeftRightWidth();
	virtual int GetJambLeftRightHeight();
	virtual int GetJambLeftRightY();
	virtual int GetJambLeftX();
	virtual int GetJambRightX();
	virtual int GetJambTopDrawstop();
	virtual int GetJambTopPiston();
	virtual int GetJambTopHeight();
	virtual int GetJambTopWidth();
	virtual int GetJambTopX();
	virtual int GetJambTopY();
	virtual int GetPistonTopHeight();
	virtual int GetPistonWidth();
	virtual int GetPistonX();
	virtual int GetCenterWidth();
	virtual int GetCenterY();
	virtual int GetCenterX();

	virtual int GetDrawstopBackgroundImageNum();
	virtual int GetConsoleBackgroundImageNum();
	virtual int GetDrawstopInsetBackgroundImageNum();
	virtual int GetKeyVertBackgroundImageNum();
	virtual int GetKeyHorizBackgroundImageNum();

	virtual bool HasPairDrawstopCols();
	virtual bool HasTrimAboveExtraRows();
	virtual bool HasExtraPedalButtonRow();

	virtual unsigned NewEnclosure();

	virtual const MANUAL_RENDER_INFO& GetManualRenderInfo(const unsigned manual_nb) const;

	virtual int GetDrawstopWidth();
	virtual int GetDrawstopHeight();
	virtual int GetButtonWidth();
	virtual int GetButtonHeight();
};

#endif
