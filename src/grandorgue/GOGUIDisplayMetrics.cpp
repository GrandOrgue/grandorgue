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

#include <wx/font.h>
#include <wx/intl.h>

GOGUIDisplayMetrics::GOGUIDisplayMetrics() :
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
	m_ManualKeyWidth(12)
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
