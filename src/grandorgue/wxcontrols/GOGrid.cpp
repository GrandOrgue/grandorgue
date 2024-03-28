/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGrid.h"

#include <algorithm>

#include <wx/dc.h>

#include "size/GOAdditionalSizeKeeper.h"

class GOCellRenderer : public wxGridCellStringRenderer {
private:
  bool m_IsRightVisible;

  virtual void Draw(
    wxGrid &grid,
    wxGridCellAttr &attr,
    wxDC &dc,
    const wxRect &rectCell,
    int row,
    int col,
    bool isSelected) override;

public:
  GOCellRenderer(bool isRightVisible) : m_IsRightVisible(isRightVisible) {}
};

static wxString truncate_line_left(
  wxDC &dc, const wxString &line, wxCoord targetWidth) {
  // Measure line
  wxCoord lineWidth, lineHeight;
  dc.GetTextExtent(line, &lineWidth, &lineHeight);
  if (lineWidth <= targetWidth || line.empty()) {
    return line;
  }
  // Need to truncate
  wxString ellipsis(wxT("..."));
  wxCoord ellipsisW, ellipsisH;
  dc.GetTextExtent(ellipsis, &ellipsisW, &ellipsisH);
  if (ellipsisW > targetWidth) {
    while (ellipsisW > targetWidth && ellipsis.size() >= 1) {
      ellipsis = ellipsis.Mid(1);
      dc.GetTextExtent(ellipsis, &ellipsisW, &ellipsisH);
    }
    return ellipsis;
  }
  // Find the number of first characters to remove
  // Invariant: l is too low (lW > targetWidth), u is enough (uW <= targetWidth)
  size_t l = 0, u = line.size();
  wxCoord lW = lineWidth, uW = ellipsisW;
  while (l + 1 < u) {
    // Heuristic: consider average character width
    wxCoord avgCharW = (lW - uW) / (u - l);
    size_t mid = avgCharW != 0
      ? std::clamp(l + (lW - targetWidth) / avgCharW, l + 1, u - 1)
      : (l + u) / 2;
    wxCoord midW, midH;
    wxString midLine = ellipsis + line.Mid(mid);
    dc.GetTextExtent(midLine, &midW, &midH);
    if (midW <= targetWidth) {
      u = mid;
      uW = midW;
    } else {
      l = mid;
      lW = midW;
    }
  }
  return ellipsis + line.Mid(u);
}

static void draw_text_rectangle(
  wxGrid &grid,
  wxDC &dc,
  const wxArrayString &lines,
  const wxRect &rect,
  int horizAlign,
  int vertAlign,
  bool isRightVisible) {
  // Based on wxGrid::DrawTextRectangle(wxDC, wxArrayString, ...).
  // Simplified for HORIZONTAL text orientation
  if (lines.empty()) {
    return;
  }

  // from anonymous namespace in grid.cpp
  const int GRID_TEXT_MARGIN = 1;

  wxDCClipper clip(dc, rect);
  long textWidth, textHeight;
  grid.GetTextBoxSize(dc, lines, &textWidth, &textHeight);

  int x = 0, y = 0;
  switch (vertAlign) {
  case wxALIGN_BOTTOM:
    y = rect.y + (rect.height - textHeight - GRID_TEXT_MARGIN);
    break;
  case wxALIGN_CENTRE:
    y = rect.y + ((rect.height - textHeight) / 2);
    break;
  case wxALIGN_TOP:
  default:
    y = rect.y + GRID_TEXT_MARGIN;
    break;
  }

  size_t nLines = lines.GetCount();
  for (size_t l = 0; l < nLines; l++) {
    const wxString &line = lines[l];
    if (line.empty()) {
      y += dc.GetCharHeight();
      continue;
    }
    wxCoord lineWidth = 0, lineHeight = 0;
    dc.GetTextExtent(line, &lineWidth, &lineHeight);

    switch (horizAlign) {
    case wxALIGN_RIGHT:
      x = rect.x + (rect.width - lineWidth - GRID_TEXT_MARGIN);
      break;
    case wxALIGN_CENTRE:
      x = rect.x + ((rect.width - lineWidth) / 2);
      if (textWidth > rect.width) {
        // if text box is too wide, prefer showing its right part
        x -= (textWidth - rect.width) / 2;
      }
      break;
    case wxALIGN_LEFT:
    default:
      x = rect.x + GRID_TEXT_MARGIN;

      if (textWidth > rect.width && isRightVisible) {
        // if text box is too wide, prefer showing its right part
        x -= textWidth - rect.width;
      }
      break;
    }

    if (x >= rect.x || !isRightVisible) {
      // Text fits
      dc.DrawText(line, x, y);
    } else {
      wxCoord availableWidth = std::min(rect.width, x + lineWidth - rect.x);
      wxString truncatedLine = truncate_line_left(dc, line, availableWidth);
      wxCoord truncatedLineW, truncatedLineH;
      dc.GetTextExtent(truncatedLine, &truncatedLineW, &truncatedLineH);
      x += lineWidth - truncatedLineW;
      dc.DrawText(truncatedLine, x, y);
    }
    y += lineHeight;
  }
}

void GOCellRenderer::Draw(
  wxGrid &grid,
  wxGridCellAttr &attr,
  wxDC &dc,
  const wxRect &rectCell,
  int row,
  int col,
  bool isSelected) {
  /* Based on wxGridCellStringRenderer::Draw with overflow-related code
   * removed */
  // Erase background
  wxGridCellRenderer::Draw(grid, attr, dc, rectCell, row, col, isSelected);

  // Prepare for rendering text
  SetTextColoursAndFont(grid, attr, dc, isSelected);
  wxArrayString lines;
  grid.StringToLines(grid.GetCellValue(row, col), lines);
  int horizAlign, vertAlign;
  attr.GetAlignment(&horizAlign, &vertAlign);

  // Render text
  wxRect rect = rectCell;
  rect.Inflate(-1);
  draw_text_rectangle(
    grid, dc, lines, rect, horizAlign, vertAlign, m_IsRightVisible);
}

GOGrid::GOGrid(
  wxWindow *parent,
  wxWindowID id,
  const wxPoint &pos,
  const wxSize &size,
  long style,
  const wxString &name)
  : wxGrid(parent, id, pos, size, style, name),
    p_NormalVisibleRenderer(new GOCellRenderer(false)),
    p_RightVisibleRenderer(new GOCellRenderer(true)) {}

GOGrid::~GOGrid() {
  // force deleting p_RightVisibleRenderer after deletion of all columns
  p_RightVisibleRenderer->DecRef();
  p_NormalVisibleRenderer->DecRef();
}

bool GOGrid::IsColumnRightVisible(unsigned colN) const {
  return colN < m_AreColumnsRightVisible.size()
    && m_AreColumnsRightVisible[colN];
}

void GOGrid::SetColumnRightVisible(unsigned colN, bool isRightVisible) {
  // ensure that m_AreColumnsRightVisible contains colN
  while (m_AreColumnsRightVisible.size() <= colN)
    m_AreColumnsRightVisible.push_back(false);
  m_AreColumnsRightVisible[colN] = isRightVisible;
}

const wxString WX_COL_SIZE_KEY_FMT = wxT("Column%03dSize");

void GOGrid::ApplyColumnSizes(const GOAdditionalSizeKeeper &sizeKeeper) {
  for (int l = GetNumberCols(), i = 0; i < l; i++) {
    int newSize
      = sizeKeeper.GetAdditionalSize(wxString::Format(WX_COL_SIZE_KEY_FMT, i));

    if (newSize > 0)
      SetColSize(i, newSize);
  }
}

void GOGrid::CaptureColumnSizes(GOAdditionalSizeKeeper &sizeKeeper) const {
  for (int l = GetNumberCols(), i = 0; i < l; i++)
    sizeKeeper.SetAdditionalSize(
      wxString::Format(WX_COL_SIZE_KEY_FMT, i), GetColSize(i));
}

wxGridCellRenderer *GOGrid::GetDefaultRendererForCell(int row, int col) const {
  wxGridCellRenderer *pRenderer = IsColumnRightVisible(col)
    ? p_RightVisibleRenderer
    : p_NormalVisibleRenderer;

  pRenderer->IncRef();
  return pRenderer;
}
