/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUIDISPLAYMETRICS_H
#define GOGUIDISPLAYMETRICS_H

#include <functional>

#include <wx/colour.h>
#include <wx/string.h>

#include "primitives/GOFont.h"

class GOGUIDisplayMetrics {
public:
  /** The vertical gap between two adjacent rows of the layout, in pixels */
  static constexpr unsigned ROW_GAP = 12;

  /**
   * @return the offset of the cell number nCells from the start of a line of
   *   cells: the sum of the sizes of the first nCells cells plus gap after each
   *   of them
   * @param nCells the number of the cells before the wanted position
   * @param getCellSize the size of the cell by its index from 0
   * @param gap the gap after each cell: 0 for the columns, ROW_GAP for the rows
   */
  static unsigned computeCellOffset(
    unsigned nCells,
    const std::function<unsigned(unsigned)> &getCellSize,
    unsigned gap);

protected:
  /* Values loaded from ODF */
  unsigned m_DispScreenSizeHoriz;
  unsigned m_DispScreenSizeVert;
  unsigned m_DispDrawstopBackgroundImageNum;
  unsigned m_DispConsoleBackgroundImageNum;
  unsigned m_DispKeyHorizBackgroundImageNum;
  unsigned m_DispKeyVertBackgroundImageNum;
  unsigned m_DispDrawstopInsetBackgroundImageNum;
  wxString m_DispControlLabelFont;
  wxString m_DispShortcutKeyLabelFont;
  wxColour m_DispShortcutKeyLabelColour;
  wxString m_DispGroupLabelFont;
  unsigned m_DispDrawstopCols;
  unsigned m_DispDrawstopRows;
  bool m_DispDrawstopColsOffset;
  bool m_DispDrawstopOuterColOffsetUp;
  bool m_DispPairDrawstopCols;
  unsigned m_DispExtraDrawstopRows;
  unsigned m_DispExtraDrawstopCols;
  unsigned m_DispButtonCols;
  unsigned m_DispExtraButtonRows;
  bool m_DispExtraPedalButtonRow;
  bool m_DispExtraPedalButtonRowOffset;
  bool m_DispExtraPedalButtonRowOffsetRight;
  bool m_DispButtonsAboveManuals;
  bool m_DispTrimAboveManuals;
  bool m_DispTrimBelowManuals;
  bool m_DispTrimAboveExtraRows;
  bool m_DispExtraDrawstopRowsAboveExtraButtonRows;
  unsigned m_DrawStopWidth;
  unsigned m_DrawStopHeight;
  unsigned m_ButtonWidth;
  unsigned m_ButtonHeight;
  unsigned m_EnclosureWidth;
  unsigned m_EnclosureHeight;
  unsigned m_PedalHeight;
  unsigned m_PedalKeyWidth;
  unsigned m_ManualHeight;
  unsigned m_ManualKeyWidth;
  GOFont m_GroupLabelFont;
  GOFont m_ControlLabelFont;

  void Init();

public:
  GOGUIDisplayMetrics();
  virtual ~GOGUIDisplayMetrics();

  unsigned NumberOfExtraDrawstopRowsToDisplay();
  unsigned NumberOfExtraDrawstopColsToDisplay();
  unsigned NumberOfDrawstopRowsToDisplay();
  unsigned NumberOfDrawstopColsToDisplay();
  unsigned NumberOfButtonCols();
  unsigned NumberOfExtraButtonRows();

  GOFont GetControlLabelFont();
  GOFont GetGroupLabelFont();

  unsigned GetScreenWidth();
  unsigned GetScreenHeight();

  unsigned GetDrawstopBackgroundImageNum();
  unsigned GetConsoleBackgroundImageNum();
  unsigned GetDrawstopInsetBackgroundImageNum();
  unsigned GetKeyVertBackgroundImageNum();
  unsigned GetKeyHorizBackgroundImageNum();

  bool HasPairDrawstopCols();
  bool HasTrimAboveExtraRows();
  bool HasTrimAboveManuals();
  bool HasTrimBelowManuals();
  bool HasExtraPedalButtonRow();
  bool HasButtonsAboveManuals();
  bool HasExtraPedalButtonRowOffset();
  bool HasExtraDrawstopRowsAboveExtraButtonRows();
  bool HasDrawstopColsOffset();
  bool HasDrawstopOuterColOffsetUp();

  unsigned GetDrawstopWidth();
  unsigned GetDrawstopHeight();
  unsigned GetButtonWidth();
  unsigned GetButtonHeight();
  unsigned GetEnclosureWidth();
  unsigned GetEnclosureHeight();

  /**
   * The sizes of the cells of the grids by the index. The base gives the same
   * size for every column and every row; a derived metrics may give each its
   * own. The layout engine honours the size of every index: it sums them to
   * get the coordinates and uses the size of a row or a column for the offsets
   * within it.
   * @param colN the column from 1
   * @param rowN the row, numbered as in the grid: from 1 (the drawstop and
   *   the enclosure grids), the manual number for the pushbutton grid (99 for
   *   the pedal), from 100 for the extra rows of the drawstop and the
   *   pushbutton grids
   * @note the height of the drawstop jamb sums the rows up to
   *   NumberOfDrawstopRowsToDisplay() + 1; the last one is the spare row that
   *   centres the jamb, so a derived metrics should give it a height
   */
  virtual unsigned GetDrawstopColumnWidth(unsigned colN) {
    return GetDrawstopWidth();
  }
  virtual unsigned GetDrawstopRowHeight(int rowN) {
    return GetDrawstopHeight();
  }
  virtual unsigned GetButtonColumnWidth(unsigned colN) {
    return GetButtonWidth();
  }
  virtual unsigned GetButtonRowHeight(int rowN) { return GetButtonHeight(); }
  virtual unsigned GetEnclosureColumnWidth(unsigned colN) {
    return GetEnclosureWidth();
  }
  virtual unsigned GetEnclosureRowHeight(int rowN) {
    return GetEnclosureHeight();
  }

  /**
   * @return the number of the rows of the enclosure grid: the enclosures and,
   *   in a derived metrics, the rows below them. The base has one row
   */
  virtual unsigned GetNEnclosureRows() { return 1; }

  unsigned GetManualKeyWidth();
  unsigned GetManualHeight();
  unsigned GetPedalKeyWidth();
  unsigned GetPedalHeight();
};

#endif
