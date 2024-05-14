/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUIDISPLAYMETRICS_H
#define GOGUIDISPLAYMETRICS_H

#include <wx/colour.h>
#include <wx/string.h>

#include "primitives/GOFont.h"

class GOGUIDisplayMetrics {
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
  unsigned GetManualKeyWidth();
  unsigned GetManualHeight();
  unsigned GetPedalKeyWidth();
  unsigned GetPedalHeight();
};

#endif
