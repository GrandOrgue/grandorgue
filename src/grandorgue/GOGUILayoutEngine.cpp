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

#include "GOGUILayoutEngine.h"

#include "GOGUIDisplayMetrics.h"
#include "GOGUIEnclosure.h"
#include "GOGUIManual.h"
#include <wx/intl.h>

GOGUILayoutEngine::GOGUILayoutEngine(GOGUIDisplayMetrics& metrics) :
	m_metrics(metrics),
	m_ManualRenderInfo(),
	m_Enclosures(),
	m_Manuals(),
	m_HackY(0),
	m_EnclosureY(0),
	m_CenterY(0),
	m_CenterWidth(0)
{
}

GOGUILayoutEngine::~GOGUILayoutEngine()
{
}

/*
 * COMPUTED VALUES FROM ODF PARAMETERS
 */

unsigned GOGUILayoutEngine::GetJambLeftRightHeight()
{
	return (m_metrics.NumberOfDrawstopRowsToDisplay() + 1) * m_metrics.GetDrawstopHeight();
}

int GOGUILayoutEngine::GetJambLeftRightY()
{
	return ((int)(m_metrics.GetScreenHeight() - GetJambLeftRightHeight() - (m_metrics.HasDrawstopColsOffset() ? (m_metrics.GetDrawstopHeight()/2) : 0))) / 2;
}

int GOGUILayoutEngine::GetJambLeftRightWidth()
{
	int jamblrw = m_metrics.NumberOfDrawstopColsToDisplay() * m_metrics.GetDrawstopWidth() / 2;
	if (m_metrics.HasPairDrawstopCols())
		jamblrw += ((m_metrics.NumberOfDrawstopColsToDisplay() >> 2) * (m_metrics.GetDrawstopWidth() / 4)) - 8;
	return jamblrw;
}

unsigned GOGUILayoutEngine::GetJambTopHeight()
{
	return m_metrics.NumberOfExtraDrawstopRowsToDisplay() * m_metrics.GetDrawstopHeight();
}

unsigned GOGUILayoutEngine::GetJambTopWidth()
{
	return m_metrics.NumberOfExtraDrawstopColsToDisplay() * m_metrics.GetDrawstopWidth();
}

int GOGUILayoutEngine::GetJambTopX()
{
	return (m_metrics.GetScreenWidth() - GetJambTopWidth()) >> 1;
}

unsigned GOGUILayoutEngine::GetPistonTopHeight()
{
	return m_metrics.NumberOfExtraButtonRows() * m_metrics.GetButtonHeight();
}

unsigned GOGUILayoutEngine::GetPistonWidth()
{
	return m_metrics.NumberOfButtonCols() * m_metrics.GetButtonWidth();
}

int GOGUILayoutEngine::GetPistonX()
{
	return (m_metrics.GetScreenWidth() - GetPistonWidth()) >> 1;
}

/*
 * COMPUTED VALUES FROM ODF PARAMETERS & SCREEN
 */

int GOGUILayoutEngine::GetCenterWidth()
{
	return m_CenterWidth;
}

int GOGUILayoutEngine::GetCenterX()
{
	return (m_metrics.GetScreenWidth() - GetCenterWidth()) >> 1;
}

int GOGUILayoutEngine::GetCenterY()
{
	return m_CenterY;
}

int GOGUILayoutEngine::GetHackY()
{
	return m_HackY;
}

unsigned GOGUILayoutEngine::GetEnclosuresWidth()
{
	return m_metrics.GetEnclosureWidth() * m_Enclosures.size();
}

int GOGUILayoutEngine::GetEnclosureY()
{
	return m_EnclosureY;
}

int GOGUILayoutEngine::GetEnclosureX(const GOGUIEnclosure* enclosure)
{

	assert(enclosure);

	int enclosure_x = (m_metrics.GetScreenWidth() - GetEnclosuresWidth() + 6) >> 1;
	for (unsigned int i = 0; i < m_Enclosures.size(); i++)
	{
		if (enclosure == m_Enclosures[i])
			return enclosure_x;
		enclosure_x += m_metrics.GetEnclosureWidth();
	}

	throw (wxString)_("enclosure not found");

}

int GOGUILayoutEngine::GetJambLeftX()
{
	int jamblx = (GetCenterX() - GetJambLeftRightWidth()) >> 1;
	if (m_metrics.HasPairDrawstopCols())
		jamblx += 5;
	return jamblx;
}

int GOGUILayoutEngine::GetJambRightX()
{
	int jambrx = GetJambLeftX() + GetCenterX() + GetCenterWidth();
	if (m_metrics.HasPairDrawstopCols())
		jambrx += 5;
	return jambrx;
}

int GOGUILayoutEngine::GetJambTopDrawstop()
{
	if (m_metrics.HasTrimAboveExtraRows())
		return GetCenterY() + 8;
	return GetCenterY();
}

int GOGUILayoutEngine::GetJambTopPiston()
{
	if (m_metrics.HasTrimAboveExtraRows())
		return GetCenterY() + 8;
	return GetCenterY();
}

int GOGUILayoutEngine::GetJambTopY()
{
	if (m_metrics.HasTrimAboveExtraRows())
		return GetCenterY() + 8;
	return GetCenterY();
}

/*
 * BLIT POSITION FUNCTIONS
 */

void GOGUILayoutEngine::GetDrawstopBlitPosition(const int drawstopRow, const int drawstopCol, int& blitX, int& blitY)
{
	int i;
	if (drawstopRow > 99)
	{
		blitX = GetJambTopX() + (drawstopCol - 1) * m_metrics.GetDrawstopWidth() + 6;
		if (m_metrics.HasExtraDrawstopRowsAboveExtraButtonRows())
		{
			blitY = GetJambTopDrawstop() + (drawstopRow - 100) * m_metrics.GetDrawstopHeight() + 2;
		}
		else
		{
			blitY = GetJambTopDrawstop() + (drawstopRow - 100) * m_metrics.GetDrawstopHeight() + (m_metrics.NumberOfExtraButtonRows() * m_metrics.GetButtonHeight()) + 2;
		}
	}
	else
	{
		i = m_metrics.NumberOfDrawstopColsToDisplay() >> 1;
		if (drawstopCol <= i)
		{
			blitX = GetJambLeftX() + (drawstopCol - 1) * m_metrics.GetDrawstopWidth() + 6;
			blitY = GetJambLeftRightY() + (drawstopRow - 1) * m_metrics.GetDrawstopHeight() + 32;
		}
		else
		{
			blitX = GetJambRightX() + (drawstopCol - 1 - i) * m_metrics.GetDrawstopWidth() + 6;
			blitY = GetJambLeftRightY() + (drawstopRow - 1) * m_metrics.GetDrawstopHeight() + 32;
		}
		if (m_metrics.HasPairDrawstopCols())
			blitX += (((drawstopCol - 1) % i) >> 1) * (m_metrics.GetDrawstopWidth() / 4);

		if (drawstopCol <= i)
			i = drawstopCol;
		else
			i = m_metrics.NumberOfDrawstopColsToDisplay() - drawstopCol + 1;
		if (m_metrics.HasDrawstopColsOffset() && (i & 1) ^ m_metrics.HasDrawstopOuterColOffsetUp())
			blitY += m_metrics.GetDrawstopHeight() / 2;

	}
}

void GOGUILayoutEngine::GetPushbuttonBlitPosition(const int buttonRow, const int buttonCol, int& blitX, int& blitY)
{

	blitX = GetPistonX() + (buttonCol - 1) * m_metrics.GetButtonWidth() + 6;
	if (buttonRow > 99)
	{
		if (m_metrics.HasExtraDrawstopRowsAboveExtraButtonRows())
		{
			blitY = GetJambTopPiston() + (buttonRow - 100) * m_metrics.GetButtonHeight() + (m_metrics.NumberOfExtraDrawstopRowsToDisplay() * m_metrics.GetDrawstopHeight()) + 5;
		}
		else
		{
			blitY = GetJambTopPiston() + (buttonRow - 100) * m_metrics.GetButtonHeight() + 5;
		}
	}
	else
	{
		int i = buttonRow;
		if (i == 99)
			i = 0;

		if (i >= (int)m_Manuals.size())
			blitY = GetHackY() - (i + 1 - (int)m_Manuals.size()) * (m_metrics.GetManualHeight() + m_metrics.GetButtonHeight()) + m_metrics.GetManualHeight() + 5;
		else
			blitY = GetManualRenderInfo(i).piston_y + 5;

		if (m_metrics.HasExtraPedalButtonRow() && !buttonRow)
			blitY += m_metrics.GetButtonHeight();
		if (m_metrics.HasExtraPedalButtonRowOffset() && buttonRow == 99)
			blitX -= m_metrics.GetButtonWidth() / 2 + 2;
	}

}

/*
 * UPDATE METHOD - updates parameters dependent on ODF parameters and
 * screen stuff.
 */

/* TODO: this method could do with a cleanup */

void GOGUILayoutEngine::Update()
{
	if (!m_Manuals.size())
		m_Manuals.push_back(NULL);

	m_ManualRenderInfo.resize(m_Manuals.size());
	m_CenterY = m_metrics.GetScreenHeight() - m_metrics.GetPedalHeight();
	m_CenterWidth = std::max(GetJambTopWidth(), GetPistonWidth());

	for (unsigned i = 0; i < m_Manuals.size(); i++)
	{

		if (!i && m_Manuals[i])
		{
			m_ManualRenderInfo[0].height = m_metrics.GetPedalHeight();
			m_ManualRenderInfo[0].keys_y = m_ManualRenderInfo[0].y = m_CenterY;
			m_CenterY -= m_metrics.GetPedalHeight();
			if (m_metrics.HasExtraPedalButtonRow())
				m_CenterY -= m_metrics.GetButtonHeight();
			m_ManualRenderInfo[0].piston_y = m_CenterY;
			m_CenterWidth = std::max(m_CenterWidth, (int)GetEnclosuresWidth());
			m_CenterY -= 12;
			m_CenterY -= m_metrics.GetEnclosureHeight();
			m_EnclosureY = m_CenterY;
			m_CenterY -= 12;
		}
		if (!i && !m_Manuals[i] && m_Enclosures.size())
		{
			m_CenterY -= 12;
			m_CenterY -= m_metrics.GetEnclosureHeight();
			m_EnclosureY = m_CenterY;
			m_CenterY -= 12;
		}

		if (!m_Manuals[i])
			continue;

		if (i)
		{
			if (!m_metrics.HasButtonsAboveManuals())
			{
				m_CenterY -= m_metrics.GetButtonHeight();
				m_ManualRenderInfo[i].piston_y = m_CenterY;
			}
			m_ManualRenderInfo[i].height = m_metrics.GetManualHeight();
			if (m_metrics.HasTrimBelowManuals() && i == 1)
			{
				m_ManualRenderInfo[i].height += 8;
				m_CenterY -= 8;
			}
			m_CenterY -= m_metrics.GetManualHeight();
			m_ManualRenderInfo[i].keys_y = m_CenterY;
			if (m_metrics.HasTrimAboveManuals() && i + 1 == m_Manuals.size())
			{
				m_CenterY -= 8;
				m_ManualRenderInfo[i].height += 8;
			}
			if (m_metrics.HasButtonsAboveManuals())
			{
				m_CenterY -= m_metrics.GetButtonHeight();
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
					m_ManualRenderInfo[i].width += m_metrics.GetManualKeyWidth();
			}
		}
		else
		{
			for (unsigned j = 0; j < m_Manuals[i]->GetKeyCount(); j++)
			{
				m_ManualRenderInfo[i].width += m_metrics.GetPedalKeyWidth();
				if (j && !m_Manuals[i]->IsSharp(j - 1) && !m_Manuals[i]->IsSharp(j))
					m_ManualRenderInfo[i].width += m_metrics.GetPedalKeyWidth();
			}
		}
		m_ManualRenderInfo[i].x = (m_metrics.GetScreenWidth() - m_ManualRenderInfo[i].width) >> 1;
		m_ManualRenderInfo[i].width += 16;
		if ((int)m_ManualRenderInfo[i].width > m_CenterWidth)
			m_CenterWidth = m_ManualRenderInfo[i].width;
	}

	m_HackY = m_CenterY;

	if (m_CenterWidth + GetJambLeftRightWidth() * 2 < (int)m_metrics.GetScreenWidth())
		m_CenterWidth += (m_metrics.GetScreenWidth() - m_CenterWidth - GetJambLeftRightWidth() * 2) / 3;

	m_CenterY -= GetPistonTopHeight();
	m_CenterY -= GetJambTopHeight();
	if (m_metrics.HasTrimAboveExtraRows())
		m_CenterY -= 8;
}

const GOGUILayoutEngine::MANUAL_RENDER_INFO& GOGUILayoutEngine::GetManualRenderInfo(const unsigned manual_nb) const
{
	assert(manual_nb < m_ManualRenderInfo.size());
	return m_ManualRenderInfo[manual_nb];
}

void GOGUILayoutEngine::RegisterEnclosure(GOGUIEnclosure* enclosure)
{
	m_Enclosures.push_back(enclosure);
}

void GOGUILayoutEngine::RegisterManual(GOGUIManual* manual)
{
	m_Manuals.push_back(manual);
}

unsigned GOGUILayoutEngine::GetManualNumber()
{
	return m_Manuals.size();
}
