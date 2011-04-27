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

#include "GOrgueDisplayMetrics.h"
#include "GrandOrgueFile.h"
#include "IniFileConfig.h"

extern GrandOrgueFile* organfile;

#define DISPLAY_METRICS_GROUP "Organ"

GOrgueDisplayMetrics::GOrgueDisplayMetrics(IniFileConfig& ini) :
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
    m_DispScreenSizeHoriz = ini.ReadSize(DISPLAY_METRICS_GROUP, "DispScreenSizeHoriz", 0);
    m_DispScreenSizeVert = ini.ReadSize(DISPLAY_METRICS_GROUP, "DispScreenSizeVert", 1);
    m_DispDrawstopBackgroundImageNum = ini.ReadInteger(DISPLAY_METRICS_GROUP, "DispDrawstopBackgroundImageNum", 1, 64);
    m_DispConsoleBackgroundImageNum = ini.ReadInteger(DISPLAY_METRICS_GROUP, "DispConsoleBackgroundImageNum", 1, 64);
    m_DispKeyHorizBackgroundImageNum = ini.ReadInteger(DISPLAY_METRICS_GROUP, "DispKeyHorizBackgroundImageNum", 1, 64);
    m_DispKeyVertBackgroundImageNum = ini.ReadInteger(DISPLAY_METRICS_GROUP, "DispKeyVertBackgroundImageNum", 1, 64);
    m_DispDrawstopInsetBackgroundImageNum = ini.ReadInteger(DISPLAY_METRICS_GROUP, "DispDrawstopInsetBackgroundImageNum", 1, 64);
    m_DispControlLabelFont = ini.ReadString(DISPLAY_METRICS_GROUP, "DispControlLabelFont");
    m_DispShortcutKeyLabelFont = ini.ReadString(DISPLAY_METRICS_GROUP, "DispShortcutKeyLabelFont");
    m_DispShortcutKeyLabelColour = ini.ReadColor(DISPLAY_METRICS_GROUP, "DispShortcutKeyLabelColour");
    m_DispGroupLabelFont = ini.ReadString(DISPLAY_METRICS_GROUP, "DispGroupLabelFont");
    m_DispDrawstopCols = ini.ReadInteger(DISPLAY_METRICS_GROUP, "DispDrawstopCols", 2, 12);
    m_DispDrawstopRows = ini.ReadInteger(DISPLAY_METRICS_GROUP, "DispDrawstopRows", 1, 20);
    m_DispDrawstopColsOffset = ini.ReadBoolean(DISPLAY_METRICS_GROUP, "DispDrawstopColsOffset");
    m_DispDrawstopOuterColOffsetUp = ini.ReadBoolean(DISPLAY_METRICS_GROUP, "DispDrawstopOuterColOffsetUp", m_DispDrawstopColsOffset);
    m_DispPairDrawstopCols = ini.ReadBoolean(DISPLAY_METRICS_GROUP, "DispPairDrawstopCols");
    m_DispExtraDrawstopRows = ini.ReadInteger(DISPLAY_METRICS_GROUP, "DispExtraDrawstopRows", 0, 8);
    m_DispExtraDrawstopCols = ini.ReadInteger(DISPLAY_METRICS_GROUP, "DispExtraDrawstopCols", 0, 40);
    m_DispButtonCols = ini.ReadInteger(DISPLAY_METRICS_GROUP, "DispButtonCols", 1, 32);
    m_DispExtraButtonRows = ini.ReadInteger(DISPLAY_METRICS_GROUP, "DispExtraButtonRows", 0, 8);
    m_DispExtraPedalButtonRow = ini.ReadBoolean(DISPLAY_METRICS_GROUP, "DispExtraPedalButtonRow");
    m_DispExtraPedalButtonRowOffset = ini.ReadBoolean(DISPLAY_METRICS_GROUP, "DispExtraPedalButtonRowOffset", m_DispExtraPedalButtonRow);
    m_DispExtraPedalButtonRowOffsetRight = ini.ReadBoolean(DISPLAY_METRICS_GROUP, "DispExtraPedalButtonRowOffsetRight", m_DispExtraPedalButtonRow);
    m_DispButtonsAboveManuals = ini.ReadBoolean(DISPLAY_METRICS_GROUP, "DispButtonsAboveManuals");
    m_DispTrimAboveManuals = ini.ReadBoolean(DISPLAY_METRICS_GROUP, "DispTrimAboveManuals");
    m_DispTrimBelowManuals = ini.ReadBoolean(DISPLAY_METRICS_GROUP, "DispTrimBelowManuals");
    m_DispTrimAboveExtraRows = ini.ReadBoolean(DISPLAY_METRICS_GROUP, "DispTrimAboveExtraRows");
    m_DispExtraDrawstopRowsAboveExtraButtonRows = ini.ReadBoolean(DISPLAY_METRICS_GROUP, "DispExtraDrawstopRowsAboveExtraButtonRows");
}

/*
 * BASIC EXPORTS STRAIGHT FROM ODF
 */

int GOrgueDisplayMetrics::NumberOfExtraDrawstopRowsToDisplay()
{
	return m_DispExtraDrawstopRows;
}

int GOrgueDisplayMetrics::NumberOfExtraDrawstopColsToDisplay()
{
	return m_DispExtraDrawstopCols;
}

int GOrgueDisplayMetrics::NumberOfDrawstopColsToDisplay()
{
	return m_DispDrawstopCols;
}

int GOrgueDisplayMetrics::NumberOfButtonCols()
{
	return m_DispButtonCols;
}

int GOrgueDisplayMetrics::NumberOfExtraButtonRows()
{
	return m_DispExtraButtonRows;
}

int GOrgueDisplayMetrics::GetDrawstopBackgroundImageNum()
{
	return m_DispDrawstopBackgroundImageNum;
}

int GOrgueDisplayMetrics::GetConsoleBackgroundImageNum()
{
	return m_DispConsoleBackgroundImageNum;
}

int GOrgueDisplayMetrics::GetDrawstopInsetBackgroundImageNum()
{
	return m_DispDrawstopInsetBackgroundImageNum;
}

int GOrgueDisplayMetrics::GetKeyVertBackgroundImageNum()
{
	return m_DispKeyVertBackgroundImageNum;
}

int GOrgueDisplayMetrics::GetKeyHorizBackgroundImageNum()
{
	return m_DispKeyHorizBackgroundImageNum;
}

bool GOrgueDisplayMetrics::HasPairDrawstopCols()
{
	return m_DispPairDrawstopCols;
}

bool GOrgueDisplayMetrics::HasTrimAboveExtraRows()
{
	return m_DispTrimAboveExtraRows;
}

bool GOrgueDisplayMetrics::HasExtraPedalButtonRow()
{
	return m_DispExtraPedalButtonRow;
}

wxFont GOrgueDisplayMetrics::GetControlLabelFont()
{
	wxFont font = *wxNORMAL_FONT;
	font.SetFaceName(m_DispControlLabelFont);
	return font;
}

wxFont GOrgueDisplayMetrics::GetGroupLabelFont()
{
	wxFont font = *wxNORMAL_FONT;
#ifdef linux
	font.SetFamily(wxFONTFAMILY_SWISS);
#endif
	font.SetFaceName(m_DispGroupLabelFont);
	return font;
}

int GOrgueDisplayMetrics::GetScreenWidth()
{
	return m_DispScreenSizeHoriz;
}

int GOrgueDisplayMetrics::GetScreenHeight()
{
	return m_DispScreenSizeVert;
}

/*
 * COMPUTED VALUES FROM ODF PARAMETERS
 */

int GOrgueDisplayMetrics::GetJambLeftRightHeight()
{
	return (m_DispDrawstopRows + 1) * 69;
}

int GOrgueDisplayMetrics::GetJambLeftRightY()
{
	return (m_DispScreenSizeVert - GetJambLeftRightHeight() - (m_DispDrawstopColsOffset ? 35 : 0)) >> 1;
}

int GOrgueDisplayMetrics::GetJambLeftRightWidth()
{
	int jamblrw = m_DispDrawstopCols * 39;
	if (m_DispPairDrawstopCols)
		jamblrw += ((m_DispDrawstopCols >> 2) * 18) - 8;
	return jamblrw;
}

int GOrgueDisplayMetrics::GetJambTopHeight()
{
	return m_DispExtraDrawstopRows * 69;
}

int GOrgueDisplayMetrics::GetJambTopWidth()
{
	return m_DispExtraDrawstopCols * 78;
}

int GOrgueDisplayMetrics::GetJambTopX()
{
	return (m_DispScreenSizeHoriz - GetJambTopWidth()) >> 1;
}

int GOrgueDisplayMetrics::GetPistonTopHeight()
{
	return m_DispExtraButtonRows * 40;
}

int GOrgueDisplayMetrics::GetPistonWidth()
{
	return m_DispButtonCols * 44;
}

int GOrgueDisplayMetrics::GetPistonX()
{
	return (m_DispScreenSizeHoriz - GetPistonWidth()) >> 1;
}

/*
 * COMPUTED VALUES FROM ODF PARAMETERS & SCREEN
 */

int GOrgueDisplayMetrics::GetCenterWidth()
{
	return m_CenterWidth;
}

int GOrgueDisplayMetrics::GetCenterX()
{
	return (m_DispScreenSizeHoriz - m_CenterWidth) >> 1;
}

int GOrgueDisplayMetrics::GetCenterY()
{
	return m_CenterY;
}

int GOrgueDisplayMetrics::GetEnclosureWidth()
{
	return 52 * organfile->GetEnclosureCount();
}

int GOrgueDisplayMetrics::GetEnclosureY()
{
	return m_EnclosureY;
}

int GOrgueDisplayMetrics::GetJambLeftX()
{
	int jamblx = (GetCenterX() - GetJambLeftRightWidth()) >> 1;
	if (m_DispPairDrawstopCols)
		jamblx += 5;
	return jamblx;
}

int GOrgueDisplayMetrics::GetJambRightX()
{
	int jambrx = GetJambLeftX() + GetCenterX() + m_CenterWidth;
	if (m_DispPairDrawstopCols)
		jambrx += 5;
	return jambrx;
}

int GOrgueDisplayMetrics::GetJambTopDrawstop()
{
	if (m_DispTrimAboveExtraRows)
		return m_CenterY + 8;
	return m_CenterY;
}

int GOrgueDisplayMetrics::GetJambTopPiston()
{
	if (m_DispTrimAboveExtraRows)
		return m_CenterY + 8;
	return m_CenterY;
}

int GOrgueDisplayMetrics::GetJambTopY()
{
	if (m_DispTrimAboveExtraRows)
		return m_CenterY + 8;
	return m_CenterY;
}

/*
 * BLIT POSITION FUNCTIONS
 */

void GOrgueDisplayMetrics::GetDrawstopBlitPosition(const int drawstopRow, const int drawstopCol, int* blitX, int* blitY)
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

void GOrgueDisplayMetrics::GetPushbuttonBlitPosition(const int buttonRow, const int buttonCol, int* blitX, int* blitY)
{
	int i;
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
		i = buttonRow;
		if (i == 99)
			i = 0;

		if (i > organfile->GetManualAndPedalCount())
			*blitY = m_HackY - (i - organfile->GetManualAndPedalCount()) * 72 + 32 + 5;
		else
			*blitY = organfile->GetManual(i)->m_PistonY + 5;

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

void GOrgueDisplayMetrics::Update()
{

	m_CenterY = m_DispScreenSizeVert - 40;
	m_CenterWidth = std::max(GetJambTopWidth(), GetPistonWidth());

	for (int i = 0; i <= organfile->GetManualAndPedalCount(); i++)
	{

		if (!i)
		{
			organfile->GetManual(0)->m_Height = 40;
			organfile->GetManual(0)->m_KeysY = organfile->GetManual(0)->m_Y = m_CenterY;
			m_CenterY -= 40;
			if (m_DispExtraPedalButtonRow)
				m_CenterY -= 40;
			organfile->GetManual(0)->m_PistonY = m_CenterY;
			m_CenterY -= 87;	// enclosure area
			m_CenterWidth = std::max(m_CenterWidth, GetEnclosureWidth());
			m_EnclosureY = m_CenterY + 12;
		}

		if (!organfile->GetManual(i)->Displayed || i < organfile->GetFirstManualIndex())
			continue;

		if (i)
		{
			if (!m_DispButtonsAboveManuals)
			{
				m_CenterY -= 40;
				organfile->GetManual(i)->m_PistonY = m_CenterY;
			}
			organfile->GetManual(i)->m_Height = 32;
			if (m_DispTrimBelowManuals && i == 1)
			{
				organfile->GetManual(i)->m_Height += 8;
				m_CenterY -= 8;
			}
			m_CenterY -= 32;
			organfile->GetManual(i)->m_KeysY = m_CenterY;
			if (m_DispTrimAboveManuals && i == organfile->GetManualAndPedalCount())
			{
				organfile->GetManual(i)->m_Height += 8;
				m_CenterY -= 8;
			}
			if (m_DispButtonsAboveManuals)
			{
				m_CenterY -= 40;
				organfile->GetManual(i)->m_PistonY = m_CenterY;
			}
			organfile->GetManual(i)->m_Y = m_CenterY;
		}
		organfile->GetManual(i)->m_Width = 1;
		if (i)
		{
			for (int j = 0; j < organfile->GetManual(i)->GetNumberOfAccessibleKeys(); j++)
			{
				int k = (organfile->GetManual(i)->GetFirstAccessibleKeyMIDINoteNumber() + j) % 12;
				if ((k < 5 && !(k & 1)) || (k >= 5 && (k & 1)))
					organfile->GetManual(i)->m_Width += 12;
			}
		}
		else
		{
			for (int j = 0; j < organfile->GetManual(i)->GetNumberOfAccessibleKeys(); j++)
			{
				organfile->GetManual(i)->m_Width += 7;
				int k = (organfile->GetManual(i)->GetFirstAccessibleKeyMIDINoteNumber() + j) % 12;
				if (j && (!k || k == 5))
					organfile->GetManual(i)->m_Width += 7;
			}
		}
		organfile->GetManual(i)->m_X = (m_DispScreenSizeHoriz - organfile->GetManual(i)->m_Width) >> 1;
		organfile->GetManual(i)->m_Width += 16;
		if (organfile->GetManual(i)->m_Width > m_CenterWidth)
			m_CenterWidth = organfile->GetManual(i)->m_Width;
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


	/* FIXME: what is this below to-do supposed to do? do we need to do it? (nappleton) */
	/* TODO: trim should be displayed, blah */

}
