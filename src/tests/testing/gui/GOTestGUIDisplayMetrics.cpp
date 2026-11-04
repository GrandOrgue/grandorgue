/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestGUIDisplayMetrics.h"

#include <vector>

#include "gui/panels/GOGUIDisplayMetrics.h"

#include "GOTestScope.h"

const std::string GOTestGUIDisplayMetrics::TEST_NAME
  = "GOTestGUIDisplayMetrics";

void GOTestGUIDisplayMetrics::TestCellOffset() {
  const std::vector<unsigned> columnWidths = {52, 80, 52};
  const std::vector<unsigned> rowHeights = {63, 40};
  const auto getColumnWidth = [&](unsigned colI) { return columnWidths[colI]; };
  const auto getRowHeight = [&](unsigned rowI) { return rowHeights[rowI]; };

  GOAssert(
    GOGUIDisplayMetrics::computeCellOffset(0, getColumnWidth, 0) == 0,
    "The offset before the first cell should be 0");
  GOAssert(
    GOGUIDisplayMetrics::computeCellOffset(1, getColumnWidth, 0) == 52,
    "The offset of the second column should be the width of the first");
  GOAssert(
    GOGUIDisplayMetrics::computeCellOffset(2, getColumnWidth, 0) == 132,
    "The offset of the third column should be 52 + 80");
  GOAssert(
    GOGUIDisplayMetrics::computeCellOffset(3, getColumnWidth, 0) == 184,
    "The total width of the three columns should be 184");
  GOAssert(
    GOGUIDisplayMetrics::computeCellOffset(1, getRowHeight, 12) == 75,
    "The offset of the second row should be 63 + the gap 12");
  GOAssert(
    GOGUIDisplayMetrics::computeCellOffset(2, getRowHeight, 12) == 127,
    "The total height of the two rows with the gap after each should be "
    "63 + 12 + 40 + 12");
}

void GOTestGUIDisplayMetrics::run() { GO_RUN_TEST(TestCellOffset()) }
