/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGUILabel.h"

#include "GODC.h"
#include "GODefinitionFile.h"
#include "GOGUIDisplayMetrics.h"
#include "GOGUILayoutEngine.h"
#include "GOGUIPanel.h"
#include "GOLabel.h"
#include "config/GOConfig.h"
#include "config/GOConfigReader.h"

GOGUILabel::GOGUILabel(GOGUIPanel *panel, GOLabel *label)
    : GOGUIControl(panel, label),
      m_DispXpos(0),
      m_DispYpos(0),
      m_Label(label),
      m_Bitmap(),
      m_FontSize(0),
      m_FontName(),
      m_Text(),
      m_TextColor(0, 0, 0),
      m_TextRect(),
      m_TextWidth(0),
      m_TileOffsetX(0),
      m_TileOffsetY(0) {}

void GOGUILabel::Init(
  GOConfigReader &cfg,
  wxString group,
  unsigned x_pos,
  unsigned y_pos,
  wxString name,
  unsigned DispImageNum) {
  GOGUIControl::Init(cfg, group);

  m_TextColor = wxColour(0x00, 0x00, 0x00);
  m_FontSize = 7;
  m_FontName = wxT("");
  m_Text = name;

  wxString image_file
    = wxString::Format(wxT(GOBitmapPrefix "label%02d"), DispImageNum);
  wxString image_mask_file = wxEmptyString;

  m_Bitmap = m_panel->LoadBitmap(image_file, image_mask_file);

  int x, y, w, h;
  x = x_pos;
  y = y_pos;
  w = m_Bitmap.GetWidth();
  h = m_Bitmap.GetHeight();
  m_BoundingRect = wxRect(x, y, w, h);

  m_TileOffsetX = 0;
  m_TileOffsetY = 0;

  x = 1;
  y = 1;
  w = m_BoundingRect.GetWidth() - x;
  h = m_BoundingRect.GetHeight() - y;
  m_TextRect = wxRect(x, y, w, h);
  m_TextWidth = m_TextRect.GetWidth();

  m_Font = m_metrics->GetGroupLabelFont();
  m_Font.SetName(m_FontName);
  m_Font.SetPoints(m_FontSize);
}

void GOGUILabel::Load(GOConfigReader &cfg, wxString group) {
  GOGUIControl::Load(cfg, group);

  bool FreeXPlacement
    = cfg.ReadBoolean(ODFSetting, group, wxT("FreeXPlacement"), false, true);
  bool FreeYPlacement
    = cfg.ReadBoolean(ODFSetting, group, wxT("FreeYPlacement"), false, true);

  int x = -1, y = -1;
  if (!FreeXPlacement) {
    int DispDrawstopCol = cfg.ReadInteger(
      ODFSetting,
      group,
      wxT("DispDrawstopCol"),
      1,
      m_metrics->NumberOfDrawstopColsToDisplay(),
      true,
      1);
    bool DispSpanDrawstopColToRight = cfg.ReadBoolean(
      ODFSetting, group, wxT("DispSpanDrawstopColToRight"), true, false);

    int i = m_metrics->NumberOfDrawstopColsToDisplay() >> 1;
    if (DispSpanDrawstopColToRight)
      m_DispXpos = 39;
    else
      m_DispXpos = 0;
    if (DispDrawstopCol <= i)
      m_DispXpos = m_DispXpos + (DispDrawstopCol - 1) * 78 + 1;
    else
      m_DispXpos = -(m_DispXpos + (DispDrawstopCol - 1 - i) * 78 + 1);
  } else {
    x = cfg.ReadInteger(
      ODFSetting,
      group,
      wxT("DispXpos"),
      0,
      m_metrics->GetScreenWidth(),
      false,
      0);
  }

  if (!FreeYPlacement) {
    bool DispAtTopOfDrawstopCol = cfg.ReadBoolean(
      ODFSetting, group, wxT("DispAtTopOfDrawstopCol"), true, false);

    m_DispYpos = 1;
    if (!DispAtTopOfDrawstopCol) m_DispYpos += -32;
  } else {
    y = cfg.ReadInteger(
      ODFSetting,
      group,
      wxT("DispYpos"),
      0,
      m_metrics->GetScreenHeight(),
      false,
      0);
  }

  m_TextColor = cfg.ReadColor(
    ODFSetting, group, wxT("DispLabelColour"), false, wxT("BLACK"));
  m_FontSize = cfg.ReadFontSize(
    ODFSetting, group, wxT("DispLabelFontSize"), false, wxT("normal"));
  m_FontName = cfg.ReadStringTrim(
    ODFSetting, group, wxT("DispLabelFontName"), false, wxT(""));
  if (!m_Label || !m_panel->GetOrganFile()->GetSettings().ODFCheck())
    m_Text
      = cfg.ReadString(ODFSetting, group, wxT("Name"), false, wxEmptyString);

  unsigned DispImageNum
    = cfg.ReadInteger(ODFSetting, group, wxT("DispImageNum"), 1, 12, false, 1);

  wxString image_file = cfg.ReadStringTrim(
    ODFSetting,
    group,
    wxT("Image"),
    false,
    wxString::Format(wxT(GOBitmapPrefix "label%02d"), DispImageNum));
  wxString image_mask_file
    = cfg.ReadStringTrim(ODFSetting, group, wxT("Mask"), false, wxEmptyString);

  m_Bitmap = m_panel->LoadBitmap(image_file, image_mask_file);

  int w, h;
  x = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("PositionX"),
    0,
    m_metrics->GetScreenWidth(),
    false,
    x);
  y = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("PositionY"),
    0,
    m_metrics->GetScreenHeight(),
    false,
    y);
  w = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("Width"),
    1,
    m_metrics->GetScreenWidth(),
    false,
    m_Bitmap.GetWidth());
  h = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("Height"),
    1,
    m_metrics->GetScreenHeight(),
    false,
    m_Bitmap.GetHeight());
  m_BoundingRect = wxRect(x, y, w, h);

  m_TileOffsetX = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("TileOffsetX"),
    0,
    m_Bitmap.GetWidth() - 1,
    false,
    0);
  m_TileOffsetY = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("TileOffsetY"),
    0,
    m_Bitmap.GetHeight() - 1,
    false,
    0);

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
    m_TextRect.GetWidth());

  m_Font = m_metrics->GetGroupLabelFont();
  m_Font.SetName(m_FontName);
  m_Font.SetPoints(m_FontSize);
}

void GOGUILabel::Layout() {
  if (m_DispXpos >= 0)
    m_DispXpos = m_layout->GetJambLeftX() + m_DispXpos;
  else
    m_DispXpos = m_layout->GetJambRightX() - m_DispXpos;

  if (m_DispYpos >= 0)
    m_DispYpos = m_layout->GetJambLeftRightY() + 1;
  else
    m_DispYpos = m_layout->GetJambLeftRightY() + 1
      + m_layout->GetJambLeftRightHeight() - 32;

  if (m_BoundingRect.GetX() == -1) m_BoundingRect.SetX(m_DispXpos);
  if (m_BoundingRect.GetY() == -1) m_BoundingRect.SetY(m_DispYpos);

  m_TextRect.Offset(m_BoundingRect.GetX(), m_BoundingRect.GetY());
}

void GOGUILabel::PrepareDraw(double scale, GOBitmap *background) {
  m_Bitmap.PrepareTileBitmap(
    scale, m_BoundingRect, m_TileOffsetX, m_TileOffsetY, background);
}

void GOGUILabel::Draw(GODC &dc) {
  if (m_Label) m_Text = m_Label->GetContent();

  dc.DrawBitmap(m_Bitmap, m_BoundingRect);
  if (m_TextWidth)
    dc.DrawText(m_Text, m_TextRect, m_TextColor, m_Font, m_TextWidth);

  GOGUIControl::Draw(dc);
}

bool GOGUILabel::HandleMousePress(
  int x, int y, bool right, GOGUIMouseState &state) {
  if (!m_BoundingRect.Contains(x, y)) return false;
  if (right) {
    if (!m_Label) return false;
    m_Label->ShowConfigDialog();
    return true;
  } else
    return false;
}
