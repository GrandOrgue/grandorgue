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

#ifndef GORGUEDISPLAYMETRICS_H
#define GORGUEDISPLAYMETRICS_H

#include <wx/wx.h>

class IniFileConfig;

class GOrgueDisplayMetrics
{

private:

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
	bool m_DispDrawstopColsOffset : 1;
	bool m_DispDrawstopOuterColOffsetUp : 1;
	bool m_DispPairDrawstopCols : 1;
	int m_DispExtraDrawstopRows;
	int m_DispExtraDrawstopCols;
	int m_DispButtonCols;
	int m_DispExtraButtonRows;
	bool m_DispExtraPedalButtonRow : 1;
	bool m_DispExtraPedalButtonRowOffset : 1;
	bool m_DispExtraPedalButtonRowOffsetRight : 1;
	bool m_DispButtonsAboveManuals : 1;
	bool m_DispTrimAboveManuals : 1;
	bool m_DispTrimBelowManuals : 1;
	bool m_DispTrimAboveExtraRows : 1;
	bool m_DispExtraDrawstopRowsAboveExtraButtonRows : 1;

	/* values computed on screen update */
	/* FIXME: some of these are candidates to become locals - I don't
	 * think they are used anywhere outside of Update(); */
//	wxInt16 m_JambLeftX;
//	wxInt16 m_JambTopDrawstop;

	/* not yet exported */
//	wxInt16 m_JambLeftRightWidth;
//	wxInt16 m_JambRightX;
//	wxInt16 m_JambTopHeight;
	int m_HackY;
//	wxInt16 m_JambTopPiston;
	int m_CenterY;
	int m_CenterWidth;
//	wxInt16 m_PistonTopHeight;
//	wxInt16 m_EnclosureWidth;
	int m_EnclosureY;

public:

	GOrgueDisplayMetrics(IniFileConfig& ini);

	int NumberOfExtraDrawstopRowsToDisplay();
	int NumberOfExtraDrawstopColsToDisplay();
	int NumberOfDrawstopColsToDisplay();
	int NumberOfButtonCols();
	int NumberOfExtraButtonRows();


	void GetDrawstopBlitPosition(const int drawstopRow, const int drawstopCol, int* blitX, int* blitY);
	void GetPushbuttonBlitPosition(const int buttonRow, const int buttonCol, int* blitX, int* blitY);
	wxFont GetControlLabelFont();
	wxFont GetGroupLabelFont();

	int GetEnclosureWidth();
	int GetEnclosureY();

	int GetScreenWidth();
	int GetScreenHeight();

	int GetJambLeftRightWidth();
	int GetJambLeftRightHeight();
	int GetJambLeftRightY();
	int GetJambLeftX();
	int GetJambRightX();
	int GetJambTopDrawstop();
	int GetJambTopPiston();
	int GetJambTopHeight();
	int GetJambTopWidth();
	int GetJambTopX();
	int GetJambTopY();
	int GetPistonTopHeight();
	int GetPistonWidth();
	int GetPistonX();
	int GetCenterWidth();
	int GetCenterY();
	int GetCenterX();

	int GetDrawstopBackgroundImageNum();
	int GetConsoleBackgroundImageNum();
	int GetDrawstopInsetBackgroundImageNum();
	int GetKeyVertBackgroundImageNum();
	int GetKeyHorizBackgroundImageNum();

	bool HasPairDrawstopCols();
	bool HasTrimAboveExtraRows();
	bool HasExtraPedalButtonRow();

	void Update();

};

#endif /* GORGUEDISPLAYMETRICS_H */
