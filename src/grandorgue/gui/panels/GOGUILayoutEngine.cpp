/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGUILayoutEngine.h"

#include <wx/intl.h>

#include "GOGUIDisplayMetrics.h"
#include "GOGUIEnclosure.h"
#include "GOGUIManual.h"

GOGUILayoutEngine::GOGUILayoutEngine(GOGUIDisplayMetrics &metrics)
  : r_metrics(metrics),
    m_ManualRenderInfo(),
    m_Enclosures(),
    m_Manuals(),
    m_HackY(0),
    m_EnclosureY(0),
    m_CenterY(0),
    m_CenterWidth(0) {}

GOGUILayoutEngine::~GOGUILayoutEngine() {}

/*
 * COMPUTED VALUES FROM ODF PARAMETERS
 */

unsigned GOGUILayoutEngine::GetJambLeftRightHeight() {
  return (r_metrics.NumberOfDrawstopRowsToDisplay() + 1)
    * r_metrics.GetDrawstopHeight();
}

int GOGUILayoutEngine::GetJambLeftRightY() {
  return ((int)(r_metrics.GetScreenHeight() - GetJambLeftRightHeight() -
                (r_metrics.HasDrawstopColsOffset()
                     ? (r_metrics.GetDrawstopHeight() / 2)
                     : 0))) /
         2;
}

int GOGUILayoutEngine::GetJambLeftRightWidth() {
  int jamblrw = r_metrics.NumberOfDrawstopColsToDisplay()
    * r_metrics.GetDrawstopWidth() / 2;
  if (r_metrics.HasPairDrawstopCols())
    jamblrw += ((r_metrics.NumberOfDrawstopColsToDisplay() >> 2)
                * (r_metrics.GetDrawstopWidth() / 4))
      - 8;
  return jamblrw;
}

unsigned GOGUILayoutEngine::GetJambTopHeight() {
  return r_metrics.NumberOfExtraDrawstopRowsToDisplay()
    * r_metrics.GetDrawstopHeight();
}

unsigned GOGUILayoutEngine::GetJambTopWidth() {
  return r_metrics.NumberOfExtraDrawstopColsToDisplay()
    * r_metrics.GetDrawstopWidth();
}

int GOGUILayoutEngine::GetJambTopX() {
  return (r_metrics.GetScreenWidth() - GetJambTopWidth()) >> 1;
}

unsigned GOGUILayoutEngine::GetPistonTopHeight() {
  return r_metrics.NumberOfExtraButtonRows() * r_metrics.GetButtonHeight();
}

unsigned GOGUILayoutEngine::GetPistonWidth() {
  return r_metrics.NumberOfButtonCols() * r_metrics.GetButtonWidth();
}

int GOGUILayoutEngine::GetPistonX() {
  return (r_metrics.GetScreenWidth() - GetPistonWidth()) >> 1;
}

/*
 * COMPUTED VALUES FROM ODF PARAMETERS & SCREEN
 */

int GOGUILayoutEngine::GetCenterWidth() { return m_CenterWidth; }

int GOGUILayoutEngine::GetCenterX() {
  return ((int)r_metrics.GetScreenWidth() - GetCenterWidth()) >> 1;
}

int GOGUILayoutEngine::GetCenterY() { return m_CenterY; }

int GOGUILayoutEngine::GetHackY() { return m_HackY; }

unsigned GOGUILayoutEngine::GetEnclosureAreaWidth() {
  return r_metrics.GetEnclosureWidth() * m_Enclosures.size();
}

int GOGUILayoutEngine::GetEnclosureY() { return m_EnclosureY; }

int GOGUILayoutEngine::GetEnclosureX(const GOGUIEnclosure *enclosure) {
  assert(enclosure);

  int enclosure_x
    = (r_metrics.GetScreenWidth() - GetEnclosureAreaWidth() + 6) >> 1;
  for (unsigned int i = 0; i < m_Enclosures.size(); i++) {
    if (enclosure == m_Enclosures[i])
      return enclosure_x;
    enclosure_x += r_metrics.GetEnclosureWidth();
  }

  throw(wxString) _("enclosure not found");
}

int GOGUILayoutEngine::GetJambLeftX() {
  int jamblx = (GetCenterX() - GetJambLeftRightWidth()) >> 1;
  if (r_metrics.HasPairDrawstopCols())
    jamblx += 5;
  return jamblx;
}

int GOGUILayoutEngine::GetJambRightX() {
  int jambrx = GetJambLeftX() + GetCenterX() + GetCenterWidth();
  if (r_metrics.HasPairDrawstopCols())
    jambrx += 5;
  return jambrx;
}

int GOGUILayoutEngine::GetJambTopDrawstop() {
  if (r_metrics.HasTrimAboveExtraRows())
    return GetCenterY() + 8;
  return GetCenterY();
}

int GOGUILayoutEngine::GetJambTopPiston() {
  if (r_metrics.HasTrimAboveExtraRows())
    return GetCenterY() + 8;
  return GetCenterY();
}

int GOGUILayoutEngine::GetJambTopY() {
  if (r_metrics.HasTrimAboveExtraRows())
    return GetCenterY() + 8;
  return GetCenterY();
}

/*
 * BLIT POSITION FUNCTIONS
 */

void GOGUILayoutEngine::GetDrawstopBlitPosition(
  const int drawstopRow, const int drawstopCol, int &blitX, int &blitY) {
  int i;
  if (drawstopRow > 99) {
    blitX
      = GetJambTopX() + (drawstopCol - 1) * r_metrics.GetDrawstopWidth() + 6;
    if (r_metrics.HasExtraDrawstopRowsAboveExtraButtonRows()) {
      blitY = GetJambTopDrawstop()
        + (drawstopRow - 100) * r_metrics.GetDrawstopHeight() + 2;
    } else {
      blitY = GetJambTopDrawstop()
        + (drawstopRow - 100) * r_metrics.GetDrawstopHeight()
        + (r_metrics.NumberOfExtraButtonRows() * r_metrics.GetButtonHeight())
        + 2;
    }
  } else {
    i = r_metrics.NumberOfDrawstopColsToDisplay() >> 1;
    if (drawstopCol <= i) {
      blitX
        = GetJambLeftX() + (drawstopCol - 1) * r_metrics.GetDrawstopWidth() + 6;
      blitY = GetJambLeftRightY()
        + (drawstopRow - 1) * r_metrics.GetDrawstopHeight() + 32;
    } else {
      blitX = GetJambRightX()
        + (drawstopCol - 1 - i) * r_metrics.GetDrawstopWidth() + 6;
      blitY = GetJambLeftRightY()
        + (drawstopRow - 1) * r_metrics.GetDrawstopHeight() + 32;
    }
    if (r_metrics.HasPairDrawstopCols())
      blitX
        += (((drawstopCol - 1) % i) >> 1) * (r_metrics.GetDrawstopWidth() / 4);

    if (drawstopCol <= i)
      i = drawstopCol;
    else
      i = r_metrics.NumberOfDrawstopColsToDisplay() - drawstopCol + 1;
    if (
      r_metrics.HasDrawstopColsOffset()
      && (i & 1) ^ r_metrics.HasDrawstopOuterColOffsetUp())
      blitY += r_metrics.GetDrawstopHeight() / 2;
  }
}

void GOGUILayoutEngine::GetPushbuttonBlitPosition(
  const int buttonRow, const int buttonCol, int &blitX, int &blitY) {
  blitX = GetPistonX() + (buttonCol - 1) * r_metrics.GetButtonWidth() + 6;
  if (buttonRow > 99) {
    if (r_metrics.HasExtraDrawstopRowsAboveExtraButtonRows()) {
      blitY = GetJambTopPiston()
        + (buttonRow - 100) * r_metrics.GetButtonHeight()
        + (r_metrics.NumberOfExtraDrawstopRowsToDisplay()
           * r_metrics.GetDrawstopHeight())
        + 5;
    } else {
      blitY = GetJambTopPiston()
        + (buttonRow - 100) * r_metrics.GetButtonHeight() + 5;
    }
  } else {
    int i = buttonRow;
    if (i == 99)
      i = 0;

    if (i >= (int)m_Manuals.size())
      blitY = GetHackY()
        - (i + 1 - (int)m_Manuals.size())
          * (r_metrics.GetManualHeight() + r_metrics.GetButtonHeight())
        + r_metrics.GetManualHeight() + 5;
    else
      blitY = GetManualRenderInfo(i).piston_y + 5;

    if (r_metrics.HasExtraPedalButtonRow() && !buttonRow)
      blitY += r_metrics.GetButtonHeight();
    if (r_metrics.HasExtraPedalButtonRowOffset() && buttonRow == 99)
      blitX -= r_metrics.GetButtonWidth() / 2 + 2;
  }
}

/*
 * UPDATE METHOD - updates parameters dependent on ODF parameters and
 * screen stuff.
 */

/* TODO: this method could do with a cleanup */

void GOGUILayoutEngine::Update() {
  if (!m_Manuals.size())
    m_Manuals.push_back(NULL);

  m_ManualRenderInfo.resize(m_Manuals.size());
  m_CenterY = r_metrics.GetScreenHeight() - r_metrics.GetPedalHeight();
  m_CenterWidth = std::max(GetJambTopWidth(), GetPistonWidth());

  for (unsigned i = 0; i < m_Manuals.size(); i++) {
    if (!i && m_Manuals[i]) {
      m_ManualRenderInfo[0].height = r_metrics.GetPedalHeight();
      m_ManualRenderInfo[0].keys_y = m_ManualRenderInfo[0].y = m_CenterY;
      m_CenterY -= r_metrics.GetPedalHeight();
      if (r_metrics.HasExtraPedalButtonRow())
        m_CenterY -= r_metrics.GetButtonHeight();
      m_ManualRenderInfo[0].piston_y = m_CenterY;
      m_CenterWidth = std::max(m_CenterWidth, (int)GetEnclosureAreaWidth());
      m_CenterY -= GOGUIDisplayMetrics::ROW_GAP;
      m_CenterY -= r_metrics.GetEnclosureHeight();
      m_EnclosureY = m_CenterY;
      m_CenterY -= GOGUIDisplayMetrics::ROW_GAP;
    }
    if (!i && !m_Manuals[i] && m_Enclosures.size()) {
      m_CenterY -= GOGUIDisplayMetrics::ROW_GAP;
      m_CenterY -= r_metrics.GetEnclosureHeight();
      m_EnclosureY = m_CenterY;
      m_CenterY -= GOGUIDisplayMetrics::ROW_GAP;
    }

    if (!m_Manuals[i])
      continue;

    if (i) {
      if (!r_metrics.HasButtonsAboveManuals()) {
        m_CenterY -= r_metrics.GetButtonHeight();
        m_ManualRenderInfo[i].piston_y = m_CenterY;
      }
      m_ManualRenderInfo[i].height = r_metrics.GetManualHeight();
      if (r_metrics.HasTrimBelowManuals() && i == 1) {
        m_ManualRenderInfo[i].height += 8;
        m_CenterY -= 8;
      }
      m_CenterY -= r_metrics.GetManualHeight();
      m_ManualRenderInfo[i].keys_y = m_CenterY;
      if (r_metrics.HasTrimAboveManuals() && i + 1 == m_Manuals.size()) {
        m_CenterY -= 8;
        m_ManualRenderInfo[i].height += 8;
      }
      if (r_metrics.HasButtonsAboveManuals()) {
        m_CenterY -= r_metrics.GetButtonHeight();
        m_ManualRenderInfo[i].piston_y = m_CenterY;
      }
      m_ManualRenderInfo[i].y = m_CenterY;
    }
    m_ManualRenderInfo[i].width = 1;
    if (i) {
      for (unsigned j = 0; j < m_Manuals[i]->GetKeyCount(); j++) {
        if (!m_Manuals[i]->IsSharp(j))
          m_ManualRenderInfo[i].width += r_metrics.GetManualKeyWidth();
      }
    } else {
      for (unsigned j = 0; j < m_Manuals[i]->GetKeyCount(); j++) {
        m_ManualRenderInfo[i].width += r_metrics.GetPedalKeyWidth();
        if (j && !m_Manuals[i]->IsSharp(j - 1) && !m_Manuals[i]->IsSharp(j))
          m_ManualRenderInfo[i].width += r_metrics.GetPedalKeyWidth();
      }
    }
    m_ManualRenderInfo[i].x
      = (r_metrics.GetScreenWidth() - m_ManualRenderInfo[i].width) >> 1;
    m_ManualRenderInfo[i].width += 16;
    if ((int)m_ManualRenderInfo[i].width > m_CenterWidth)
      m_CenterWidth = m_ManualRenderInfo[i].width;
  }

  m_HackY = m_CenterY;

  if (
    m_CenterWidth + GetJambLeftRightWidth() * 2
    < (int)r_metrics.GetScreenWidth())
    m_CenterWidth += (r_metrics.GetScreenWidth() - m_CenterWidth
                      - GetJambLeftRightWidth() * 2)
      / 3;

  m_CenterY -= GetPistonTopHeight();
  m_CenterY -= GetJambTopHeight();
  if (r_metrics.HasTrimAboveExtraRows())
    m_CenterY -= 8;
}

const GOGUILayoutEngine::MANUAL_RENDER_INFO &GOGUILayoutEngine::
  GetManualRenderInfo(const unsigned manual_nb) const {
  assert(manual_nb < m_ManualRenderInfo.size());
  return m_ManualRenderInfo[manual_nb];
}

void GOGUILayoutEngine::RegisterEnclosure(GOGUIEnclosure *enclosure) {
  m_Enclosures.push_back(enclosure);
}

void GOGUILayoutEngine::RegisterManual(GOGUIManual *manual) {
  m_Manuals.push_back(manual);
}

unsigned GOGUILayoutEngine::GetManualNumber() { return m_Manuals.size(); }
