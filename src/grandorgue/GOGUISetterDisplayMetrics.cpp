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

#include "GOGUISetterDisplayMetrics.h"
#include "GrandOrgueFile.h"
#include "IniFileConfig.h"

GOGUISetterDisplayMetrics::GOGUISetterDisplayMetrics(IniFileConfig& ini, GrandOrgueFile* organfile, wxString group, GOGUISetterType type) :
	GOGUIDisplayMetrics(organfile, group)
{
	wxString x_size, y_size;
	int drawstop_rows, drawstop_cols;
	int button_rows, button_cols;

	switch (type)
	{
	case GOGUI_SETTER_SETTER:
		x_size = wxT("800");
		y_size = wxT("300");
		drawstop_rows = 3;
		drawstop_cols = 10;
		button_cols = 10;
		button_rows = 0;
		break;

	case GOGUI_SETTER_CRESCENDO:
		x_size = wxT("700");
		y_size = wxT("300");
		drawstop_rows = 2;
		drawstop_cols = 8;
		button_cols = 10;
		button_rows = 0;
		break;

	case GOGUI_SETTER_DIVISIONALS:
		x_size = wxT("600");
		y_size = wxString::Format(wxT("%d"), 40 * (organfile->GetManualAndPedalCount() + 1) + 140);
		drawstop_rows = 1;
		drawstop_cols = 7;
		button_cols = 13;
		button_rows = organfile->GetManualAndPedalCount() + 1;
		break;

	case GOGUI_SETTER_COUPLER:
		x_size = wxT("500");
		y_size = wxString::Format(wxT("%d"), 20 + 80 * (organfile->GetManualAndPedalCount() + 1));;
		drawstop_rows = organfile->GetManualAndPedalCount() + 1;
		drawstop_cols = 6;
		button_cols = 10;
		button_rows = 0;
		break;

	case GOGUI_SETTER_GENERALS:
	default:
		x_size = wxT("600");
		y_size = wxT("250");
		drawstop_rows = 1;
		drawstop_cols = 7;
		button_cols = 10;
		button_rows = 3;
	}

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

	int background_img = ini.ReadInteger(wxT("Organ"), wxT("DispConsoleBackgroundImageNum"), 1, 64, false, 32);

	m_DispScreenSizeHoriz = ini.ReadSize(m_group, wxT("DispScreenSizeHoriz"), 0, false, x_size);
	m_DispScreenSizeVert = ini.ReadSize(m_group, wxT("DispScreenSizeVert"), 1, false, y_size);
	m_DispDrawstopBackgroundImageNum = ini.ReadInteger(m_group, wxT("DispDrawstopBackgroundImageNum"), 1, 64, false, background_img);
	m_DispConsoleBackgroundImageNum = ini.ReadInteger(m_group, wxT("DispConsoleBackgroundImageNum"), 1, 64, false, background_img);
	m_DispKeyHorizBackgroundImageNum = ini.ReadInteger(m_group, wxT("DispKeyHorizBackgroundImageNum"), 1, 64, false, background_img);
	m_DispKeyVertBackgroundImageNum = ini.ReadInteger(m_group, wxT("DispKeyVertBackgroundImageNum"), 1, 64, false, background_img);
	m_DispDrawstopInsetBackgroundImageNum = ini.ReadInteger(m_group, wxT("DispDrawstopInsetBackgroundImageNum"), 1, 64, false, background_img);
	m_DispControlLabelFont = ini.ReadString(m_group, wxT("DispControlLabelFont"), 255, false, wxT("Times New Roman"));
	m_DispShortcutKeyLabelFont = ini.ReadString(m_group, wxT("DispShortcutKeyLabelFont"), 255, false, wxT("Times New Roman"));
	m_DispShortcutKeyLabelColour = ini.ReadColor(m_group, wxT("DispShortcutKeyLabelColour"), false, wxT("Yellow"));
	m_DispGroupLabelFont = ini.ReadString(m_group, wxT("DispGroupLabelFont"), 255, false, wxT("Times New Roman"));
	m_DispDrawstopCols = ini.ReadInteger(m_group, wxT("DispDrawstopCols"), 0, 12, false, 0);
	m_DispDrawstopRows = ini.ReadInteger(m_group, wxT("DispDrawstopRows"), 0, 20, false, 0);
	m_DispDrawstopColsOffset = ini.ReadBoolean(m_group, wxT("DispDrawstopColsOffset"), false, false);
	m_DispDrawstopOuterColOffsetUp = ini.ReadBoolean(m_group, wxT("DispDrawstopOuterColOffsetUp"), false, false);
	m_DispPairDrawstopCols = ini.ReadBoolean(m_group, wxT("DispPairDrawstopCols"), false, false);
	m_DispExtraDrawstopRows = ini.ReadInteger(m_group, wxT("DispExtraDrawstopRows"), 0, 99, false, drawstop_rows);
	m_DispExtraDrawstopCols = ini.ReadInteger(m_group, wxT("DispExtraDrawstopCols"), 0, 40, false, drawstop_cols);
	m_DispButtonCols = ini.ReadInteger(m_group, wxT("DispButtonCols"), 1, 32, false, button_cols);
	m_DispExtraButtonRows = ini.ReadInteger(m_group, wxT("DispExtraButtonRows"), 0, 8, false, button_rows);
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
