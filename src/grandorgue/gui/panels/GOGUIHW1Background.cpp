/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGUIHW1Background.h"

#include "primitives/GODC.h"

#include "GOGUIDisplayMetrics.h"
#include "GOGUILayoutEngine.h"
#include "GOGUIPanel.h"

GOGUIHW1Background::GOGUIHW1Background(GOGUIPanel *panel)
  : GOGUIControl(panel, NULL), m_Images() {}

void GOGUIHW1Background::Init(GOConfigReader &cfg, wxString group) {
  GOGUIControl::Init(cfg, group);
}

bool is_rect_valid(wxRect &rect) { return rect.width > 0 && rect.height > 0; }

void GOGUIHW1Background::Layout() {
  m_Images.clear();
  wxRect rect;

  rect = wxRect(0, 0, p_layout->GetCenterX(), p_metrics->GetScreenHeight());
  if (is_rect_valid(rect))
    m_Images.emplace_back(
      rect, p_panel->GetWoodImage(p_metrics->GetDrawstopBackgroundImageNum()));
  rect = wxRect(
    p_layout->GetCenterX() + p_layout->GetCenterWidth(),
    0,
    p_metrics->GetScreenWidth()
      - (p_layout->GetCenterX() + p_layout->GetCenterWidth()),
    p_metrics->GetScreenHeight());
  if (is_rect_valid(rect))
    m_Images.emplace_back(
      rect, p_panel->GetWoodImage(p_metrics->GetDrawstopBackgroundImageNum()));
  rect = wxRect(
    p_layout->GetCenterX(),
    0,
    p_layout->GetCenterWidth(),
    p_metrics->GetScreenHeight());
  if (is_rect_valid(rect))
    m_Images.emplace_back(
      rect, p_panel->GetWoodImage(p_metrics->GetConsoleBackgroundImageNum()));

  if (p_metrics->HasPairDrawstopCols()) {
    for (unsigned i = 0; i < (p_metrics->NumberOfDrawstopColsToDisplay() >> 2);
         i++) {
      rect = wxRect(
        i * (2 * p_metrics->GetDrawstopWidth() + 18) + p_layout->GetJambLeftX()
          - 5,
        p_layout->GetJambLeftRightY(),
        2 * p_metrics->GetDrawstopWidth() + 10,
        p_layout->GetJambLeftRightHeight());
      if (is_rect_valid(rect))
        m_Images.emplace_back(
          rect,
          p_panel->GetWoodImage(
            p_metrics->GetDrawstopInsetBackgroundImageNum()));
      rect = wxRect(
        i * (2 * p_metrics->GetDrawstopWidth() + 18) + p_layout->GetJambRightX()
          - 5,
        p_layout->GetJambLeftRightY(),
        2 * p_metrics->GetDrawstopWidth() + 10,
        p_layout->GetJambLeftRightHeight());
      if (is_rect_valid(rect))
        m_Images.emplace_back(
          rect,
          p_panel->GetWoodImage(
            p_metrics->GetDrawstopInsetBackgroundImageNum()));
    }
  }

  if (p_metrics->HasTrimAboveExtraRows()) {
    rect = wxRect(
      p_layout->GetCenterX(),
      p_layout->GetCenterY(),
      p_layout->GetCenterWidth(),
      8);
    if (is_rect_valid(rect))
      m_Images.emplace_back(
        rect, p_panel->GetWoodImage(p_metrics->GetKeyVertBackgroundImageNum()));
  }

  if (p_layout->GetJambTopHeight() + p_layout->GetPistonTopHeight()) {
    rect = wxRect(
      p_layout->GetCenterX(),
      p_layout->GetJambTopY(),
      p_layout->GetCenterWidth(),
      p_layout->GetJambTopHeight() + p_layout->GetPistonTopHeight());
    if (is_rect_valid(rect))
      m_Images.emplace_back(
        rect,
        p_panel->GetWoodImage(p_metrics->GetKeyHorizBackgroundImageNum()));
  }
}

void GOGUIHW1Background::PrepareDraw(double scale, GOBitmap *background) {
  for (unsigned i = 0; i < m_Images.size(); i++)
    m_Images[i].bmp.BuildTileBitmap(scale, m_Images[i].rect, 0, 0, background);
}

void GOGUIHW1Background::Draw(GODC &dc) {
  for (unsigned i = 0; i < m_Images.size(); i++)
    dc.DrawBitmap(m_Images[i].bmp, m_Images[i].rect);
  GOGUIControl::Draw(dc);
}
