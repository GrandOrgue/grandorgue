/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGUIButton.h"

#include <wx/intl.h>

#include "config/GOConfigReader.h"
#include "control/GOButtonControl.h"
#include "primitives/GODC.h"
#include "primitives/go_gui_utils.h"

#include "GOGUIDisplayMetrics.h"
#include "GOGUILayoutEngine.h"
#include "GOGUIMouseState.h"
#include "GOGUIPanel.h"

GOGUIButton::GOGUIButton(
  GOGUIPanel *panel, GOButtonControl *control, bool is_piston)
  : GOGUIControl(panel, control),
    m_IsPiston(is_piston),
    m_DispKeyLabelOnLeft(true),
    m_ButtonControl(control),
    m_MouseRect(),
    m_Radius(0),
    m_FontSize(0),
    m_FontName(),
    m_TextColor(0, 0, 0),
    m_Text(),
    m_TextRect(),
    m_TextWidth(0),
    m_DispCol(0),
    m_DispRow(0),
    m_TileOffsetX(0),
    m_TileOffsetY(0) {
  assert(control);
}

void GOGUIButton::Init(
  GOConfigReader &cfg,
  wxString group,
  unsigned x_pos,
  unsigned y_pos,
  unsigned image) {
  GOGUIControl::Init(cfg, group);

  m_DispCol = x_pos;
  m_DispRow = y_pos;

  m_TextColor = wxColour(0x80, 0x00, 0x00);
  m_FontSize = 9;
  m_FontName = wxT("");
  m_Text = m_ButtonControl->GetName();

  int x, y, w, h;

  wxString off_mask_file, on_mask_file;
  wxString on_file, off_file;
  if (m_IsPiston) {
    int DispImageNum = image ? image : m_ButtonControl->IsReadOnly() ? 3 : 1;
    off_file
      = wxString::Format(wxT(GOBitmapPrefix "piston%02d_off"), DispImageNum);
    on_file
      = wxString::Format(wxT(GOBitmapPrefix "piston%02d_on"), DispImageNum);
  } else {
    int DispImageNum = image ? image : m_ButtonControl->IsReadOnly() ? 4 : 1;
    off_file
      = wxString::Format(wxT(GOBitmapPrefix "drawstop%02d_off"), DispImageNum);
    on_file
      = wxString::Format(wxT(GOBitmapPrefix "drawstop%02d_on"), DispImageNum);
  }

  on_mask_file = wxEmptyString;
  off_mask_file = on_mask_file;

  m_OnBitmap = m_panel->LoadBitmap(on_file, on_mask_file);
  m_OffBitmap = m_panel->LoadBitmap(off_file, off_mask_file);

  x = -1;
  y = -1;
  w = m_OnBitmap.GetWidth();
  h = m_OnBitmap.GetHeight();
  m_BoundingRect = wxRect(x, y, w, h);

  if (
    m_OnBitmap.GetWidth() != m_OffBitmap.GetWidth()
    || m_OnBitmap.GetHeight() != m_OffBitmap.GetHeight())
    throw wxString::Format(
      _("bitmap size does not match for '%s'"), group.c_str());

  m_TileOffsetX = 0;
  m_TileOffsetY = 0;

  x = 0;
  y = 0;
  w = m_BoundingRect.GetWidth() - x;
  h = m_BoundingRect.GetHeight() - y;
  m_MouseRect = wxRect(x, y, w, h);
  m_Radius = std::min(w / 2, h / 2);

  x = 1;
  y = 1;
  w = m_BoundingRect.GetWidth() - x;
  h = m_BoundingRect.GetHeight() - y;
  m_TextRect = wxRect(x, y, w, h);
  m_TextWidth = m_TextRect.GetWidth() - (m_TextRect.GetWidth() < 50 ? 4 : 14);

  m_Font = m_metrics->GetControlLabelFont();
  m_Font.SetName(m_FontName);
  m_Font.SetPoints(m_FontSize);
}

void GOGUIButton::Load(GOConfigReader &cfg, wxString group) {
  GOGUIControl::Load(cfg, group);
  m_IsPiston = cfg.ReadBoolean(
    ODFSetting, group, wxT("DisplayAsPiston"), false, m_IsPiston);

  m_TextColor = logicalToWxColour(cfg.ReadColor(
    ODFSetting, group, wxT("DispLabelColour"), false, wxT("Dark Red")));
  m_FontSize = cfg.ReadFontSize(
    ODFSetting, group, wxT("DispLabelFontSize"), false, wxT("normal"));
  m_FontName = cfg.ReadStringTrim(
    ODFSetting, group, wxT("DispLabelFontName"), false, wxT(""));
  m_Text = cfg.ReadString(
    ODFSetting, group, wxT("DispLabelText"), false, m_ButtonControl->GetName());

  int x, y, w, h;

  m_DispKeyLabelOnLeft = cfg.ReadBoolean(
    ODFSetting, group, wxT("DispKeyLabelOnLeft"), false, true);

  wxString off_mask_file, on_mask_file;
  wxString on_file, off_file;
  if (m_IsPiston) {
    int DispImageNum = cfg.ReadInteger(
      ODFSetting,
      group,
      wxT("DispImageNum"),
      1,
      5,
      false,
      m_ButtonControl->IsReadOnly() ? 3 : 1);
    off_file
      = wxString::Format(wxT(GOBitmapPrefix "piston%02d_off"), DispImageNum);
    on_file
      = wxString::Format(wxT(GOBitmapPrefix "piston%02d_on"), DispImageNum);

    m_DispRow = cfg.ReadInteger(
      ODFSetting,
      group,
      wxT("DispButtonRow"),
      0,
      99 + m_metrics->NumberOfExtraButtonRows(),
      false,
      1);
    m_DispCol = cfg.ReadInteger(
      ODFSetting,
      group,
      wxT("DispButtonCol"),
      1,
      m_metrics->NumberOfButtonCols(),
      false,
      1);
  } else {
    int DispImageNum = cfg.ReadInteger(
      ODFSetting,
      group,
      wxT("DispImageNum"),
      1,
      7,
      false,
      m_ButtonControl->IsReadOnly() ? 4 : 1);
    off_file
      = wxString::Format(wxT(GOBitmapPrefix "drawstop%02d_off"), DispImageNum);
    on_file
      = wxString::Format(wxT(GOBitmapPrefix "drawstop%02d_on"), DispImageNum);

    m_DispRow = cfg.ReadInteger(
      ODFSetting,
      group,
      wxT("DispDrawstopRow"),
      1,
      99 + m_metrics->NumberOfExtraDrawstopRowsToDisplay(),
      false,
      1);
    m_DispCol = cfg.ReadInteger(
      ODFSetting,
      group,
      wxT("DispDrawstopCol"),
      1,
      m_DispRow > 99 ? m_metrics->NumberOfExtraDrawstopColsToDisplay()
                     : m_metrics->NumberOfDrawstopColsToDisplay(),
      false,
      1);
  }

  on_file
    = cfg.ReadStringTrim(ODFSetting, group, wxT("ImageOn"), false, on_file);
  off_file
    = cfg.ReadStringTrim(ODFSetting, group, wxT("ImageOff"), false, off_file);
  on_mask_file = cfg.ReadStringTrim(
    ODFSetting, group, wxT("MaskOn"), false, wxEmptyString);
  off_mask_file = cfg.ReadStringTrim(
    ODFSetting, group, wxT("MaskOff"), false, on_mask_file);

  m_OnBitmap = m_panel->LoadBitmap(on_file, on_mask_file);
  m_OffBitmap = m_panel->LoadBitmap(off_file, off_mask_file);

  x = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("PositionX"),
    0,
    m_metrics->GetScreenWidth(),
    false,
    -1);
  y = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("PositionY"),
    0,
    m_metrics->GetScreenHeight(),
    false,
    -1);
  w = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("Width"),
    1,
    m_metrics->GetScreenWidth(),
    false,
    m_OnBitmap.GetWidth());
  h = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("Height"),
    1,
    m_metrics->GetScreenHeight(),
    false,
    m_OnBitmap.GetHeight());
  m_BoundingRect = wxRect(x, y, w, h);

  if (
    m_OnBitmap.GetWidth() != m_OffBitmap.GetWidth()
    || m_OnBitmap.GetHeight() != m_OffBitmap.GetHeight())
    throw wxString::Format(
      _("bitmap size does not match for '%s'"), group.c_str());

  m_TileOffsetX = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("TileOffsetX"),
    0,
    m_OnBitmap.GetWidth() - 1,
    false,
    0);
  m_TileOffsetY = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("TileOffsetY"),
    0,
    m_OnBitmap.GetHeight() - 1,
    false,
    0);

  x = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("MouseRectLeft"),
    0,
    m_BoundingRect.GetWidth() - 1,
    false,
    0);
  y = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("MouseRectTop"),
    0,
    m_BoundingRect.GetHeight() - 1,
    false,
    0);
  w = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("MouseRectWidth"),
    1,
    m_BoundingRect.GetWidth() - x,
    false,
    m_BoundingRect.GetWidth() - x);
  h = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("MouseRectHeight"),
    1,
    m_BoundingRect.GetHeight() - y,
    false,
    m_BoundingRect.GetHeight() - y);
  m_MouseRect = wxRect(x, y, w, h);
  m_Radius = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("MouseRadius"),
    0,
    std::max(m_MouseRect.GetWidth(), m_MouseRect.GetHeight()),
    false,
    std::min(w / 2, h / 2));

  x = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("TextRectLeft"),
    0,
    m_BoundingRect.GetWidth() - 1,
    false,
    1);
  y = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("TextRectTop"),
    0,
    m_BoundingRect.GetHeight() - 1,
    false,
    1);
  w = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("TextRectWidth"),
    1,
    m_BoundingRect.GetWidth() - x,
    false,
    m_BoundingRect.GetWidth() - x);
  h = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("TextRectHeight"),
    1,
    m_BoundingRect.GetHeight() - y,
    false,
    m_BoundingRect.GetHeight() - y);
  m_TextRect = wxRect(x, y, w, h);
  m_TextWidth = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("TextBreakWidth"),
    0,
    m_TextRect.GetWidth(),
    false,
    m_TextRect.GetWidth() - (m_TextRect.GetWidth() < 50 ? 4 : 14));

  m_Font = m_metrics->GetControlLabelFont();
  m_Font.SetName(m_FontName);
  m_Font.SetPoints(m_FontSize);
}

void GOGUIButton::Layout() {
  int x, y;
  if (m_IsPiston) {
    m_layout->GetPushbuttonBlitPosition(m_DispRow, m_DispCol, x, y);
    if (!m_DispKeyLabelOnLeft)
      x -= 13;
  } else {
    m_layout->GetDrawstopBlitPosition(m_DispRow, m_DispCol, x, y);
  }

  if (m_BoundingRect.GetX() == -1)
    m_BoundingRect.SetX(x);
  if (m_BoundingRect.GetY() == -1)
    m_BoundingRect.SetY(y);

  m_TextRect.Offset(m_BoundingRect.GetX(), m_BoundingRect.GetY());
  m_MouseRect.Offset(m_BoundingRect.GetX(), m_BoundingRect.GetY());
}

bool GOGUIButton::HandleMousePress(
  int x, int y, bool right, GOGUIMouseState &state) {
  if (!m_MouseRect.Contains(x, y))
    return false;
  if (m_Radius) {
    if (
      (m_MouseRect.GetX() + m_Radius - x) * (m_MouseRect.GetX() + m_Radius - x)
        + (m_MouseRect.GetY() + m_Radius - y)
          * (m_MouseRect.GetY() + m_Radius - y)
      > m_Radius * m_Radius)
      return false;
  }
  if (right) {
    m_ButtonControl->ShowConfigDialog();
    return true;
  } else {
    if (state.GetControl() == this)
      return true;
    state.SetControl(this);
    if (m_ButtonControl->IsReadOnly())
      return true;

    m_ButtonControl->Push();
    return true;
  }
}

void GOGUIButton::PrepareDraw(double scale, GOBitmap *background) {
  m_OnBitmap.PrepareTileBitmap(
    scale, m_BoundingRect, m_TileOffsetX, m_TileOffsetY, background);
  m_OffBitmap.PrepareTileBitmap(
    scale, m_BoundingRect, m_TileOffsetX, m_TileOffsetY, background);
}

void GOGUIButton::Draw(GODC &dc) {
  GOBitmap &bmp
    = m_ButtonControl->DisplayInverted() ^ m_ButtonControl->IsEngaged()
    ? m_OnBitmap
    : m_OffBitmap;
  dc.DrawBitmap(bmp, m_BoundingRect);
  if (m_TextWidth)
    dc.DrawText(m_Text, m_TextRect, m_TextColor, m_Font, m_TextWidth);

  GOGUIControl::Draw(dc);
}
