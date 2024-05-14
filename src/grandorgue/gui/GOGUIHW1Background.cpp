/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
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

  rect = wxRect(0, 0, m_layout->GetCenterX(), m_metrics->GetScreenHeight());
  if (is_rect_valid(rect))
    m_Images.push_back(GOBackgroundImage(
      rect, m_panel->GetWood(m_metrics->GetDrawstopBackgroundImageNum())));
  rect = wxRect(
    m_layout->GetCenterX() + m_layout->GetCenterWidth(),
    0,
    m_metrics->GetScreenWidth()
      - (m_layout->GetCenterX() + m_layout->GetCenterWidth()),
    m_metrics->GetScreenHeight());
  if (is_rect_valid(rect))
    m_Images.push_back(GOBackgroundImage(
      rect, m_panel->GetWood(m_metrics->GetDrawstopBackgroundImageNum())));
  rect = wxRect(
    m_layout->GetCenterX(),
    0,
    m_layout->GetCenterWidth(),
    m_metrics->GetScreenHeight());
  if (is_rect_valid(rect))
    m_Images.push_back(GOBackgroundImage(
      rect, m_panel->GetWood(m_metrics->GetConsoleBackgroundImageNum())));

  if (m_metrics->HasPairDrawstopCols()) {
    for (unsigned i = 0; i < (m_metrics->NumberOfDrawstopColsToDisplay() >> 2);
         i++) {
      rect = wxRect(
        i * (2 * m_metrics->GetDrawstopWidth() + 18) + m_layout->GetJambLeftX()
          - 5,
        m_layout->GetJambLeftRightY(),
        2 * m_metrics->GetDrawstopWidth() + 10,
        m_layout->GetJambLeftRightHeight());
      if (is_rect_valid(rect))
        m_Images.push_back(GOBackgroundImage(
          rect,
          m_panel->GetWood(m_metrics->GetDrawstopInsetBackgroundImageNum())));
      rect = wxRect(
        i * (2 * m_metrics->GetDrawstopWidth() + 18) + m_layout->GetJambRightX()
          - 5,
        m_layout->GetJambLeftRightY(),
        2 * m_metrics->GetDrawstopWidth() + 10,
        m_layout->GetJambLeftRightHeight());
      if (is_rect_valid(rect))
        m_Images.push_back(GOBackgroundImage(
          rect,
          m_panel->GetWood(m_metrics->GetDrawstopInsetBackgroundImageNum())));
    }
  }

  if (m_metrics->HasTrimAboveExtraRows()) {
    rect = wxRect(
      m_layout->GetCenterX(),
      m_layout->GetCenterY(),
      m_layout->GetCenterWidth(),
      8);
    if (is_rect_valid(rect))
      m_Images.push_back(GOBackgroundImage(
        rect, m_panel->GetWood(m_metrics->GetKeyVertBackgroundImageNum())));
  }

  if (m_layout->GetJambTopHeight() + m_layout->GetPistonTopHeight()) {
    rect = wxRect(
      m_layout->GetCenterX(),
      m_layout->GetJambTopY(),
      m_layout->GetCenterWidth(),
      m_layout->GetJambTopHeight() + m_layout->GetPistonTopHeight());
    if (is_rect_valid(rect))
      m_Images.push_back(GOBackgroundImage(
        rect, m_panel->GetWood(m_metrics->GetKeyHorizBackgroundImageNum())));
  }
}

void GOGUIHW1Background::PrepareDraw(double scale, GOBitmap *background) {
  for (unsigned i = 0; i < m_Images.size(); i++)
    m_Images[i].bmp.PrepareTileBitmap(
      scale, m_Images[i].rect, 0, 0, background);
}

void GOGUIHW1Background::Draw(GODC &dc) {
  for (unsigned i = 0; i < m_Images.size(); i++)
    dc.DrawBitmap(m_Images[i].bmp, m_Images[i].rect);
  GOGUIControl::Draw(dc);
}
