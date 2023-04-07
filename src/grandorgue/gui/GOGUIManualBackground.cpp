/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGUIManualBackground.h"

#include "config/GOConfigReader.h"
#include "model/GOManual.h"
#include "primitives/GODC.h"

#include "GOGUIDisplayMetrics.h"
#include "GOGUILayoutEngine.h"
#include "GOGUIPanel.h"

GOGUIManualBackground::GOGUIManualBackground(
  GOGUIPanel *panel, unsigned manual_number)
  : GOGUIControl(panel, NULL), m_ManualNumber(manual_number) {}

void GOGUIManualBackground::Init(GOConfigReader &cfg, wxString group) {
  GOGUIControl::Init(cfg, group);
}

void GOGUIManualBackground::Load(GOConfigReader &cfg, wxString group) {
  GOGUIControl::Load(cfg, group);
}

void GOGUIManualBackground::Layout() {
  const GOGUILayoutEngine::MANUAL_RENDER_INFO &mri
    = m_layout->GetManualRenderInfo(m_ManualNumber);
  m_BoundingRect = wxRect(mri.x, mri.y, mri.width, mri.height);
  m_VRect = wxRect(
    m_layout->GetCenterX(), mri.y, m_layout->GetCenterWidth(), mri.height);
  m_VBackground = m_panel->GetWood(m_metrics->GetKeyVertBackgroundImageNum());
  m_HRect = wxRect(
    m_layout->GetCenterX(),
    mri.piston_y,
    m_layout->GetCenterWidth(),
    (!m_ManualNumber && m_metrics->HasExtraPedalButtonRow())
      ? 2 * m_metrics->GetButtonHeight()
      : m_metrics->GetButtonHeight());
  m_HBackground = m_panel->GetWood(m_metrics->GetKeyHorizBackgroundImageNum());
}

void GOGUIManualBackground::PrepareDraw(double scale, GOBitmap *background) {
  m_VBackground.PrepareTileBitmap(scale, m_VRect, 0, 0, background);
  m_HBackground.PrepareTileBitmap(scale, m_HRect, 0, 0, background);
}

void GOGUIManualBackground::Draw(GODC &dc) {
  dc.DrawBitmap(m_VBackground, m_VRect);
  dc.DrawBitmap(m_HBackground, m_HRect);
  GOGUIControl::Draw(dc);
}
