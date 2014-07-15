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

#include "GOGUIHW1DisplayMetrics.h"

#include "GOrgueConfigReader.h"
#include "GOrgueManual.h"
#include "GrandOrgueFile.h"

GOGUIHW1DisplayMetrics::GOGUIHW1DisplayMetrics(GOrgueConfigReader& ini, GrandOrgueFile* organfile, wxString group) :
	GOGUIDisplayMetrics(organfile, group)
{
	if (group.IsEmpty())
		m_group = wxT("Organ");

	m_DispScreenSizeHoriz = ini.ReadSize(ODFSetting, m_group, wxT("DispScreenSizeHoriz"), 0);
	m_DispScreenSizeVert = ini.ReadSize(ODFSetting, m_group, wxT("DispScreenSizeVert"), 1);
	m_DispDrawstopBackgroundImageNum = ini.ReadInteger(ODFSetting, m_group, wxT("DispDrawstopBackgroundImageNum"), 1, 64);
	m_DispConsoleBackgroundImageNum = ini.ReadInteger(ODFSetting, m_group, wxT("DispConsoleBackgroundImageNum"), 1, 64);
	m_DispKeyHorizBackgroundImageNum = ini.ReadInteger(ODFSetting, m_group, wxT("DispKeyHorizBackgroundImageNum"), 1, 64);
	m_DispKeyVertBackgroundImageNum = ini.ReadInteger(ODFSetting, m_group, wxT("DispKeyVertBackgroundImageNum"), 1, 64);
	m_DispDrawstopInsetBackgroundImageNum = ini.ReadInteger(ODFSetting, m_group, wxT("DispDrawstopInsetBackgroundImageNum"), 1, 64);
	m_DispControlLabelFont = ini.ReadStringTrim(ODFSetting, m_group, wxT("DispControlLabelFont"));
	m_DispShortcutKeyLabelFont = ini.ReadStringTrim(ODFSetting, m_group, wxT("DispShortcutKeyLabelFont"));
	m_DispShortcutKeyLabelColour = ini.ReadColor(ODFSetting, m_group, wxT("DispShortcutKeyLabelColour"));
	m_DispGroupLabelFont = ini.ReadStringTrim(ODFSetting, m_group, wxT("DispGroupLabelFont"));
	m_DispDrawstopCols = ini.ReadInteger(ODFSetting, m_group, wxT("DispDrawstopCols"), 2, 12);
	m_DispDrawstopRows = ini.ReadInteger(ODFSetting, m_group, wxT("DispDrawstopRows"), 1, 20);
	m_DispDrawstopColsOffset = ini.ReadBoolean(ODFSetting, m_group, wxT("DispDrawstopColsOffset"));
	m_DispDrawstopOuterColOffsetUp = ini.ReadBoolean(ODFSetting, m_group, wxT("DispDrawstopOuterColOffsetUp"), m_DispDrawstopColsOffset);
	m_DispPairDrawstopCols = ini.ReadBoolean(ODFSetting, m_group, wxT("DispPairDrawstopCols"));
	m_DispExtraDrawstopRows = ini.ReadInteger(ODFSetting, m_group, wxT("DispExtraDrawstopRows"), 0, 99);
	m_DispExtraDrawstopCols = ini.ReadInteger(ODFSetting, m_group, wxT("DispExtraDrawstopCols"), 0, 40);
	m_DispButtonCols = ini.ReadInteger(ODFSetting, m_group, wxT("DispButtonCols"), 1, 32);
	m_DispExtraButtonRows = ini.ReadInteger(ODFSetting, m_group, wxT("DispExtraButtonRows"), 0, 99);
	m_DispExtraPedalButtonRow = ini.ReadBoolean(ODFSetting, m_group, wxT("DispExtraPedalButtonRow"));
	m_DispExtraPedalButtonRowOffset = ini.ReadBoolean(ODFSetting, m_group, wxT("DispExtraPedalButtonRowOffset"), m_DispExtraPedalButtonRow);
	m_DispExtraPedalButtonRowOffsetRight = ini.ReadBoolean(ODFSetting, m_group, wxT("DispExtraPedalButtonRowOffsetRight"), m_DispExtraPedalButtonRow);
	m_DispButtonsAboveManuals = ini.ReadBoolean(ODFSetting, m_group, wxT("DispButtonsAboveManuals"));
	m_DispTrimAboveManuals = ini.ReadBoolean(ODFSetting, m_group, wxT("DispTrimAboveManuals"));
	m_DispTrimBelowManuals = ini.ReadBoolean(ODFSetting, m_group, wxT("DispTrimBelowManuals"));
	m_DispTrimAboveExtraRows = ini.ReadBoolean(ODFSetting, m_group, wxT("DispTrimAboveExtraRows"));
	m_DispExtraDrawstopRowsAboveExtraButtonRows = ini.ReadBoolean(ODFSetting, m_group, wxT("DispExtraDrawstopRowsAboveExtraButtonRows"));
	m_DrawStopWidth = ini.ReadInteger(ODFSetting, m_group, wxT("DispDrawstopWidth"), 1, 150, false, 78),
	m_DrawStopHeight = ini.ReadInteger(ODFSetting, m_group, wxT("DispDrawstopHeight"), 1, 150, false, 69),
	m_ButtonWidth = ini.ReadInteger(ODFSetting, m_group, wxT("DispPistonWidth"), 1, 150, false, 44),
	m_ButtonHeight = ini.ReadInteger(ODFSetting, m_group, wxT("DispPistonHeight"), 1, 150, false, 40),
	m_EnclosureWidth = ini.ReadInteger(ODFSetting, m_group, wxT("DispEnclosureWidth"), 1, 150, false, 52),
	m_EnclosureHeight = ini.ReadInteger(ODFSetting, m_group, wxT("DispEnclosureHeight"), 1, 150, false, 63),
	m_PedalHeight = ini.ReadInteger(ODFSetting, m_group, wxT("DispPedalHeight"), 1, 500, false, 40),
	m_PedalKeyWidth = ini.ReadInteger(ODFSetting, m_group, wxT("DispPedalKeyWidth"), 1, 500, false, 7),
	m_ManualHeight = ini.ReadInteger(ODFSetting, m_group, wxT("DispManualHeight"), 1, 500, false, 32),
	m_ManualKeyWidth = ini.ReadInteger(ODFSetting, m_group, wxT("DispManualKeyWidth"), 1, 500, false, 12),

	Init();
}
