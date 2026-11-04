/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestGUILayoutEngine.h"

#include <format>
#include <vector>

#include "gui/panels/GOGUILayoutEngine.h"

#include "GOTestScope.h"

const std::string GOTestGUILayoutEngine::TEST_NAME = "GOTestGUILayoutEngine";

void GOTestGUILayoutEngine::TestGridBlitPosition() {
  // The sizes of PanelMetrics: the master column 80, the others 52; the
  // enclosure row 61, the row of the pistons 32; ROW_GAP 12
  const auto getColumnWidth
    = [](unsigned colN) { return colN == 1 ? 80u : 52u; };
  const auto getRowHeight = [](unsigned rowN) { return rowN == 1 ? 61u : 32u; };
  const int areaX = 200;
  const int areaY = 50;
  int enclosureX = 0;
  int enclosureY = 0;
  int pistonX = 0;
  int pistonY = 0;
  int labelX = 0;
  int labelY = 0;

  GOGUILayoutEngine::computeGridBlitPosition(
    areaX,
    areaY,
    getColumnWidth,
    getRowHeight,
    12,
    1,
    2,
    46,
    61,
    enclosureX,
    enclosureY);
  GOGUILayoutEngine::computeGridBlitPosition(
    areaX,
    areaY,
    getColumnWidth,
    getRowHeight,
    12,
    2,
    2,
    32,
    32,
    pistonX,
    pistonY);
  GOGUILayoutEngine::computeGridBlitPosition(
    areaX,
    areaY,
    getColumnWidth,
    getRowHeight,
    12,
    2,
    1,
    80,
    25,
    labelX,
    labelY);

  GOAssert(
    pistonX + 16 == enclosureX + 23,
    std::format(
      "The centre of the piston ({}) should equal the centre of the "
      "enclosure ({}) in the same column",
      pistonX + 16,
      enclosureX + 23));
  GOAssert(
    pistonY == areaY + 61 + 12,
    std::format(
      "The piston Y should be {}, got: {}", areaY + 61 + 12, pistonY));
  GOAssert(
    labelX == areaX,
    std::format("The label X should be {}, got: {}", areaX, labelX));
  GOAssert(
    labelY == areaY + 61 + 12 + 4,
    std::format(
      "The label should be centred in the row of 32 (16 - 12 = 4): Y {}, "
      "got: {}",
      areaY + 61 + 12 + 4,
      labelY));
}

void GOTestGUILayoutEngine::TestCentredPosition() {
  GOAssert(
    GOGUILayoutEngine::computeCentredPosition(100, 52, 52) == 100,
    "An item that fills its cell should be at the start of the cell");
  GOAssert(
    GOGUILayoutEngine::computeCentredPosition(100, 52, 32) == 110,
    "A 32 px item in a 52 px cell should be shifted by 10");
  GOAssert(
    GOGUILayoutEngine::computeCentredPosition(100, 80, 52) == 114,
    "A 52 px item in an 80 px cell should be shifted by 14");
  GOAssert(
    GOGUILayoutEngine::computeCentredPosition(100, 80, 80) == 100,
    "An 80 px label in an 80 px cell should be at the start of the cell");
  GOAssert(
    GOGUILayoutEngine::computeCentredPosition(100, 40, 63) == 89,
    "An item that is higher than its cell should be shifted up");
}

void GOTestGUILayoutEngine::TestEnclosureAreaX() {
  GOAssert(
    GOGUILayoutEngine::computeEnclosureAreaX(600, 156) == 225,
    "The area should be centred on the screen with the shift of 6");

  const int narrowX = GOGUILayoutEngine::computeEnclosureAreaX(100, 156);

  GOAssert(
    narrowX < 0,
    std::format(
      "The area that is wider than the screen should start at a negative X, "
      "got: {}",
      narrowX));
}

void GOTestGUILayoutEngine::TestJambWidths() {
  const std::vector<unsigned> widths = {240, 78, 78, 78};
  const auto getWidth = [&widths](unsigned colI) { return widths[colI]; };

  // the split at the column nCols / 2, not the half of the sum
  const auto [leftWidth, rightWidth]
    = GOGUILayoutEngine::computeJambWidths(getWidth, 4, false, 78);

  GOAssert(
    leftWidth == 318,
    std::format("The left jamb should be 240 + 78 = 318, got: {}", leftWidth));
  GOAssert(
    rightWidth == 156,
    std::format("The right jamb should be 78 + 78 = 156, got: {}", rightWidth));

  // equal columns split evenly, and the pair layout nudges both jambs
  const auto [evenLeftWidth, evenRightWidth]
    = GOGUILayoutEngine::computeJambWidths(
      [](unsigned) { return 78u; }, 4, false, 78);

  GOAssert(
    evenLeftWidth == 156 && evenRightWidth == 156,
    std::format(
      "Equal columns should split evenly, got: {} and {}",
      evenLeftWidth,
      evenRightWidth));

  const auto [pairLeftWidth, pairRightWidth]
    = GOGUILayoutEngine::computeJambWidths(
      [](unsigned) { return 78u; }, 4, true, 78);

  GOAssert(
    pairLeftWidth == 167 && pairRightWidth == 167,
    std::format(
      "The pair nudge (4 >> 2) * (78 / 4) - 8 = 11 should be added to both "
      "jambs, got: {} and {}",
      pairLeftWidth,
      pairRightWidth));
}

void GOTestGUILayoutEngine::TestJambXs() {
  // equal jambs: the legacy positions exactly
  const auto [evenLeftX, evenRightX]
    = GOGUILayoutEngine::computeJambXs(350, 400, 78, 78, false);

  GOAssert(
    evenLeftX == 136,
    std::format(
      "The left jamb should be centred in 0 .. 350, got: {}", evenLeftX));
  GOAssert(
    evenRightX == 886,
    std::format(
      "The right jamb should keep the legacy position, got: {}", evenRightX));

  /* Unequal jambs (the columns 240, 78, 78, 78): the averaged width 237 put
   * the left jamb at 56, 24 px into the centre area; it must end before it */
  const auto [leftX, rightX]
    = GOGUILayoutEngine::computeJambXs(350, 400, 318, 156, false);

  GOAssert(
    leftX == 16,
    std::format("The left jamb should be centred in 0 .. 350, got: {}", leftX));
  GOAssert(
    leftX + 318 <= 350,
    std::format("The left jamb should not reach the centre, got: {}", leftX));
  GOAssert(
    rightX == 847,
    std::format(
      "The right jamb should be centred in the right margin, got: {}", rightX));

  const auto [pairLeftX, pairRightX]
    = GOGUILayoutEngine::computeJambXs(350, 400, 318, 156, true);

  GOAssert(
    pairLeftX == 21 && pairRightX == 857,
    std::format(
      "The pair layout should shift the left jamb by 5 and the right one by "
      "10, got: {} and {}",
      pairLeftX,
      pairRightX));
}

void GOTestGUILayoutEngine::run() {
  GO_RUN_TEST(TestGridBlitPosition())
  GO_RUN_TEST(TestCentredPosition())
  GO_RUN_TEST(TestEnclosureAreaX())
  GO_RUN_TEST(TestJambWidths())
  GO_RUN_TEST(TestJambXs())
}
