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

#include "GOGUIManualBackground.h"

#include "GOGUIDisplayMetrics.h"
#include "GOGUIPanel.h"
#include "GOrgueConfigReader.h"
#include "GOrgueManual.h"

GOGUIManualBackground::GOGUIManualBackground(GOGUIPanel* panel, unsigned manual_number):
	GOGUIControl(panel, NULL),
	m_ManualNumber(manual_number)
{
}

void GOGUIManualBackground::Init(GOrgueConfigReader& cfg, wxString group)
{
	GOGUIControl::Init(cfg, group);

	const GOGUIDisplayMetrics::MANUAL_RENDER_INFO &mri = m_metrics->GetManualRenderInfo(m_ManualNumber);
	m_BoundingRect = wxRect(mri.x, mri.y, mri.width, mri.height);
}

void GOGUIManualBackground::Load(GOrgueConfigReader& cfg, wxString group)
{
	GOGUIControl::Load(cfg, group);

	const GOGUIDisplayMetrics::MANUAL_RENDER_INFO &mri = m_metrics->GetManualRenderInfo(m_ManualNumber);
	m_BoundingRect = wxRect(mri.x, mri.y, mri.width, mri.height);
}

void GOGUIManualBackground::Draw(GOrgueDC& dc)
{
	const GOGUIDisplayMetrics::MANUAL_RENDER_INFO &mri = m_metrics->GetManualRenderInfo(m_ManualNumber);

	m_panel->TileWood(dc, m_metrics->GetKeyVertBackgroundImageNum(), m_metrics->GetCenterX(), mri.y,
			  m_metrics->GetCenterWidth(), mri.height);

	m_panel->TileWood(dc, m_metrics->GetKeyHorizBackgroundImageNum(), m_metrics->GetCenterX(),
			  mri.piston_y, m_metrics->GetCenterWidth(), 
			  (!m_ManualNumber && m_metrics->HasExtraPedalButtonRow()) ? 2 * m_metrics->GetButtonHeight() : m_metrics->GetButtonHeight());
	GOGUIControl::Draw(dc);
}

