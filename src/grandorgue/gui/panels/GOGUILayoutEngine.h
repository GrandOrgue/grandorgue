/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUILAYOUTENGINE_H
#define GOGUILAYOUTENGINE_H

#include <functional>
#include <utility>
#include <vector>

class GOGUIDisplayMetrics;
class GOGUIEnclosure;
class GOGUIManual;

class GOGUILayoutEngine {
public:
  /** The grids of the layout: a cell is addressed by a row and a column of a
   * grid */
  enum GridType { GRID_DRAWSTOP, GRID_PUSHBUTTON, GRID_ENCLOSURE };

  typedef struct {
    unsigned width;
    unsigned height;
    int y;
    int x;
    int keys_y;
    int piston_y;
  } MANUAL_RENDER_INFO;

private:
  friend class GOTestGUILayoutEngine; // tests the pure arithmetic below

  /**
   * @return the left (top) coordinate of an item centred in a cell
   * @param cellStart the left (top) coordinate of the cell
   * @param cellSize the width (height) of the cell
   * @param itemSize the width (height) of the item
   */
  static int computeCentredPosition(
    int cellStart, unsigned cellSize, unsigned itemSize);

  /**
   * @return the X of the left edge of the area of the enclosure columns:
   *   centred on the screen, shifted by 6 pixels
   */
  static int computeEnclosureAreaX(unsigned screenWidth, unsigned areaWidth);

  /**
   * The position of an item of the size width x height centred in a cell of a
   * grid with the given sizes of the columns and the rows. It needs neither
   * the metrics nor the screen.
   * @param areaX the X of the left edge of the grid
   * @param areaY the Y of the top edge of the grid
   * @param getColumnWidth the width of the column by its number from 1
   * @param getRowHeight the height of the row by its number from 1
   * @param rowGap the gap after each row
   * @param rowN the row of the cell from 1
   * @param colN the column of the cell from 1
   * @param width the width of the item
   * @param height the height of the item
   * @param outX the X of the left edge of the item
   * @param outY the Y of the top edge of the item
   */
  static void computeGridBlitPosition(
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
    int &outY);

  /**
   * Splits the drawstop columns into the left and the right jamb at the column
   * GetDrawstopBlitPosition() splits them, honouring the size of every column.
   * @param getColumnWidth the width of a column by its index from 0, as
   *   computeCellOffset() takes it
   * @param nCols the number of the drawstop columns to display
   * @param isPairDrawstopCols whether the pair-column layout is used: both
   *   jambs get the pair nudge (nCols >> 2) * (standardWidth / 4) - 8
   * @param standardWidth the standard width of a drawstop, for the nudge
   * @return the widths of the left and the right jamb
   */
  static std::pair<int, int> computeJambWidths(
    const std::function<unsigned(unsigned)> &getColumnWidth,
    unsigned nCols,
    bool isPairDrawstopCols,
    unsigned standardWidth);

  /**
   * The X of the jambs for the given widths. Each jamb is centred in its own
   * margin: the left one in 0 .. centerX, the right one in
   * centerX + centerWidth .. screen. With equal widths the result is the
   * legacy leftX, leftX + centerX + centerWidth.
   * @param isPairDrawstopCols whether both jambs get the pair shift exactly
   *   as in the legacy code: 5 to the left jamb, 5 to the right one
   * @return the X of the left and the right jamb
   */
  static std::pair<int, int> computeJambXs(
    int centerX,
    int centerWidth,
    int leftWidth,
    int rightWidth,
    bool isPairDrawstopCols);

protected:
  GOGUIDisplayMetrics &r_metrics;
  std::vector<MANUAL_RENDER_INFO> m_ManualRenderInfo;
  std::vector<GOGUIEnclosure *> m_Enclosures;
  std::vector<GOGUIManual *> m_Manuals;

  /* values computed on screen update */
  int m_HackY;
  int m_EnclosureY;
  int m_CenterY;
  int m_CenterWidth;

private:
  /**
   * The total width of nCols columns of the drawstop grid starting from
   * firstColN, as the metrics give it by the index of the column
   * @param firstColN the first column from 1
   */
  unsigned GetDrawstopColumnsWidth(unsigned firstColN, unsigned nCols);

  /**
   * The total height of nRows rows of the drawstop grid starting from
   * firstRowN. The row numbers are as in GetDrawstopBlitPosition
   */
  unsigned GetDrawstopRowsHeight(int firstRowN, unsigned nRows);

  /** The same for the pushbutton grid */
  unsigned GetButtonColumnsWidth(unsigned firstColN, unsigned nCols);
  unsigned GetButtonRowsHeight(int firstRowN, unsigned nRows);

  /** The same for the enclosure grid */
  unsigned GetEnclosureColumnsWidth(unsigned firstColN, unsigned nCols);

  /**
   * The same for the enclosure grid, but with ROW_GAP after each row
   * @param firstRowN the first row from 1
   */
  unsigned GetEnclosureRowsHeight(int firstRowN, unsigned nRows);

protected:
  /**
   * @return the width of the area of the enclosures: the sum of the widths of
   *   its columns, one per registered enclosure
   */
  unsigned GetEnclosureAreaWidth();

  /**
   * @return the height that Update() reserves for the area of the enclosures:
   *   the sum of the heights of its rows and ROW_GAP between them
   */
  unsigned GetEnclosureAreaHeight();

public:
  GOGUILayoutEngine(GOGUIDisplayMetrics &metrics);
  virtual ~GOGUILayoutEngine();

  /**
   * The position of an item in a cell of a grid: calls the method of the grid.
   * Buttons and labels call only this method.
   * @param gridType the grid of the cell
   * @param rowN the row of the cell (the meaning depends on the grid)
   * @param colN the column of the cell from 1
   * @param width the width of the item; used to centre it in an enclosure
   *   cell, ignored by the other grids, which place only items of their
   *   standard cell size
   * @param height the height of the item; used to centre it vertically in an
   *   enclosure cell, ignored by the other grids
   * @param outX the X of the left edge of the item
   * @param outY the Y of the top edge of the item
   */
  void GetBlitPosition(
    GridType gridType,
    int rowN,
    int colN,
    unsigned width,
    unsigned height,
    int &outX,
    int &outY);

  /**
   * The position of a cell of the drawstop grid (the jambs). The size of the
   * item is not passed, so the method gives the corner of the cell plus the
   * fixed offsets, which are right only for an item of the standard size of a
   * drawstop (the cell GetDrawstopWidth() x GetDrawstopHeight()); an item of
   * another size is not centred and not aligned. To place an item of any size
   * use GetBlitPosition().
   * @param drawstopRow the row: from 1, or from 100 for the extra rows above
   *   the manuals
   * @param drawstopCol the column from 1
   * @param blitX the X of the left edge of the cell
   * @param blitY the Y of the top edge of the cell
   */
  virtual void GetDrawstopBlitPosition(
    const int drawstopRow, const int drawstopCol, int &blitX, int &blitY);

  /**
   * The same for the grid of the pushbuttons (the pistons): valid only for an
   * item of the standard size of a button cell (GetButtonWidth() x
   * GetButtonHeight()).
   * @param buttonRow the row: the number of the manual, 99 for the pedal, from
   *   100 for the extra rows
   * @param buttonCol the column from 1
   * @param blitX the X of the left edge of the cell
   * @param blitY the Y of the top edge of the cell
   */
  virtual void GetPushbuttonBlitPosition(
    const int buttonRow, const int buttonCol, int &blitX, int &blitY);

  /**
   * The third grid method, next to GetDrawstopBlitPosition and
   * GetPushbuttonBlitPosition. The grid of the enclosure area: the rows from
   * the top (1 is the row of the enclosures), the columns from 1; the sizes of
   * the cells are those of the metrics. The area is centred on the screen, the
   * item is centred in its cell. Valid after Update().
   * @param rowN the row from 1
   * @param colN the column from 1
   * @param width the width of the item
   * @param height the height of the item
   * @param outX the X of the left edge of the item
   * @param outY the Y of the top edge of the item
   */
  void GetEnclosureBlitPosition(
    int rowN, int colN, unsigned width, unsigned height, int &outX, int &outY);

  virtual int GetEnclosureY();
  virtual int GetEnclosureX(const GOGUIEnclosure *enclosure);

  /**
   * @return the widths of the left and the right jamb, see computeJambWidths()
   */
  std::pair<int, int> GetJambWidths();
  /** @return the X of the left and the right jamb, see computeJambXs() */
  std::pair<int, int> GetJambXs();
  virtual unsigned GetJambLeftRightHeight();
  virtual int GetJambLeftRightY();
  virtual int GetJambLeftX() { return GetJambXs().first; }
  virtual int GetJambRightX() { return GetJambXs().second; }
  virtual int GetJambTopDrawstop();
  virtual int GetJambTopPiston();
  virtual unsigned GetJambTopHeight();
  virtual unsigned GetJambTopWidth();
  virtual int GetJambTopX();
  virtual int GetJambTopY();
  virtual unsigned GetPistonTopHeight();
  virtual unsigned GetPistonWidth();
  virtual int GetPistonX();
  virtual int GetCenterWidth();
  virtual int GetCenterY();
  virtual int GetCenterX();
  virtual int GetHackY();

  virtual const MANUAL_RENDER_INFO &GetManualRenderInfo(
    const unsigned manual_nb) const;

  void RegisterEnclosure(GOGUIEnclosure *enclosure);
  void RegisterManual(GOGUIManual *manual);
  unsigned GetManualNumber();

  virtual void Update();
};

#endif
