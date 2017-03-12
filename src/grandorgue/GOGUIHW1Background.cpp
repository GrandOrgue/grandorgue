/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2017 GrandOrgue contributors (see AUTHORS)
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

#include "GOGUIHW1Background.h"

#include "GOrgueDC.h"
#include "GOGUIDisplayMetrics.h"
#include "GOGUILayoutEngine.h"
#include "GOGUIPanel.h"

GOGUIHW1Background::GOGUIHW1Background(GOGUIPanel* panel):
	GOGUIControl(panel, NULL),
	m_Images()
{
}

void GOGUIHW1Background::Init(GOrgueConfigReader& cfg, wxString group)
{
	GOGUIControl::Init(cfg, group);
}

void GOGUIHW1Background::Layout()
{
	m_Images.clear();
	wxRect rect;

	rect = wxRect(0, 0, m_layout->GetCenterX(), m_metrics->GetScreenHeight());
	m_Images.push_back(GOBackgroundImage(rect, m_panel->GetWood(m_metrics->GetDrawstopBackgroundImageNum())));
	rect = wxRect(m_layout->GetCenterX() + m_layout->GetCenterWidth(), 0,
		      m_metrics->GetScreenWidth() - (m_layout->GetCenterX() + m_layout->GetCenterWidth()), m_metrics->GetScreenHeight());
	m_Images.push_back(GOBackgroundImage(rect, m_panel->GetWood(m_metrics->GetDrawstopBackgroundImageNum())));
	rect = wxRect(m_layout->GetCenterX(), 0, m_layout->GetCenterWidth(), m_metrics->GetScreenHeight());
	m_Images.push_back(GOBackgroundImage(rect, m_panel->GetWood(m_metrics->GetConsoleBackgroundImageNum())));

	if (m_metrics->HasPairDrawstopCols())
	{
		for (unsigned i = 0; i < (m_metrics->NumberOfDrawstopColsToDisplay() >> 2); i++)
		{
			rect = wxRect(i * (2 * m_metrics->GetDrawstopWidth() + 18) + m_layout->GetJambLeftX() - 5,
				      m_layout->GetJambLeftRightY(), 2 * m_metrics->GetDrawstopWidth() + 10, m_layout->GetJambLeftRightHeight());
			m_Images.push_back(GOBackgroundImage(rect, m_panel->GetWood(m_metrics->GetDrawstopInsetBackgroundImageNum())));
			rect = wxRect(i * (2 * m_metrics->GetDrawstopWidth() + 18) + m_layout->GetJambRightX() - 5,
				      m_layout->GetJambLeftRightY(), 2 * m_metrics->GetDrawstopWidth() + 10, m_layout->GetJambLeftRightHeight());
			m_Images.push_back(GOBackgroundImage(rect, m_panel->GetWood(m_metrics->GetDrawstopInsetBackgroundImageNum())));
		}
	}

	if (m_metrics->HasTrimAboveExtraRows())
	{
		rect = wxRect(m_layout->GetCenterX(), m_layout->GetCenterY(), m_layout->GetCenterWidth(), 8);
		m_Images.push_back(GOBackgroundImage(rect, m_panel->GetWood(m_metrics->GetKeyVertBackgroundImageNum())));
	}

	if (m_layout->GetJambTopHeight() + m_layout->GetPistonTopHeight())
	{
		rect = wxRect(m_layout->GetCenterX(), m_layout->GetJambTopY(), m_layout->GetCenterWidth(), m_layout->GetJambTopHeight() + m_layout->GetPistonTopHeight());
		m_Images.push_back(GOBackgroundImage(rect, m_panel->GetWood(m_metrics->GetKeyHorizBackgroundImageNum())));
	}
}

void GOGUIHW1Background::PrepareDraw(double scale, GOrgueBitmap* background)
{
	for(unsigned i = 0; i < m_Images.size(); i++)
		m_Images[i].bmp.PrepareTileBitmap(scale, m_Images[i].rect, 0, 0, background);
}

void GOGUIHW1Background::Draw(GOrgueDC& dc)
{
	for(unsigned i = 0; i < m_Images.size(); i++)
		dc.DrawBitmap(m_Images[i].bmp, m_Images[i].rect);
	GOGUIControl::Draw(dc);
}
