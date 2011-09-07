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

#include "GOGUIDisplayMetrics.h"
#include "IniFileConfig.h"
#include "GOGUIEnclosure.h"

#define DISPLAY_METRICS_GROUP wxT("Organ")

GOGUIDisplayMetrics::GOGUIDisplayMetrics(IniFileConfig& ini) :
	m_nb_enclosures(0),
	m_nb_manuals(0),
	m_first_manual(0),
	m_manual_info(),
	m_DispScreenSizeHoriz(0),
	m_DispScreenSizeVert(0),
	m_DispDrawstopBackgroundImageNum(0),
	m_DispConsoleBackgroundImageNum(0),
	m_DispKeyHorizBackgroundImageNum(0),
	m_DispKeyVertBackgroundImageNum(0),
	m_DispDrawstopInsetBackgroundImageNum(0),
	m_DispControlLabelFont(),
	m_DispShortcutKeyLabelFont(),
	m_DispShortcutKeyLabelColour(),
	m_DispGroupLabelFont(),
	m_DispDrawstopCols(0),
	m_DispDrawstopRows(0),
	m_DispDrawstopColsOffset(false),
	m_DispDrawstopOuterColOffsetUp(false),
	m_DispPairDrawstopCols(false),
	m_DispExtraDrawstopRows(0),
	m_DispExtraDrawstopCols(0),
	m_DispButtonCols(0),
	m_DispExtraButtonRows(0),
	m_DispExtraPedalButtonRow(false),
	m_DispExtraPedalButtonRowOffset(false),
	m_DispExtraPedalButtonRowOffsetRight(false),
	m_DispButtonsAboveManuals(false),
	m_DispTrimAboveManuals(false),
	m_DispTrimBelowManuals(false),
	m_DispTrimAboveExtraRows(false),
	m_DispExtraDrawstopRowsAboveExtraButtonRows(false),
	m_HackY(0),
	m_EnclosureY(0),
	m_CenterY(0),
	m_CenterWidth(0)
{

	m_nb_enclosures = ini.ReadInteger(wxT("Organ"), wxT("NumberOfEnclosures"), 0, 6);
	m_nb_manuals    = ini.ReadInteger(wxT("Organ"), wxT("NumberOfManuals"), 1, 6);
	m_first_manual  = ini.ReadBoolean(wxT("Organ"), wxT("HasPedals")) ? 0 : 1;

	for (unsigned int i = 0; i <= m_nb_manuals; i++)
	{
		wxString buffer;
		MANUAL_INFO man;
		memset(&man, 0, sizeof(man));
		if (i >= m_first_manual)
		{
			buffer.Printf(wxT("Manual%03u"), i);
			man.displayed                         = ini.ReadBoolean(buffer, wxT("Displayed"));
			man.first_accessible_key_midi_note_nb = ini.ReadInteger(buffer, wxT("FirstAccessibleKeyMIDINoteNumber"), 0, 127);
			man.nb_accessible_keys                = ini.ReadInteger(buffer, wxT("NumberOfAccessibleKeys"), 0, 85);
		}
		m_manual_info.push_back(man);
	}

    m_DispScreenSizeHoriz = ini.ReadSize(DISPLAY_METRICS_GROUP, wxT("DispScreenSizeHoriz"), 0);
    m_DispScreenSizeVert = ini.ReadSize(DISPLAY_METRICS_GROUP, wxT("DispScreenSizeVert"), 1);
    m_DispDrawstopBackgroundImageNum = ini.ReadInteger(DISPLAY_METRICS_GROUP, wxT("DispDrawstopBackgroundImageNum"), 1, 64);
    m_DispConsoleBackgroundImageNum = ini.ReadInteger(DISPLAY_METRICS_GROUP, wxT("DispConsoleBackgroundImageNum"), 1, 64);
    m_DispKeyHorizBackgroundImageNum = ini.ReadInteger(DISPLAY_METRICS_GROUP, wxT("DispKeyHorizBackgroundImageNum"), 1, 64);
    m_DispKeyVertBackgroundImageNum = ini.ReadInteger(DISPLAY_METRICS_GROUP, wxT("DispKeyVertBackgroundImageNum"), 1, 64);
    m_DispDrawstopInsetBackgroundImageNum = ini.ReadInteger(DISPLAY_METRICS_GROUP, wxT("DispDrawstopInsetBackgroundImageNum"), 1, 64);
    m_DispControlLabelFont = ini.ReadString(DISPLAY_METRICS_GROUP, wxT("DispControlLabelFont"));
    m_DispShortcutKeyLabelFont = ini.ReadString(DISPLAY_METRICS_GROUP, wxT("DispShortcutKeyLabelFont"));
    m_DispShortcutKeyLabelColour = ini.ReadColor(DISPLAY_METRICS_GROUP, wxT("DispShortcutKeyLabelColour"));
    m_DispGroupLabelFont = ini.ReadString(DISPLAY_METRICS_GROUP, wxT("DispGroupLabelFont"));
    m_DispDrawstopCols = ini.ReadInteger(DISPLAY_METRICS_GROUP, wxT("DispDrawstopCols"), 2, 12);
    m_DispDrawstopRows = ini.ReadInteger(DISPLAY_METRICS_GROUP, wxT("DispDrawstopRows"), 1, 20);
    m_DispDrawstopColsOffset = ini.ReadBoolean(DISPLAY_METRICS_GROUP, wxT("DispDrawstopColsOffset"));
    m_DispDrawstopOuterColOffsetUp = ini.ReadBoolean(DISPLAY_METRICS_GROUP, wxT("DispDrawstopOuterColOffsetUp"), m_DispDrawstopColsOffset);
    m_DispPairDrawstopCols = ini.ReadBoolean(DISPLAY_METRICS_GROUP, wxT("DispPairDrawstopCols"));
    m_DispExtraDrawstopRows = ini.ReadInteger(DISPLAY_METRICS_GROUP, wxT("DispExtraDrawstopRows"), 0, 8);
    m_DispExtraDrawstopCols = ini.ReadInteger(DISPLAY_METRICS_GROUP, wxT("DispExtraDrawstopCols"), 0, 40);
    m_DispButtonCols = ini.ReadInteger(DISPLAY_METRICS_GROUP, wxT("DispButtonCols"), 1, 32);
    m_DispExtraButtonRows = ini.ReadInteger(DISPLAY_METRICS_GROUP, wxT("DispExtraButtonRows"), 0, 8);
    m_DispExtraPedalButtonRow = ini.ReadBoolean(DISPLAY_METRICS_GROUP, wxT("DispExtraPedalButtonRow"));
    m_DispExtraPedalButtonRowOffset = ini.ReadBoolean(DISPLAY_METRICS_GROUP, wxT("DispExtraPedalButtonRowOffset"), m_DispExtraPedalButtonRow);
    m_DispExtraPedalButtonRowOffsetRight = ini.ReadBoolean(DISPLAY_METRICS_GROUP, wxT("DispExtraPedalButtonRowOffsetRight"), m_DispExtraPedalButtonRow);
    m_DispButtonsAboveManuals = ini.ReadBoolean(DISPLAY_METRICS_GROUP, wxT("DispButtonsAboveManuals"));
    m_DispTrimAboveManuals = ini.ReadBoolean(DISPLAY_METRICS_GROUP, wxT("DispTrimAboveManuals"));
    m_DispTrimBelowManuals = ini.ReadBoolean(DISPLAY_METRICS_GROUP, wxT("DispTrimBelowManuals"));
    m_DispTrimAboveExtraRows = ini.ReadBoolean(DISPLAY_METRICS_GROUP, wxT("DispTrimAboveExtraRows"));
    m_DispExtraDrawstopRowsAboveExtraButtonRows = ini.ReadBoolean(DISPLAY_METRICS_GROUP, wxT("DispExtraDrawstopRowsAboveExtraButtonRows"));

    Update();

}

/*
 * BASIC EXPORTS STRAIGHT FROM ODF
 */

int GOGUIDisplayMetrics::NumberOfExtraDrawstopRowsToDisplay()
{
	return m_DispExtraDrawstopRows;
}

int GOGUIDisplayMetrics::NumberOfExtraDrawstopColsToDisplay()
{
	return m_DispExtraDrawstopCols;
}

int GOGUIDisplayMetrics::NumberOfDrawstopColsToDisplay()
{
	return m_DispDrawstopCols;
}

int GOGUIDisplayMetrics::NumberOfButtonCols()
{
	return m_DispButtonCols;
}

int GOGUIDisplayMetrics::NumberOfExtraButtonRows()
{
	return m_DispExtraButtonRows;
}

int GOGUIDisplayMetrics::GetDrawstopBackgroundImageNum()
{
	return m_DispDrawstopBackgroundImageNum;
}

int GOGUIDisplayMetrics::GetConsoleBackgroundImageNum()
{
	return m_DispConsoleBackgroundImageNum;
}

int GOGUIDisplayMetrics::GetDrawstopInsetBackgroundImageNum()
{
	return m_DispDrawstopInsetBackgroundImageNum;
}

int GOGUIDisplayMetrics::GetKeyVertBackgroundImageNum()
{
	return m_DispKeyVertBackgroundImageNum;
}

int GOGUIDisplayMetrics::GetKeyHorizBackgroundImageNum()
{
	return m_DispKeyHorizBackgroundImageNum;
}

bool GOGUIDisplayMetrics::HasPairDrawstopCols()
{
	return m_DispPairDrawstopCols;
}

bool GOGUIDisplayMetrics::HasTrimAboveExtraRows()
{
	return m_DispTrimAboveExtraRows;
}

bool GOGUIDisplayMetrics::HasExtraPedalButtonRow()
{
	return m_DispExtraPedalButtonRow;
}

wxFont GOGUIDisplayMetrics::GetControlLabelFont()
{
	wxFont font = *wxNORMAL_FONT;
	font.SetFaceName(m_DispControlLabelFont);
	return font;
}

wxFont GOGUIDisplayMetrics::GetGroupLabelFont()
{
	wxFont font = *wxNORMAL_FONT;
#ifdef linux
	font.SetFamily(wxFONTFAMILY_SWISS);
#endif
	font.SetFaceName(m_DispGroupLabelFont);
	return font;
}

int GOGUIDisplayMetrics::GetScreenWidth()
{
	return m_DispScreenSizeHoriz;
}

int GOGUIDisplayMetrics::GetScreenHeight()
{
	return m_DispScreenSizeVert;
}

/*
 * COMPUTED VALUES FROM ODF PARAMETERS
 */

int GOGUIDisplayMetrics::GetJambLeftRightHeight()
{
	return (m_DispDrawstopRows + 1) * 69;
}

int GOGUIDisplayMetrics::GetJambLeftRightY()
{
	return (m_DispScreenSizeVert - GetJambLeftRightHeight() - (m_DispDrawstopColsOffset ? 35 : 0)) >> 1;
}

int GOGUIDisplayMetrics::GetJambLeftRightWidth()
{
	int jamblrw = m_DispDrawstopCols * 39;
	if (m_DispPairDrawstopCols)
		jamblrw += ((m_DispDrawstopCols >> 2) * 18) - 8;
	return jamblrw;
}

int GOGUIDisplayMetrics::GetJambTopHeight()
{
	return m_DispExtraDrawstopRows * 69;
}

int GOGUIDisplayMetrics::GetJambTopWidth()
{
	return m_DispExtraDrawstopCols * 78;
}

int GOGUIDisplayMetrics::GetJambTopX()
{
	return (m_DispScreenSizeHoriz - GetJambTopWidth()) >> 1;
}

int GOGUIDisplayMetrics::GetPistonTopHeight()
{
	return m_DispExtraButtonRows * 40;
}

int GOGUIDisplayMetrics::GetPistonWidth()
{
	return m_DispButtonCols * 44;
}

int GOGUIDisplayMetrics::GetPistonX()
{
	return (m_DispScreenSizeHoriz - GetPistonWidth()) >> 1;
}

/*
 * COMPUTED VALUES FROM ODF PARAMETERS & SCREEN
 */

int GOGUIDisplayMetrics::GetCenterWidth()
{
	return m_CenterWidth;
}

int GOGUIDisplayMetrics::GetCenterX()
{
	return (m_DispScreenSizeHoriz - m_CenterWidth) >> 1;
}

int GOGUIDisplayMetrics::GetCenterY()
{
	return m_CenterY;
}

int GOGUIDisplayMetrics::GetEnclosureWidth()
{
	return 52 * m_nb_enclosures;
}

int GOGUIDisplayMetrics::GetEnclosureY()
{
	return m_EnclosureY;
}

int GOGUIDisplayMetrics::GetEnclosureX(const GOGUIEnclosure* enclosure)
{

	assert(enclosure);

	int enclosure_x = (GetScreenWidth() - GetEnclosureWidth() + 6) >> 1;
	for (unsigned int i = 0; i < m_nb_enclosures; i++)
	{
		if (enclosure->IsEnclosure(i))
			return enclosure_x;
		enclosure_x += 52;
	}

	throw (wxString)_("enclosure not found");

}

int GOGUIDisplayMetrics::GetJambLeftX()
{
	int jamblx = (GetCenterX() - GetJambLeftRightWidth()) >> 1;
	if (m_DispPairDrawstopCols)
		jamblx += 5;
	return jamblx;
}

int GOGUIDisplayMetrics::GetJambRightX()
{
	int jambrx = GetJambLeftX() + GetCenterX() + m_CenterWidth;
	if (m_DispPairDrawstopCols)
		jambrx += 5;
	return jambrx;
}

int GOGUIDisplayMetrics::GetJambTopDrawstop()
{
	if (m_DispTrimAboveExtraRows)
		return m_CenterY + 8;
	return m_CenterY;
}

int GOGUIDisplayMetrics::GetJambTopPiston()
{
	if (m_DispTrimAboveExtraRows)
		return m_CenterY + 8;
	return m_CenterY;
}

int GOGUIDisplayMetrics::GetJambTopY()
{
	if (m_DispTrimAboveExtraRows)
		return m_CenterY + 8;
	return m_CenterY;
}

/*
 * BLIT POSITION FUNCTIONS
 */

void GOGUIDisplayMetrics::GetDrawstopBlitPosition(const int drawstopRow, const int drawstopCol, int* blitX, int* blitY)
{
	// FIXME: check blitX and blitY for null ptr
	int i;
	if (drawstopRow > 99)
	{
		*blitX = GetJambTopX() + (drawstopCol - 1) * 78 + 6;
		if (m_DispExtraDrawstopRowsAboveExtraButtonRows)
		{
		    *blitY = GetJambTopDrawstop() + (drawstopRow - 100) * 69 + 2;
		}
		else
		{
            *blitY = GetJambTopDrawstop() + (drawstopRow - 100) * 69 + (m_DispExtraButtonRows * 40) + 2;
		}
	}
	else
	{
		i = m_DispDrawstopCols >> 1;
		if (drawstopCol <= i)
		{
			*blitX = GetJambLeftX() + (drawstopCol - 1) * 78 + 6;
			*blitY = GetJambLeftRightY() + (drawstopRow - 1) * 69 + 32;
		}
		else
		{
			*blitX = GetJambRightX() + (drawstopCol - 1 - i) * 78 + 6;
			*blitY = GetJambLeftRightY() + (drawstopRow - 1) * 69 + 32;
		}
		if (m_DispPairDrawstopCols)
			*blitX += (((drawstopCol - 1) % i) >> 1) * 18;

		if (drawstopCol <= i)
			i = drawstopCol;
		else
			i = m_DispDrawstopCols - drawstopCol + 1;
		if (m_DispDrawstopColsOffset && (i & 1) ^ m_DispDrawstopOuterColOffsetUp)
			*blitY += 35;

	}
}

void GOGUIDisplayMetrics::GetPushbuttonBlitPosition(const int buttonRow, const int buttonCol, int* blitX, int* blitY)
{

	*blitX = GetPistonX() + (buttonCol - 1) * 44 + 6;
	if (buttonRow > 99)
	{
	    if (m_DispExtraDrawstopRowsAboveExtraButtonRows)
	    {
        *blitY = GetJambTopPiston() + (buttonRow - 100) * 40 + (m_DispExtraDrawstopRows * 69) + 5;
	    }
		else
		{
		*blitY = GetJambTopPiston() + (buttonRow - 100) * 40 + 5;
		}
	}
	else
	{
		int i = buttonRow;
		if (i == 99)
			i = 0;

		if (i > (int)m_nb_manuals)
			*blitY = m_HackY - (i - (int)m_nb_manuals) * 72 + 32 + 5;
		else
			*blitY = m_manual_info[i].render_info.piston_y + 5;

		if (m_DispExtraPedalButtonRow && !buttonRow)
			*blitY += 40;
		if (m_DispExtraPedalButtonRowOffset && buttonRow == 99)
			*blitX -= 22;
	}

}

/*
 * UPDATE METHOD - updates parameters dependent on ODF parameters and
 * screen stuff.
 */

/* TODO: this method could do with a cleanup */

void GOGUIDisplayMetrics::Update()
{

	m_CenterY = m_DispScreenSizeVert - 40;
	m_CenterWidth = std::max(GetJambTopWidth(), GetPistonWidth());

	for (unsigned i = 0; i <= m_nb_manuals; i++)
	{

		if (!i)
		{
			m_manual_info[0].render_info.height = 40;
			m_manual_info[0].render_info.keys_y = m_manual_info[0].render_info.y = m_CenterY;
			m_CenterY -= 40;
			if (m_DispExtraPedalButtonRow)
				m_CenterY -= 40;
			m_manual_info[0].render_info.piston_y = m_CenterY;
			m_CenterY -= 87;
			m_CenterWidth = std::max(m_CenterWidth, GetEnclosureWidth());
			m_EnclosureY = m_CenterY + 12;
		}

		if (!m_manual_info[i].displayed || i < m_first_manual)
			continue;

		if (i)
		{
			if (!m_DispButtonsAboveManuals)
			{
				m_CenterY -= 40;
				m_manual_info[i].render_info.piston_y = m_CenterY;
			}
			m_manual_info[i].render_info.height = 32;
			if (m_DispTrimBelowManuals && i == 1)
			{
				m_manual_info[i].render_info.height += 8;
				m_CenterY -= 8;
			}
			m_CenterY -= 32;
			m_manual_info[i].render_info.keys_y = m_CenterY;
			if (m_DispTrimAboveManuals && i == m_nb_manuals)
			{
				m_CenterY -= 8;
				m_manual_info[i].render_info.height += 8;
			}
			if (m_DispButtonsAboveManuals)
			{
				m_CenterY -= 40;
				m_manual_info[i].render_info.piston_y = m_CenterY;
			}
			m_manual_info[i].render_info.y = m_CenterY;
		}
		m_manual_info[i].render_info.width = 1;
		if (i)
		{
			for (unsigned j = 0; j < m_manual_info[i].nb_accessible_keys; j++)
			{
				int k = (m_manual_info[i].first_accessible_key_midi_note_nb + j) % 12;
				if ((k < 5 && !(k & 1)) || (k >= 5 && (k & 1)))
					m_manual_info[i].render_info.width += 12;
			}
		}
		else
		{
			for (unsigned j = 0; j < m_manual_info[i].nb_accessible_keys; j++)
			{
				m_manual_info[i].render_info.width += 7;
				int k = (m_manual_info[i].first_accessible_key_midi_note_nb + j) % 12;
				if (j && (!k || k == 5))
					m_manual_info[i].render_info.width += 7;
			}
		}
		m_manual_info[i].render_info.x = (m_DispScreenSizeHoriz - m_manual_info[i].render_info.width) >> 1;
		m_manual_info[i].render_info.width += 16;
		if (m_manual_info[i].render_info.width > m_CenterWidth)
			m_CenterWidth = m_manual_info[i].render_info.width;
	}

  /*
	for (; i <= 6; i++)		// evil: for jpOtt
	organfile->m_manual[i].m_PistonY = organfile->m_CenterY - 40;
  */

	m_HackY = m_CenterY;

	if (m_CenterWidth + GetJambLeftRightWidth() * 2 < m_DispScreenSizeHoriz)
		m_CenterWidth += (m_DispScreenSizeHoriz - m_CenterWidth - GetJambLeftRightWidth() * 2) / 3;

	m_CenterY -= GetPistonTopHeight();
	m_CenterY -= GetJambTopHeight();
	if (m_DispTrimAboveExtraRows)
		m_CenterY -= 8;

}

const GOGUIDisplayMetrics::MANUAL_RENDER_INFO& GOGUIDisplayMetrics::GetManualRenderInfo(const unsigned manual_nb) const
{

	assert(manual_nb < m_manual_info.size());
	assert(manual_nb >= m_first_manual);
	return m_manual_info[manual_nb].render_info;

}

wxBrush GOGUIDisplayMetrics::GetPedalBrush()
{
	wxBrush brush;
	brush.SetColour(0xA0, 0x80, 0x40);
	return brush;
}
