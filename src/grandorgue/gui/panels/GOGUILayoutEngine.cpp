/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGUILayoutEngine.h"

#include <algorithm>

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
 * SIZES OF THE LINES OF THE CELLS OF THE GRIDS
 */

int GOGUILayoutEngine::computeCentredPosition(
  int cellStart, unsigned cellSize, unsigned itemSize) {
  return cellStart + (int)cellSize / 2 - (int)itemSize / 2;
}

int GOGUILayoutEngine::computeEnclosureAreaX(
  unsigned screenWidth, unsigned areaWidth) {
  return ((int)screenWidth - (int)areaWidth + 6) >> 1;
}

void GOGUILayoutEngine::computeGridBlitPosition(
  int areaX,
  int areaY,
  const std::function<unsigned(unsigned)> &getColumnWidth,
  const std::function<unsigned(unsigned)> &getRowHeight,
  unsigned rowGap,
  int rowN,
  int colN,
  unsigned width,
  unsigned height,
  int &outX,
  int &outY) {
  // the getters take the index from 0, the numbers of the grid are from 1
  const auto getColumnWidthByI
    = [&getColumnWidth](unsigned colI) { return getColumnWidth(colI + 1); };
  const auto getRowHeightByI
    = [&getRowHeight](unsigned rowI) { return getRowHeight(rowI + 1); };

  outX = computeCentredPosition(
    areaX
      + (int)GOGUIDisplayMetrics::computeCellOffset(
        colN - 1, getColumnWidthByI, 0),
    getColumnWidth(colN),
    width);
  outY = computeCentredPosition(
    areaY
      + (int)GOGUIDisplayMetrics::computeCellOffset(
        rowN - 1, getRowHeightByI, rowGap),
    getRowHeight(rowN),
    height);
}

std::pair<int, int> GOGUILayoutEngine::computeJambWidths(
  const std::function<unsigned(unsigned)> &getColumnWidth,
  unsigned nCols,
  bool isPairDrawstopCols,
  unsigned standardWidth) {
  const unsigned nLeftCols = nCols >> 1;
  const int totalWidth
    = (int)GOGUIDisplayMetrics::computeCellOffset(nCols, getColumnWidth, 0);
  const int leftWidth
    = (int)GOGUIDisplayMetrics::computeCellOffset(nLeftCols, getColumnWidth, 0);
  // the right jamb is the rest of the columns
  int rightWidth = totalWidth - leftWidth;
  int nudgeWidth = 0;

  if (isPairDrawstopCols)
    nudgeWidth = (int)((nCols >> 2) * (standardWidth / 4)) - 8;
  return {leftWidth + nudgeWidth, rightWidth + nudgeWidth};
}

std::pair<int, int> GOGUILayoutEngine::computeJambXs(
  int centerX,
  int centerWidth,
  int leftWidth,
  int rightWidth,
  bool isPairDrawstopCols) {
  int leftX = (centerX - leftWidth) >> 1;
  int rightX = 0;

  if (isPairDrawstopCols)
    leftX += 5;
  // The right jamb is centred in the right margin, whose width is centerX.
  // The shift of the left jamb propagates, as it did in the legacy code.
  rightX = leftX + centerX + centerWidth + (leftWidth - rightWidth) / 2;
  if (isPairDrawstopCols)
    rightX += 5;
  return {leftX, rightX};
}

int GOGUILayoutEngine::computeCenterExpansion(
  int screenWidth, int centerWidth, int leftWidth, int rightWidth) {
  const int freeWidth
    = screenWidth - centerWidth - 2 * std::max(leftWidth, rightWidth);

  return freeWidth > 0 ? freeWidth / 3 : 0;
}

unsigned GOGUILayoutEngine::GetDrawstopColumnsWidth(
  unsigned firstColN, unsigned nCols) {
  return GOGUIDisplayMetrics::computeCellOffset(
    nCols,
    [this, firstColN](unsigned colI) {
      return r_metrics.GetDrawstopColumnWidth(firstColN + colI);
    },
    0);
}

unsigned GOGUILayoutEngine::GetDrawstopRowsHeight(
  int firstRowN, unsigned nRows) {
  return GOGUIDisplayMetrics::computeCellOffset(
    nRows,
    [this, firstRowN](unsigned rowI) {
      return r_metrics.GetDrawstopRowHeight(firstRowN + rowI);
    },
    0);
}

unsigned GOGUILayoutEngine::GetButtonColumnsWidth(
  unsigned firstColN, unsigned nCols) {
  return GOGUIDisplayMetrics::computeCellOffset(
    nCols,
    [this, firstColN](unsigned colI) {
      return r_metrics.GetButtonColumnWidth(firstColN + colI);
    },
    0);
}

unsigned GOGUILayoutEngine::GetButtonRowsHeight(int firstRowN, unsigned nRows) {
  return GOGUIDisplayMetrics::computeCellOffset(
    nRows,
    [this, firstRowN](unsigned rowI) {
      return r_metrics.GetButtonRowHeight(firstRowN + rowI);
    },
    0);
}

unsigned GOGUILayoutEngine::GetEnclosureColumnsWidth(
  unsigned firstColN, unsigned nCols) {
  return GOGUIDisplayMetrics::computeCellOffset(
    nCols,
    [this, firstColN](unsigned colI) {
      return r_metrics.GetEnclosureColumnWidth(firstColN + colI);
    },
    0);
}

unsigned GOGUILayoutEngine::GetEnclosureRowsHeight(
  int firstRowN, unsigned nRows) {
  return GOGUIDisplayMetrics::computeCellOffset(
    nRows,
    [this, firstRowN](unsigned rowI) {
      return r_metrics.GetEnclosureRowHeight(firstRowN + rowI);
    },
    GOGUIDisplayMetrics::ROW_GAP);
}

/*
 * COMPUTED VALUES FROM ODF PARAMETERS
 */

unsigned GOGUILayoutEngine::GetJambLeftRightHeight() {
  return GetDrawstopRowsHeight(
    1, r_metrics.NumberOfDrawstopRowsToDisplay() + 1);
}

int GOGUILayoutEngine::GetJambLeftRightY() {
  const int lastRowN = (int)r_metrics.NumberOfDrawstopRowsToDisplay();

  return ((int)(r_metrics.GetScreenHeight() - GetJambLeftRightHeight() -
                (r_metrics.HasDrawstopColsOffset()
                   ? ((int)r_metrics.GetDrawstopRowHeight(lastRowN) / 2)
                   : 0))) /
         2;
}

std::pair<int, int> GOGUILayoutEngine::GetJambWidths() {
  return computeJambWidths(
    [this](unsigned colI) {
      return r_metrics.GetDrawstopColumnWidth(colI + 1);
    },
    r_metrics.NumberOfDrawstopColsToDisplay(),
    r_metrics.HasPairDrawstopCols(),
    r_metrics.GetDrawstopWidth());
}

unsigned GOGUILayoutEngine::GetJambTopHeight() {
  return GetDrawstopRowsHeight(
    100, r_metrics.NumberOfExtraDrawstopRowsToDisplay());
}

unsigned GOGUILayoutEngine::GetJambTopWidth() {
  return GetDrawstopColumnsWidth(
    1, r_metrics.NumberOfExtraDrawstopColsToDisplay());
}

int GOGUILayoutEngine::GetJambTopX() {
  return (r_metrics.GetScreenWidth() - GetJambTopWidth()) >> 1;
}

unsigned GOGUILayoutEngine::GetPistonTopHeight() {
  return GetButtonRowsHeight(100, r_metrics.NumberOfExtraButtonRows());
}

unsigned GOGUILayoutEngine::GetPistonWidth() {
  return GetButtonColumnsWidth(1, r_metrics.NumberOfButtonCols());
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
  return GetEnclosureColumnsWidth(1, m_Enclosures.size());
}

unsigned GOGUILayoutEngine::GetEnclosureAreaHeight() {
  // the gap after the last row is not a part of the area
  return GetEnclosureRowsHeight(1, r_metrics.GetNEnclosureRows())
    - GOGUIDisplayMetrics::ROW_GAP;
}

int GOGUILayoutEngine::GetEnclosureY() {
  int x = 0;
  int y = 0;

  GetEnclosureBlitPosition(
    1, 1, r_metrics.GetEnclosureWidth(), r_metrics.GetEnclosureHeight(), x, y);
  return y;
}

int GOGUILayoutEngine::GetEnclosureX(const GOGUIEnclosure *enclosure) {
  assert(enclosure);

  const auto it
    = std::find(m_Enclosures.begin(), m_Enclosures.end(), enclosure);
  int x = 0;
  int y = 0;

  if (it == m_Enclosures.end())
    throw(wxString) _("enclosure not found");
  GetEnclosureBlitPosition(
    1,
    (it - m_Enclosures.begin()) + 1,
    r_metrics.GetEnclosureWidth(),
    r_metrics.GetEnclosureHeight(),
    x,
    y);
  return x;
}

std::pair<int, int> GOGUILayoutEngine::GetJambXs() {
  const auto [leftWidth, rightWidth] = GetJambWidths();

  return computeJambXs(
    GetCenterX(),
    GetCenterWidth(),
    leftWidth,
    rightWidth,
    r_metrics.HasPairDrawstopCols());
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

void GOGUILayoutEngine::GetBlitPosition(
  GridType gridType,
  int rowN,
  int colN,
  unsigned width,
  unsigned height,
  int &outX,
  int &outY) {
  switch (gridType) {
  case GRID_DRAWSTOP:
    GetDrawstopBlitPosition(rowN, colN, outX, outY);
    break;
  case GRID_PUSHBUTTON:
    GetPushbuttonBlitPosition(rowN, colN, outX, outY);
    break;
  case GRID_ENCLOSURE:
    GetEnclosureBlitPosition(rowN, colN, width, height, outX, outY);
    break;
  }
}

void GOGUILayoutEngine::GetEnclosureBlitPosition(
  int rowN, int colN, unsigned width, unsigned height, int &outX, int &outY) {
  computeGridBlitPosition(
    computeEnclosureAreaX(r_metrics.GetScreenWidth(), GetEnclosureAreaWidth()),
    m_EnclosureY,
    [this](unsigned colN) { return r_metrics.GetEnclosureColumnWidth(colN); },
    [this](unsigned rowN) { return r_metrics.GetEnclosureRowHeight(rowN); },
    GOGUIDisplayMetrics::ROW_GAP,
    rowN,
    colN,
    width,
    height,
    outX,
    outY);
}

void GOGUILayoutEngine::GetDrawstopBlitPosition(
  const int drawstopRow, const int drawstopCol, int &blitX, int &blitY) {
  int i;
  if (drawstopRow > 99) {
    blitX = GetJambTopX() + GetDrawstopColumnsWidth(1, drawstopCol - 1) + 6;
    if (r_metrics.HasExtraDrawstopRowsAboveExtraButtonRows()) {
      blitY = GetJambTopDrawstop()
        + GetDrawstopRowsHeight(100, drawstopRow - 100) + 2;
    } else {
      blitY = GetJambTopDrawstop()
        + GetDrawstopRowsHeight(100, drawstopRow - 100)
        + GetButtonRowsHeight(100, r_metrics.NumberOfExtraButtonRows()) + 2;
    }
  } else {
    i = r_metrics.NumberOfDrawstopColsToDisplay() >> 1;
    if (drawstopCol <= i) {
      blitX = GetJambLeftX() + GetDrawstopColumnsWidth(1, drawstopCol - 1) + 6;
      blitY
        = GetJambLeftRightY() + GetDrawstopRowsHeight(1, drawstopRow - 1) + 32;
    } else {
      blitX = GetJambRightX()
        + GetDrawstopColumnsWidth(i + 1, drawstopCol - 1 - i) + 6;
      blitY
        = GetJambLeftRightY() + GetDrawstopRowsHeight(1, drawstopRow - 1) + 32;
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
      blitY += (int)r_metrics.GetDrawstopRowHeight(drawstopRow) / 2;
  }
}

void GOGUILayoutEngine::GetPushbuttonBlitPosition(
  const int buttonRow, const int buttonCol, int &blitX, int &blitY) {
  blitX = GetPistonX() + GetButtonColumnsWidth(1, buttonCol - 1) + 6;
  if (buttonRow > 99) {
    if (r_metrics.HasExtraDrawstopRowsAboveExtraButtonRows()) {
      blitY = GetJambTopPiston() + GetButtonRowsHeight(100, buttonRow - 100)
        + GetDrawstopRowsHeight(
                100, r_metrics.NumberOfExtraDrawstopRowsToDisplay())
        + 5;
    } else {
      blitY
        = GetJambTopPiston() + GetButtonRowsHeight(100, buttonRow - 100) + 5;
    }
  } else {
    int i = buttonRow;
    if (i == 99)
      i = 0;

    if (i >= (int)m_Manuals.size()) {
      // The rows are stacked upward from the hack Y: every row is a manual
      // and its piston row
      int rowsHeight = 0;

      for (int rowN = (int)m_Manuals.size(); rowN <= i; ++rowN)
        rowsHeight += (int)r_metrics.GetManualHeight()
          + (int)r_metrics.GetButtonRowHeight(rowN);
      blitY = GetHackY() - rowsHeight + (int)r_metrics.GetManualHeight() + 5;
    } else
      blitY = GetManualRenderInfo(i).piston_y + 5;

    if (r_metrics.HasExtraPedalButtonRow() && !buttonRow)
      blitY += (int)r_metrics.GetButtonRowHeight(99);
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
        m_CenterY -= r_metrics.GetButtonRowHeight(99);
      m_ManualRenderInfo[0].piston_y = m_CenterY;
      m_CenterWidth = std::max(m_CenterWidth, (int)GetEnclosureAreaWidth());
      m_CenterY -= GOGUIDisplayMetrics::ROW_GAP;
      m_CenterY -= GetEnclosureAreaHeight();
      m_EnclosureY = m_CenterY;
      m_CenterY -= GOGUIDisplayMetrics::ROW_GAP;
    }
    if (!i && !m_Manuals[i] && m_Enclosures.size()) {
      m_CenterY -= GOGUIDisplayMetrics::ROW_GAP;
      m_CenterY -= GetEnclosureAreaHeight();
      m_EnclosureY = m_CenterY;
      m_CenterY -= GOGUIDisplayMetrics::ROW_GAP;
    }

    if (!m_Manuals[i])
      continue;

    if (i) {
      if (!r_metrics.HasButtonsAboveManuals()) {
        m_CenterY -= r_metrics.GetButtonRowHeight(i);
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
        m_CenterY -= r_metrics.GetButtonRowHeight(i);
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

  const auto [leftWidth, rightWidth] = GetJambWidths();

  m_CenterWidth += computeCenterExpansion(
    (int)r_metrics.GetScreenWidth(), m_CenterWidth, leftWidth, rightWidth);

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
