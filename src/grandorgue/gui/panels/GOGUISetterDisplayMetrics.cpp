/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGUISetterDisplayMetrics.h"

#include <algorithm>

#include "config/GOConfigReader.h"
#include "model/GOManual.h"

#include "GOOrganController.h"

GOGUISetterDisplayMetrics::GOGUISetterDisplayMetrics(
  GOConfigReader &ini, GOOrganController *organController, GOGUISetterType type)
  : GOGUIDisplayMetrics() {
  unsigned x_size, y_size;
  int drawstop_rows, drawstop_cols;
  int button_rows, button_cols;

  switch (type) {
  case GOGUI_SETTER_SETTER:
    x_size = 800;
    y_size = 360;
    drawstop_rows = 4;
    drawstop_cols = 10;
    button_cols = 10;
    button_rows = 0;
    break;

  case GOGUI_SETTER_CRESCENDO:
    x_size = 700;
    y_size = 300;
    drawstop_rows = 2;
    drawstop_cols = 8;
    button_cols = 10;
    button_rows = 0;
    break;

  case GOGUI_SETTER_DIVISIONALS:
    x_size = 720;
    y_size = 40 * organController->GetODFManualCount() + 140;
    drawstop_rows = 1;
    drawstop_cols = 7;
    button_cols = 16;
    button_rows = organController->GetODFManualCount();
    break;

  case GOGUI_SETTER_COUPLER:
    x_size = 600;
    y_size = 20 + 80 * organController->GetODFManualCount();
    drawstop_rows = organController->GetODFManualCount();
    drawstop_cols = 7;
    button_cols = 0;
    button_rows = 0;
    break;

  case GOGUI_SETTER_FLOATING:
    x_size
      = 40
      + std::max(
          10
            * organController->GetManual(organController->GetODFManualCount())
                ->GetLogicalKeyCount(),
          (unsigned)10 * 40);
    y_size = (organController->GetManualAndPedalCount()
              - organController->GetODFManualCount() + 1)
        * 60
      + 160;
    drawstop_rows = 0;
    drawstop_cols = 0;
    button_cols = 10;
    button_rows = 0;
    break;

  case GOGUI_SETTER_MASTER:
    x_size = 550;
    y_size = 300;
    drawstop_rows = 3;
    drawstop_cols = 7;
    button_cols = 10;
    button_rows = 1;
    break;

  case GOGUI_METRONOME:
    x_size = 400;
    y_size = 200;
    drawstop_rows = 2;
    drawstop_cols = 5;
    button_cols = 10;
    button_rows = 0;
    break;

  case GOGUI_RECORDER:
    x_size = 400;
    y_size = 270;
    drawstop_rows = 3;
    drawstop_cols = 5;
    button_cols = 10;
    button_rows = 0;
    break;

  case GOGUI_SETTER_GENERALS:
  default:
    x_size = 600;
    y_size = 400;
    drawstop_rows = 2;
    drawstop_cols = 7;
    button_cols = 10;
    button_rows = 5;
  }

  int background_img = ini.ReadInteger(
    ODFSetting,
    wxT("Organ"),
    wxT("DispConsoleBackgroundImageNum"),
    1,
    64,
    false,
    32);
  int horiz_img = ini.ReadInteger(
    ODFSetting,
    wxT("Organ"),
    wxT("DispKeyHorizBackgroundImageNum"),
    1,
    64,
    false,
    22);
  int vert_img = ini.ReadInteger(
    ODFSetting,
    wxT("Organ"),
    wxT("DispKeyVertBackgroundImageNum"),
    1,
    64,
    false,
    19);

  m_DispScreenSizeHoriz = x_size;
  m_DispScreenSizeVert = y_size;
  m_DispDrawstopBackgroundImageNum = background_img;
  m_DispConsoleBackgroundImageNum = background_img;
  m_DispKeyHorizBackgroundImageNum
    = type == GOGUI_SETTER_FLOATING ? horiz_img : background_img;
  m_DispKeyVertBackgroundImageNum
    = type == GOGUI_SETTER_FLOATING ? vert_img : background_img;
  m_DispDrawstopInsetBackgroundImageNum = background_img;
  m_DispControlLabelFont = wxT("Times New Roman");
  m_DispShortcutKeyLabelFont = wxT("Times New Roman");
  m_DispShortcutKeyLabelColour = wxColour(0xFF, 0xFF, 0x00);
  m_DispGroupLabelFont = wxT("Times New Roman");
  m_DispDrawstopCols = 0;
  m_DispDrawstopRows = 0;
  m_DispDrawstopColsOffset = false;
  m_DispDrawstopOuterColOffsetUp = false;
  m_DispPairDrawstopCols = false;
  m_DispExtraDrawstopRows = drawstop_rows;
  m_DispExtraDrawstopCols = drawstop_cols;
  m_DispButtonCols = button_cols;
  m_DispExtraButtonRows = button_rows;
  m_DispExtraPedalButtonRow = false;
  m_DispExtraPedalButtonRowOffset = false;
  m_DispExtraPedalButtonRowOffsetRight = false;
  m_DispButtonsAboveManuals = true;
  m_DispTrimAboveManuals = true;
  m_DispTrimBelowManuals = true;
  m_DispTrimAboveExtraRows = true;
  m_DispExtraDrawstopRowsAboveExtraButtonRows = true;

  Init();
}
