/*
 * GrandOrgue - free pipe organ simulator
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

#include "GOGUIHW1DisplayMetrics.h"
#include "IniFileConfig.h"
#include "GOrgueManual.h"
#include "GrandOrgueFile.h"

GOGUIHW1DisplayMetrics::GOGUIHW1DisplayMetrics(IniFileConfig& ini, GrandOrgueFile* organfile, wxString group) :
	GOGUIDisplayMetrics(organfile, group)
{
	bool IsMainPanel = false;

	if (group.IsEmpty())
	{
		m_group = wxT("Organ");
		IsMainPanel = true;
	}

	m_nb_enclosures = ini.ReadInteger(m_group, wxT("NumberOfEnclosures"), 0, organfile->GetEnclosureCount());
	m_nb_manuals    = ini.ReadInteger(m_group, wxT("NumberOfManuals"), IsMainPanel ? 1 : 0, organfile->GetManualAndPedalCount());
	m_first_manual  = ini.ReadBoolean(m_group, wxT("HasPedals")) ? 0 : 1;
	if (m_first_manual < organfile->GetFirstManualIndex())
		m_first_manual = organfile->GetFirstManualIndex();

	for (unsigned int i = 0; i <= m_nb_manuals; i++)
	{
		wxString buffer;
		MANUAL_INFO man;
		memset(&man, 0, sizeof(man));
		if (i >= m_first_manual)
		{
			man.displayed                         = !IsMainPanel || m_organfile->GetManual(i)->IsDisplayed();
			unsigned manual_nb = i;
			if (!IsMainPanel)
			{
				wxString Buffer;
				buffer.Printf(wxT("Manual%03d"), i);
				manual_nb  = ini.ReadInteger(m_group, buffer, m_organfile->GetFirstManualIndex(), m_organfile->GetManualAndPedalCount());
			}
			man.first_accessible_key_midi_note_nb = m_organfile->GetManual(manual_nb)->GetFirstAccessibleKeyMIDINoteNumber();
			man.nb_accessible_keys                = m_organfile->GetManual(manual_nb)->GetNumberOfAccessibleKeys();
		}
		m_manual_info.push_back(man);
	}

	m_DispScreenSizeHoriz = ini.ReadSize(m_group, wxT("DispScreenSizeHoriz"), 0);
	m_DispScreenSizeVert = ini.ReadSize(m_group, wxT("DispScreenSizeVert"), 1);
	m_DispDrawstopBackgroundImageNum = ini.ReadInteger(m_group, wxT("DispDrawstopBackgroundImageNum"), 1, 64);
	m_DispConsoleBackgroundImageNum = ini.ReadInteger(m_group, wxT("DispConsoleBackgroundImageNum"), 1, 64);
	m_DispKeyHorizBackgroundImageNum = ini.ReadInteger(m_group, wxT("DispKeyHorizBackgroundImageNum"), 1, 64);
	m_DispKeyVertBackgroundImageNum = ini.ReadInteger(m_group, wxT("DispKeyVertBackgroundImageNum"), 1, 64);
	m_DispDrawstopInsetBackgroundImageNum = ini.ReadInteger(m_group, wxT("DispDrawstopInsetBackgroundImageNum"), 1, 64);
	m_DispControlLabelFont = ini.ReadString(m_group, wxT("DispControlLabelFont"));
	m_DispShortcutKeyLabelFont = ini.ReadString(m_group, wxT("DispShortcutKeyLabelFont"));
	m_DispShortcutKeyLabelColour = ini.ReadColor(m_group, wxT("DispShortcutKeyLabelColour"));
	m_DispGroupLabelFont = ini.ReadString(m_group, wxT("DispGroupLabelFont"));
	m_DispDrawstopCols = ini.ReadInteger(m_group, wxT("DispDrawstopCols"), 2, 12);
	m_DispDrawstopRows = ini.ReadInteger(m_group, wxT("DispDrawstopRows"), 1, 20);
	m_DispDrawstopColsOffset = ini.ReadBoolean(m_group, wxT("DispDrawstopColsOffset"));
	m_DispDrawstopOuterColOffsetUp = ini.ReadBoolean(m_group, wxT("DispDrawstopOuterColOffsetUp"), m_DispDrawstopColsOffset);
	m_DispPairDrawstopCols = ini.ReadBoolean(m_group, wxT("DispPairDrawstopCols"));
	m_DispExtraDrawstopRows = ini.ReadInteger(m_group, wxT("DispExtraDrawstopRows"), 0, 8);
	m_DispExtraDrawstopCols = ini.ReadInteger(m_group, wxT("DispExtraDrawstopCols"), 0, 40);
	m_DispButtonCols = ini.ReadInteger(m_group, wxT("DispButtonCols"), 1, 32);
	m_DispExtraButtonRows = ini.ReadInteger(m_group, wxT("DispExtraButtonRows"), 0, 8);
	m_DispExtraPedalButtonRow = ini.ReadBoolean(m_group, wxT("DispExtraPedalButtonRow"));
	m_DispExtraPedalButtonRowOffset = ini.ReadBoolean(m_group, wxT("DispExtraPedalButtonRowOffset"), m_DispExtraPedalButtonRow);
	m_DispExtraPedalButtonRowOffsetRight = ini.ReadBoolean(m_group, wxT("DispExtraPedalButtonRowOffsetRight"), m_DispExtraPedalButtonRow);
	m_DispButtonsAboveManuals = ini.ReadBoolean(m_group, wxT("DispButtonsAboveManuals"));
	m_DispTrimAboveManuals = ini.ReadBoolean(m_group, wxT("DispTrimAboveManuals"));
	m_DispTrimBelowManuals = ini.ReadBoolean(m_group, wxT("DispTrimBelowManuals"));
	m_DispTrimAboveExtraRows = ini.ReadBoolean(m_group, wxT("DispTrimAboveExtraRows"));
	m_DispExtraDrawstopRowsAboveExtraButtonRows = ini.ReadBoolean(m_group, wxT("DispExtraDrawstopRowsAboveExtraButtonRows"));

	Update();
}
