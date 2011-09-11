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

#include "GOGUISetterDisplayMetrics.h"
#include "GrandOrgueFile.h"

GOGUISetterDisplayMetrics::GOGUISetterDisplayMetrics(GrandOrgueFile* organfile, wxString group) :
	GOGUIDisplayMetrics(organfile, group)
{
	m_nb_enclosures = 0;
	m_nb_manuals    = 0;
	m_first_manual  = 1;

	for (unsigned int i = 0; i <= m_nb_manuals; i++)
	{
		wxString buffer;
		MANUAL_INFO man;
		memset(&man, 0, sizeof(man));
		m_manual_info.push_back(man);
	}

	m_DispScreenSizeHoriz = 800;
	m_DispScreenSizeVert = 300;
	m_DispDrawstopBackgroundImageNum = 31;
	m_DispConsoleBackgroundImageNum = 32;
	m_DispKeyHorizBackgroundImageNum = 32;
	m_DispKeyVertBackgroundImageNum = 31;
	m_DispDrawstopInsetBackgroundImageNum = 33;
	m_DispControlLabelFont = wxT("Times New Roman");
	m_DispShortcutKeyLabelFont = wxT("Times New Roman");
	m_DispShortcutKeyLabelColour = wxColour(0xFF, 0xFF, 0x00);
	m_DispGroupLabelFont = wxT("Times New Roman");
	m_DispDrawstopCols = 0;
	m_DispDrawstopRows = 0;
	m_DispDrawstopColsOffset = false;
	m_DispDrawstopOuterColOffsetUp = false;
	m_DispPairDrawstopCols = false;
	m_DispExtraDrawstopRows = 3;
	m_DispExtraDrawstopCols = 10;
	m_DispButtonCols = 10;
	m_DispExtraButtonRows = 0;
	m_DispExtraPedalButtonRow = false;
	m_DispExtraPedalButtonRowOffset = false;
	m_DispExtraPedalButtonRowOffsetRight = false;
	m_DispButtonsAboveManuals = true;
	m_DispTrimAboveManuals = true;
	m_DispTrimBelowManuals = true;
	m_DispTrimAboveExtraRows = true;
	m_DispExtraDrawstopRowsAboveExtraButtonRows = true;

	Update();
}
