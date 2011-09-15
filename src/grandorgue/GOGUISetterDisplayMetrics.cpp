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
#include "IniFileConfig.h"

GOGUISetterDisplayMetrics::GOGUISetterDisplayMetrics(IniFileConfig& ini, GrandOrgueFile* organfile, wxString group, GOGUISetterType type) :
	GOGUIDisplayMetrics(organfile, group)
{
	bool IsSetter = type == GOGUI_SETTER_SETTER;
	bool IsCrescendo = type == GOGUI_SETTER_CRESCENDO;

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

	m_DispScreenSizeHoriz = ini.ReadSize(m_group, wxT("DispScreenSizeHoriz"), 0, false, IsSetter ? wxT("800") : IsCrescendo ? wxT("700") : wxT("600"));
	m_DispScreenSizeVert = ini.ReadSize(m_group, wxT("DispScreenSizeVert"), 1, false, IsSetter ? wxT("300") : IsCrescendo ? wxT("300") : wxT("250"));
	m_DispDrawstopBackgroundImageNum = ini.ReadInteger(m_group, wxT("DispDrawstopBackgroundImageNum"), 1, 64, false, 31);
	m_DispConsoleBackgroundImageNum = ini.ReadInteger(m_group, wxT("DispConsoleBackgroundImageNum"), 1, 64, false, 32);
	m_DispKeyHorizBackgroundImageNum = ini.ReadInteger(m_group, wxT("DispKeyHorizBackgroundImageNum"), 1, 64, false, 32);
	m_DispKeyVertBackgroundImageNum = ini.ReadInteger(m_group, wxT("DispKeyVertBackgroundImageNum"), 1, 64, false, 31);
	m_DispDrawstopInsetBackgroundImageNum = ini.ReadInteger(m_group, wxT("DispDrawstopInsetBackgroundImageNum"), 1, 64, false, 33);
	m_DispControlLabelFont = ini.ReadString(m_group, wxT("DispControlLabelFont"), 255, false, wxT("Times New Roman"));
	m_DispShortcutKeyLabelFont = ini.ReadString(m_group, wxT("DispShortcutKeyLabelFont"), 255, false, wxT("Times New Roman"));
	m_DispShortcutKeyLabelColour = ini.ReadColor(m_group, wxT("DispShortcutKeyLabelColour"), false, wxT("Yellow"));
	m_DispGroupLabelFont = ini.ReadString(m_group, wxT("DispGroupLabelFont"), 255, false, wxT("Times New Roman"));
	m_DispDrawstopCols = ini.ReadInteger(m_group, wxT("DispDrawstopCols"), 0, 12, false, 0);
	m_DispDrawstopRows = ini.ReadInteger(m_group, wxT("DispDrawstopRows"), 0, 20, false, 0);
	m_DispDrawstopColsOffset = ini.ReadBoolean(m_group, wxT("DispDrawstopColsOffset"), false, false);
	m_DispDrawstopOuterColOffsetUp = ini.ReadBoolean(m_group, wxT("DispDrawstopOuterColOffsetUp"), false, false);
	m_DispPairDrawstopCols = ini.ReadBoolean(m_group, wxT("DispPairDrawstopCols"), false, false);
	m_DispExtraDrawstopRows = ini.ReadInteger(m_group, wxT("DispExtraDrawstopRows"), 0, 8, false, IsSetter ? 3 : IsCrescendo ? 2: 1);
	m_DispExtraDrawstopCols = ini.ReadInteger(m_group, wxT("DispExtraDrawstopCols"), 0, 40, false, IsSetter ? 10 : IsCrescendo ? 8 : 7);
	m_DispButtonCols = ini.ReadInteger(m_group, wxT("DispButtonCols"), 1, 32, false, 10);
	m_DispExtraButtonRows = ini.ReadInteger(m_group, wxT("DispExtraButtonRows"), 0, 8, false, IsSetter ? 0 : IsCrescendo ? 0 : 3);
	m_DispExtraPedalButtonRow = ini.ReadBoolean(m_group, wxT("DispExtraPedalButtonRow"), false, false);
	m_DispExtraPedalButtonRowOffset = ini.ReadBoolean(m_group, wxT("DispExtraPedalButtonRowOffset"), false, false);
	m_DispExtraPedalButtonRowOffsetRight = ini.ReadBoolean(m_group, wxT("DispExtraPedalButtonRowOffsetRight"), false, false);
	m_DispButtonsAboveManuals = ini.ReadBoolean(m_group, wxT("DispButtonsAboveManuals"), false, true);
	m_DispTrimAboveManuals = ini.ReadBoolean(m_group, wxT("DispTrimAboveManuals"), false, true);
	m_DispTrimBelowManuals = ini.ReadBoolean(m_group, wxT("DispTrimBelowManuals"), false, true);
	m_DispTrimAboveExtraRows = ini.ReadBoolean(m_group, wxT("DispTrimAboveExtraRows"), false, true);
	m_DispExtraDrawstopRowsAboveExtraButtonRows = ini.ReadBoolean(m_group, wxT("DispExtraDrawstopRowsAboveExtraButtonRows"), false, true);

	Update();
}
