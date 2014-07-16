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

#include "GOGUIDisplayMetrics.h"

#include "GOGUIEnclosure.h"
#include "GOGUIManual.h"
#include "GOrgueManual.h"
#include "GrandOrgueFile.h"
#include <wx/font.h>
#include <wx/intl.h>

GOGUIDisplayMetrics::GOGUIDisplayMetrics(GrandOrgueFile* organfile, wxString group) :
	m_group(group),
	m_organfile(organfile),
	m_ManualRenderInfo(),
	m_Enclosures(),
	m_Manuals(),
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
	m_DrawStopWidth(78),
	m_DrawStopHeight(69),
	m_ButtonWidth(44),
	m_ButtonHeight(40),
	m_EnclosureWidth(52),
	m_EnclosureHeight(63),
	m_PedalHeight(40),
	m_PedalKeyWidth(7),
	m_ManualHeight(32),
	m_ManualKeyWidth(12),
	m_HackY(0),
	m_EnclosureY(0),
	m_CenterY(0),
	m_CenterWidth(0)
{
}

GOGUIDisplayMetrics::~GOGUIDisplayMetrics()
{
}

void GOGUIDisplayMetrics::Init()
{
	m_ControlLabelFont.SetName(m_DispControlLabelFont);
	m_GroupLabelFont.SetName(m_DispGroupLabelFont);
}

unsigned GOGUIDisplayMetrics::GetDrawstopWidth()
{
	return m_DrawStopWidth;
}

unsigned GOGUIDisplayMetrics::GetDrawstopHeight()
{
	return m_DrawStopHeight;
}

unsigned GOGUIDisplayMetrics::GetButtonWidth()
{
	return m_ButtonWidth;
}

unsigned GOGUIDisplayMetrics::GetButtonHeight()
{
	return m_ButtonHeight;
}

unsigned GOGUIDisplayMetrics::GetEnclosureWidth()
{
	return m_EnclosureWidth;
}

unsigned GOGUIDisplayMetrics::GetEnclosureHeight()
{
	return m_EnclosureHeight;
}

unsigned GOGUIDisplayMetrics::GetManualKeyWidth()
{
	return m_ManualKeyWidth;
}

unsigned GOGUIDisplayMetrics::GetManualHeight()
{
	return m_ManualHeight;
}

unsigned GOGUIDisplayMetrics::GetPedalKeyWidth()
{
	return m_PedalKeyWidth;
}

unsigned GOGUIDisplayMetrics::GetPedalHeight()
{
	return m_PedalHeight;
}

unsigned GOGUIDisplayMetrics::NumberOfExtraDrawstopRowsToDisplay()
{
	return m_DispExtraDrawstopRows;
}

unsigned GOGUIDisplayMetrics::NumberOfExtraDrawstopColsToDisplay()
{
	return m_DispExtraDrawstopCols;
}

unsigned GOGUIDisplayMetrics::NumberOfDrawstopRowsToDisplay()
{
	return m_DispDrawstopRows;
}

unsigned GOGUIDisplayMetrics::NumberOfDrawstopColsToDisplay()
{
	return m_DispDrawstopCols;
}

unsigned GOGUIDisplayMetrics::NumberOfButtonCols()
{
	return m_DispButtonCols;
}

unsigned GOGUIDisplayMetrics::NumberOfExtraButtonRows()
{
	return m_DispExtraButtonRows;
}

unsigned GOGUIDisplayMetrics::GetDrawstopBackgroundImageNum()
{
	return m_DispDrawstopBackgroundImageNum;
}

unsigned GOGUIDisplayMetrics::GetConsoleBackgroundImageNum()
{
	return m_DispConsoleBackgroundImageNum;
}

unsigned GOGUIDisplayMetrics::GetDrawstopInsetBackgroundImageNum()
{
	return m_DispDrawstopInsetBackgroundImageNum;
}

unsigned GOGUIDisplayMetrics::GetKeyVertBackgroundImageNum()
{
	return m_DispKeyVertBackgroundImageNum;
}

unsigned GOGUIDisplayMetrics::GetKeyHorizBackgroundImageNum()
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

bool GOGUIDisplayMetrics::HasTrimAboveManuals()
{
	return m_DispTrimAboveManuals;
}

bool GOGUIDisplayMetrics::HasTrimBelowManuals()
{
	return m_DispTrimBelowManuals;
}

bool GOGUIDisplayMetrics::HasExtraPedalButtonRow()
{
	return m_DispExtraPedalButtonRow;
}

bool GOGUIDisplayMetrics::HasButtonsAboveManuals()
{
	return m_DispButtonsAboveManuals;
}

bool GOGUIDisplayMetrics::HasExtraPedalButtonRowOffset()
{
	return m_DispExtraPedalButtonRowOffset;
}

bool GOGUIDisplayMetrics::HasExtraDrawstopRowsAboveExtraButtonRows()
{
	return m_DispExtraDrawstopRowsAboveExtraButtonRows;
}

bool GOGUIDisplayMetrics::HasDrawstopColsOffset()
{
	return m_DispDrawstopColsOffset;
}

bool GOGUIDisplayMetrics::HasDrawstopOuterColOffsetUp()
{
	return m_DispDrawstopOuterColOffsetUp;
}

GOrgueFont GOGUIDisplayMetrics::GetControlLabelFont()
{
	return m_ControlLabelFont;
}

GOrgueFont GOGUIDisplayMetrics::GetGroupLabelFont()
{
	return m_GroupLabelFont;
}

unsigned GOGUIDisplayMetrics::GetScreenWidth()
{
	return m_DispScreenSizeHoriz;
}

unsigned GOGUIDisplayMetrics::GetScreenHeight()
{
	return m_DispScreenSizeVert;
}

/*
 * COMPUTED VALUES FROM ODF PARAMETERS
 */

unsigned GOGUIDisplayMetrics::GetJambLeftRightHeight()
{
	return (NumberOfDrawstopRowsToDisplay() + 1) * GetDrawstopHeight();
}

int GOGUIDisplayMetrics::GetJambLeftRightY()
{
	return ((int)(GetScreenHeight() - GetJambLeftRightHeight() - (HasDrawstopColsOffset() ? (GetDrawstopHeight()/2) : 0))) / 2;
}

int GOGUIDisplayMetrics::GetJambLeftRightWidth()
{
	int jamblrw = NumberOfDrawstopColsToDisplay() * GetDrawstopWidth() / 2;
	if (HasPairDrawstopCols())
		jamblrw += ((NumberOfDrawstopColsToDisplay() >> 2) * (GetDrawstopWidth() / 4)) - 8;
	return jamblrw;
}

unsigned GOGUIDisplayMetrics::GetJambTopHeight()
{
	return NumberOfExtraDrawstopRowsToDisplay() * GetDrawstopHeight();
}

unsigned GOGUIDisplayMetrics::GetJambTopWidth()
{
	return NumberOfExtraDrawstopColsToDisplay() * GetDrawstopWidth();
}

int GOGUIDisplayMetrics::GetJambTopX()
{
	return (GetScreenWidth() - GetJambTopWidth()) >> 1;
}

unsigned GOGUIDisplayMetrics::GetPistonTopHeight()
{
	return NumberOfExtraButtonRows() * GetButtonHeight();
}

unsigned GOGUIDisplayMetrics::GetPistonWidth()
{
	return NumberOfButtonCols() * GetButtonWidth();
}

int GOGUIDisplayMetrics::GetPistonX()
{
	return (GetScreenWidth() - GetPistonWidth()) >> 1;
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
	return (GetScreenWidth() - GetCenterWidth()) >> 1;
}

int GOGUIDisplayMetrics::GetCenterY()
{
	return m_CenterY;
}

int GOGUIDisplayMetrics::GetHackY()
{
	return m_HackY;
}

unsigned GOGUIDisplayMetrics::GetEnclosuresWidth()
{
	return GetEnclosureWidth() * m_Enclosures.size();
}

int GOGUIDisplayMetrics::GetEnclosureY()
{
	return m_EnclosureY;
}

int GOGUIDisplayMetrics::GetEnclosureX(const GOGUIEnclosure* enclosure)
{

	assert(enclosure);

	int enclosure_x = (GetScreenWidth() - GetEnclosuresWidth() + 6) >> 1;
	for (unsigned int i = 0; i < m_Enclosures.size(); i++)
	{
		if (enclosure == m_Enclosures[i])
			return enclosure_x;
		enclosure_x += GetEnclosureWidth();
	}

	throw (wxString)_("enclosure not found");

}

int GOGUIDisplayMetrics::GetJambLeftX()
{
	int jamblx = (GetCenterX() - GetJambLeftRightWidth()) >> 1;
	if (HasPairDrawstopCols())
		jamblx += 5;
	return jamblx;
}

int GOGUIDisplayMetrics::GetJambRightX()
{
	int jambrx = GetJambLeftX() + GetCenterX() + GetCenterWidth();
	if (HasPairDrawstopCols())
		jambrx += 5;
	return jambrx;
}

int GOGUIDisplayMetrics::GetJambTopDrawstop()
{
	if (HasTrimAboveExtraRows())
		return GetCenterY() + 8;
	return GetCenterY();
}

int GOGUIDisplayMetrics::GetJambTopPiston()
{
	if (HasTrimAboveExtraRows())
		return GetCenterY() + 8;
	return GetCenterY();
}

int GOGUIDisplayMetrics::GetJambTopY()
{
	if (HasTrimAboveExtraRows())
		return GetCenterY() + 8;
	return GetCenterY();
}

/*
 * BLIT POSITION FUNCTIONS
 */

void GOGUIDisplayMetrics::GetDrawstopBlitPosition(const int drawstopRow, const int drawstopCol, int& blitX, int& blitY)
{
	int i;
	if (drawstopRow > 99)
	{
		blitX = GetJambTopX() + (drawstopCol - 1) * GetDrawstopWidth() + 6;
		if (HasExtraDrawstopRowsAboveExtraButtonRows())
		{
			blitY = GetJambTopDrawstop() + (drawstopRow - 100) * GetDrawstopHeight() + 2;
		}
		else
		{
			blitY = GetJambTopDrawstop() + (drawstopRow - 100) * GetDrawstopHeight() + (NumberOfExtraButtonRows() * GetButtonHeight()) + 2;
		}
	}
	else
	{
		i = NumberOfDrawstopColsToDisplay() >> 1;
		if (drawstopCol <= i)
		{
			blitX = GetJambLeftX() + (drawstopCol - 1) * GetDrawstopWidth() + 6;
			blitY = GetJambLeftRightY() + (drawstopRow - 1) * GetDrawstopHeight() + 32;
		}
		else
		{
			blitX = GetJambRightX() + (drawstopCol - 1 - i) * GetDrawstopWidth() + 6;
			blitY = GetJambLeftRightY() + (drawstopRow - 1) * GetDrawstopHeight() + 32;
		}
		if (HasPairDrawstopCols())
			blitX += (((drawstopCol - 1) % i) >> 1) * (GetDrawstopWidth() / 4);

		if (drawstopCol <= i)
			i = drawstopCol;
		else
			i = NumberOfDrawstopColsToDisplay() - drawstopCol + 1;
		if (HasDrawstopColsOffset() && (i & 1) ^ HasDrawstopOuterColOffsetUp())
			blitY += GetDrawstopHeight() / 2;

	}
}

void GOGUIDisplayMetrics::GetPushbuttonBlitPosition(const int buttonRow, const int buttonCol, int& blitX, int& blitY)
{

	blitX = GetPistonX() + (buttonCol - 1) * GetButtonWidth() + 6;
	if (buttonRow > 99)
	{
		if (HasExtraDrawstopRowsAboveExtraButtonRows())
		{
			blitY = GetJambTopPiston() + (buttonRow - 100) * GetButtonHeight() + (NumberOfExtraDrawstopRowsToDisplay() * GetDrawstopHeight()) + 5;
		}
		else
		{
			blitY = GetJambTopPiston() + (buttonRow - 100) * GetButtonHeight() + 5;
		}
	}
	else
	{
		int i = buttonRow;
		if (i == 99)
			i = 0;

		if (i >= (int)m_Manuals.size())
			blitY = m_HackY - (i + 1 - (int)m_Manuals.size()) * (GetManualHeight() + GetButtonHeight()) + GetManualHeight() + 5;
		else
			blitY = GetManualRenderInfo(i).piston_y + 5;

		if (HasExtraPedalButtonRow() && !buttonRow)
			blitY += GetButtonHeight();
		if (HasExtraPedalButtonRowOffset() && buttonRow == 99)
			blitX -= GetButtonWidth() / 2 + 2;
	}

}

/*
 * UPDATE METHOD - updates parameters dependent on ODF parameters and
 * screen stuff.
 */

/* TODO: this method could do with a cleanup */

void GOGUIDisplayMetrics::Update()
{
	if (!m_Manuals.size())
		m_Manuals.push_back(NULL);

	m_ManualRenderInfo.resize(m_Manuals.size());
	m_CenterY = GetScreenHeight() - GetPedalHeight();
	m_CenterWidth = std::max(GetJambTopWidth(), GetPistonWidth());

	for (unsigned i = 0; i < m_Manuals.size(); i++)
	{

		if (!i && m_Manuals[i])
		{
			m_ManualRenderInfo[0].height = GetPedalHeight();
			m_ManualRenderInfo[0].keys_y = m_ManualRenderInfo[0].y = m_CenterY;
			m_CenterY -= GetPedalHeight();
			if (HasExtraPedalButtonRow())
				m_CenterY -= GetButtonHeight();
			m_ManualRenderInfo[0].piston_y = m_CenterY;
			m_CenterWidth = std::max(m_CenterWidth, (int)GetEnclosuresWidth());
			m_CenterY -= 12;
			m_CenterY -= GetEnclosureHeight();
			m_EnclosureY = m_CenterY;
			m_CenterY -= 12;
		}
		if (!i && !m_Manuals[i] && m_Enclosures.size())
		{
			m_CenterY -= 12;
			m_CenterY -= GetEnclosureHeight();
			m_EnclosureY = m_CenterY;
			m_CenterY -= 12;
		}

		if (!m_Manuals[i])
			continue;

		if (i)
		{
			if (!HasButtonsAboveManuals())
			{
				m_CenterY -= GetButtonHeight();
				m_ManualRenderInfo[i].piston_y = m_CenterY;
			}
			m_ManualRenderInfo[i].height = GetManualHeight();
			if (HasTrimBelowManuals() && i == 1)
			{
				m_ManualRenderInfo[i].height += 8;
				m_CenterY -= 8;
			}
			m_CenterY -= GetManualHeight();
			m_ManualRenderInfo[i].keys_y = m_CenterY;
			if (HasTrimAboveManuals() && i + 1 == m_Manuals.size())
			{
				m_CenterY -= 8;
				m_ManualRenderInfo[i].height += 8;
			}
			if (HasButtonsAboveManuals())
			{
				m_CenterY -= GetButtonHeight();
				m_ManualRenderInfo[i].piston_y = m_CenterY;
			}
			m_ManualRenderInfo[i].y = m_CenterY;
		}
		m_ManualRenderInfo[i].width = 1;
		if (i)
		{
			for (unsigned j = 0; j < m_Manuals[i]->GetKeyCount(); j++)
			{
				if (!m_Manuals[i]->IsSharp(j))
					m_ManualRenderInfo[i].width += GetManualKeyWidth();
			}
		}
		else
		{
			for (unsigned j = 0; j < m_Manuals[i]->GetKeyCount(); j++)
			{
				m_ManualRenderInfo[i].width += GetPedalKeyWidth();
				if (j && !m_Manuals[i]->IsSharp(j - 1) && !m_Manuals[i]->IsSharp(j))
					m_ManualRenderInfo[i].width += GetPedalKeyWidth();
			}
		}
		m_ManualRenderInfo[i].x = (GetScreenWidth() - m_ManualRenderInfo[i].width) >> 1;
		m_ManualRenderInfo[i].width += 16;
		if ((int)m_ManualRenderInfo[i].width > m_CenterWidth)
			m_CenterWidth = m_ManualRenderInfo[i].width;
	}

	m_HackY = m_CenterY;

	if (m_CenterWidth + GetJambLeftRightWidth() * 2 < (int)GetScreenWidth())
		m_CenterWidth += (GetScreenWidth() - m_CenterWidth - GetJambLeftRightWidth() * 2) / 3;

	m_CenterY -= GetPistonTopHeight();
	m_CenterY -= GetJambTopHeight();
	if (HasTrimAboveExtraRows())
		m_CenterY -= 8;
}

const GOGUIDisplayMetrics::MANUAL_RENDER_INFO& GOGUIDisplayMetrics::GetManualRenderInfo(const unsigned manual_nb) const
{
	assert(manual_nb < m_ManualRenderInfo.size());
	return m_ManualRenderInfo[manual_nb];
}

void GOGUIDisplayMetrics::RegisterEnclosure(GOGUIEnclosure* enclosure)
{
	m_Enclosures.push_back(enclosure);
}

void GOGUIDisplayMetrics::RegisterManual(GOGUIManual* manual)
{
	m_Manuals.push_back(manual);
}
