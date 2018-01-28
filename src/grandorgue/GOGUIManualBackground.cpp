/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2018 GrandOrgue contributors (see AUTHORS)
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

#include "GOGUIManualBackground.h"

#include "GOGUIDisplayMetrics.h"
#include "GOGUILayoutEngine.h"
#include "GOGUIPanel.h"
#include "GOrgueConfigReader.h"
#include "GOrgueDC.h"
#include "GOrgueManual.h"

GOGUIManualBackground::GOGUIManualBackground(GOGUIPanel* panel, unsigned manual_number):
	GOGUIControl(panel, NULL),
	m_ManualNumber(manual_number)
{
}

void GOGUIManualBackground::Init(GOrgueConfigReader& cfg, wxString group)
{
	GOGUIControl::Init(cfg, group);
}

void GOGUIManualBackground::Load(GOrgueConfigReader& cfg, wxString group)
{
	GOGUIControl::Load(cfg, group);
}

void GOGUIManualBackground::Layout()
{
	const GOGUILayoutEngine::MANUAL_RENDER_INFO &mri = m_layout->GetManualRenderInfo(m_ManualNumber);
	m_BoundingRect = wxRect(mri.x, mri.y, mri.width, mri.height);
	m_VRect = wxRect(m_layout->GetCenterX(), mri.y, m_layout->GetCenterWidth(), mri.height);
	m_VBackground = m_panel->GetWood(m_metrics->GetKeyVertBackgroundImageNum());
	m_HRect = wxRect(m_layout->GetCenterX(), mri.piston_y, m_layout->GetCenterWidth(), 
			 (!m_ManualNumber && m_metrics->HasExtraPedalButtonRow()) ? 2 * m_metrics->GetButtonHeight() : m_metrics->GetButtonHeight());
	m_HBackground = m_panel->GetWood(m_metrics->GetKeyHorizBackgroundImageNum());
}

void GOGUIManualBackground::PrepareDraw(double scale, GOrgueBitmap* background)
{
	m_VBackground.PrepareTileBitmap(scale, m_VRect, 0, 0, background);
	m_HBackground.PrepareTileBitmap(scale, m_HRect, 0, 0, background);
}

void GOGUIManualBackground::Draw(GOrgueDC& dc)
{
	dc.DrawBitmap(m_VBackground, m_VRect);
	dc.DrawBitmap(m_HBackground, m_HRect);
	GOGUIControl::Draw(dc);
}

