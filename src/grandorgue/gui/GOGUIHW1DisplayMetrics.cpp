/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGUIHW1DisplayMetrics.h"

#include "config/GOConfigReader.h"
#include "primitives/go_gui_utils.h"

GOGUIHW1DisplayMetrics::GOGUIHW1DisplayMetrics(
  GOConfigReader &ini, wxString group)
  : GOGUIDisplayMetrics() {
  if (group.IsEmpty())
    group = wxT("Organ");

  m_DispScreenSizeHoriz
    = ini.ReadSize(ODFSetting, group, wxT("DispScreenSizeHoriz"), 0);
  m_DispScreenSizeVert
    = ini.ReadSize(ODFSetting, group, wxT("DispScreenSizeVert"), 1);
  m_DispDrawstopBackgroundImageNum = ini.ReadInteger(
    ODFSetting, group, wxT("DispDrawstopBackgroundImageNum"), 1, 64);
  m_DispConsoleBackgroundImageNum = ini.ReadInteger(
    ODFSetting, group, wxT("DispConsoleBackgroundImageNum"), 1, 64);
  m_DispKeyHorizBackgroundImageNum = ini.ReadInteger(
    ODFSetting, group, wxT("DispKeyHorizBackgroundImageNum"), 1, 64);
  m_DispKeyVertBackgroundImageNum = ini.ReadInteger(
    ODFSetting, group, wxT("DispKeyVertBackgroundImageNum"), 1, 64);
  m_DispDrawstopInsetBackgroundImageNum = ini.ReadInteger(
    ODFSetting, group, wxT("DispDrawstopInsetBackgroundImageNum"), 1, 64);
  m_DispControlLabelFont
    = ini.ReadStringTrim(ODFSetting, group, wxT("DispControlLabelFont"));
  m_DispShortcutKeyLabelFont
    = ini.ReadStringTrim(ODFSetting, group, wxT("DispShortcutKeyLabelFont"));
  m_DispShortcutKeyLabelColour = logicalToWxColour(
    ini.ReadColor(ODFSetting, group, wxT("DispShortcutKeyLabelColour")));
  m_DispGroupLabelFont
    = ini.ReadStringTrim(ODFSetting, group, wxT("DispGroupLabelFont"));
  m_DispDrawstopCols
    = ini.ReadInteger(ODFSetting, group, wxT("DispDrawstopCols"), 2, 12);
  m_DispDrawstopRows
    = ini.ReadInteger(ODFSetting, group, wxT("DispDrawstopRows"), 1, 20);
  m_DispDrawstopColsOffset
    = ini.ReadBoolean(ODFSetting, group, wxT("DispDrawstopColsOffset"));
  m_DispDrawstopOuterColOffsetUp = ini.ReadBoolean(
    ODFSetting,
    group,
    wxT("DispDrawstopOuterColOffsetUp"),
    m_DispDrawstopColsOffset);
  m_DispPairDrawstopCols
    = ini.ReadBoolean(ODFSetting, group, wxT("DispPairDrawstopCols"));
  m_DispExtraDrawstopRows
    = ini.ReadInteger(ODFSetting, group, wxT("DispExtraDrawstopRows"), 0, 99);
  m_DispExtraDrawstopCols
    = ini.ReadInteger(ODFSetting, group, wxT("DispExtraDrawstopCols"), 0, 40);
  m_DispButtonCols
    = ini.ReadInteger(ODFSetting, group, wxT("DispButtonCols"), 1, 32);
  m_DispExtraButtonRows
    = ini.ReadInteger(ODFSetting, group, wxT("DispExtraButtonRows"), 0, 99);
  m_DispExtraPedalButtonRow
    = ini.ReadBoolean(ODFSetting, group, wxT("DispExtraPedalButtonRow"));
  m_DispExtraPedalButtonRowOffset = ini.ReadBoolean(
    ODFSetting,
    group,
    wxT("DispExtraPedalButtonRowOffset"),
    m_DispExtraPedalButtonRow);
  m_DispExtraPedalButtonRowOffsetRight = ini.ReadBoolean(
    ODFSetting,
    group,
    wxT("DispExtraPedalButtonRowOffsetRight"),
    m_DispExtraPedalButtonRow);
  m_DispButtonsAboveManuals
    = ini.ReadBoolean(ODFSetting, group, wxT("DispButtonsAboveManuals"));
  m_DispTrimAboveManuals
    = ini.ReadBoolean(ODFSetting, group, wxT("DispTrimAboveManuals"));
  m_DispTrimBelowManuals
    = ini.ReadBoolean(ODFSetting, group, wxT("DispTrimBelowManuals"));
  m_DispTrimAboveExtraRows
    = ini.ReadBoolean(ODFSetting, group, wxT("DispTrimAboveExtraRows"));
  m_DispExtraDrawstopRowsAboveExtraButtonRows = ini.ReadBoolean(
    ODFSetting, group, wxT("DispExtraDrawstopRowsAboveExtraButtonRows"));
  m_DrawStopWidth = ini.ReadInteger(
    ODFSetting, group, wxT("DispDrawstopWidth"), 1, 150, false, 78),
  m_DrawStopHeight = ini.ReadInteger(
    ODFSetting, group, wxT("DispDrawstopHeight"), 1, 150, false, 69),
  m_ButtonWidth = ini.ReadInteger(
    ODFSetting, group, wxT("DispPistonWidth"), 1, 150, false, 44),
  m_ButtonHeight = ini.ReadInteger(
    ODFSetting, group, wxT("DispPistonHeight"), 1, 150, false, 40),
  m_EnclosureWidth = ini.ReadInteger(
    ODFSetting, group, wxT("DispEnclosureWidth"), 1, 150, false, 52),
  m_EnclosureHeight = ini.ReadInteger(
    ODFSetting, group, wxT("DispEnclosureHeight"), 1, 150, false, 63),
  m_PedalHeight = ini.ReadInteger(
    ODFSetting, group, wxT("DispPedalHeight"), 1, 500, false, 40),
  m_PedalKeyWidth = ini.ReadInteger(
    ODFSetting, group, wxT("DispPedalKeyWidth"), 1, 500, false, 7),
  m_ManualHeight = ini.ReadInteger(
    ODFSetting, group, wxT("DispManualHeight"), 1, 500, false, 32),
  m_ManualKeyWidth = ini.ReadInteger(
    ODFSetting, group, wxT("DispManualKeyWidth"), 1, 500, false, 12),

  Init();
}
