/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGUILabel.h"

#include "config/GOConfig.h"
#include "config/GOConfigReader.h"
#include "control/GOLabelControl.h"
#include "gui/wxcontrols/go_gui_utils.h"
#include "primitives/GOBitmap.h"
#include "primitives/GODC.h"

#include "GOGUIDisplayMetrics.h"
#include "GOGUIPanel.h"
#include "GOOrganController.h"

GOGUILabel::GOGUILabel(GOGUIPanel *panel, GOLabelControl *label)
  : GOGUIMidiControl(panel, label, label),
    m_DispXpos(0),
    m_DispYpos(0),
    m_Label(label),
    m_PBackgroundBitmap(nullptr),
    m_Text(),
    m_TextColor(0, 0, 0),
    m_TextRect(),
    m_TextWidth(0),
    m_GridColN(0),
    m_GridRowN(0),
    m_TileOffsetX(0),
    m_TileOffsetY(0) {}

/**
 * Deletes m_PBackgroundBitmap if it has been allocated
 */
void GOGUILabel::DestroyBackgroundBitmap() {
  GOBitmap *pOldBackgroundBitmap = m_PBackgroundBitmap;

  // reset the old background
  if (pOldBackgroundBitmap) {
    m_PBackgroundBitmap = nullptr;
    delete pOldBackgroundBitmap;
  }
}

static const wxString WX_BITMAP_LABEL_FMT = wxT(GOBitmapPrefix "label%02d");

/**
 * Creates m_PBackgroundBitmap image from the imageFileName and imageNum and
 * sets the default values for m_BoundingRect, m_TextRect, m_TextWidth,
 * m_TileOffsetX, m_TileOffsetY, basing on the image size. The position of
 * m_BoundingRect is set by the caller
 * @param imageFileName the file name of the bitmap. If it is empty then
 *   it is calculated from imageNum
 * @param imageNum if > 0 then the number of predefined images. If 0 then the
 *   background is transparent. This parameter is used only if imageFileName is
 *   empty.
 * @param imageMaskFilename a filename of the image mask
 */
void GOGUILabel::InitBackgroundBitmap(
  unsigned w,
  unsigned h,
  wxString imageFileName,
  unsigned imageNum,
  const wxString &imageMaskFilename) {
  DestroyBackgroundBitmap();
  if (imageFileName.IsEmpty() && imageNum)
    // Calculate imageFileName from imageNum
    imageFileName.Printf(WX_BITMAP_LABEL_FMT, imageNum);
  if (!imageFileName.IsEmpty()) {
    m_PBackgroundBitmap = new GOBitmap();
    m_PBackgroundBitmap->SetSourceImage(
      p_panel->LoadImage(imageFileName, imageMaskFilename));
  }

  // take the size from the bitmap if it is not specified
  if (!w)
    w = m_PBackgroundBitmap ? m_PBackgroundBitmap->GetSourceWidth() : 80;
  if (!h)
    h = m_PBackgroundBitmap ? m_PBackgroundBitmap->GetSourceHeight() : 25;
  unsigned textX = 1;
  unsigned textY = 1;
  unsigned textWidth = w - textX;
  unsigned textHeigth = h - textY;

  m_BoundingRect = wxRect(0, 0, w, h);
  m_TextRect = wxRect(textX, textY, textWidth, textHeigth);
  m_TextWidth = textWidth;
  m_TileOffsetX = 0;
  m_TileOffsetY = 0;
}

void GOGUILabel::InitFont(const wxString &fontName, unsigned fontSize) {
  m_Font = p_metrics->GetGroupLabelFont();
  SetFontName(fontName);
  SetFontSize(fontSize);
}

void GOGUILabel::Init(
  GOConfigReader &cfg,
  wxString group,
  unsigned x_pos,
  unsigned y_pos,
  wxString name,
  unsigned dispImageNum,
  unsigned w,
  unsigned h,
  std::optional<GOGUILayoutEngine::GridType> gridType) {
  GOGUIControl::Init(cfg, group);

  m_TextColor = wxColour(0x00, 0x00, 0x00);
  m_Text = name;
  m_GridType = gridType;
  m_GridColN = x_pos;
  m_GridRowN = y_pos;

  InitBackgroundBitmap(w, h, wxEmptyString, dispImageNum, wxEmptyString);
  // In a grid the position is set by Layout(): wxDefaultPosition means "not
  // set"
  m_BoundingRect.SetPosition(
    gridType ? wxDefaultPosition : wxPoint(x_pos, y_pos));
  InitFont(wxEmptyString, 7);
}

static const wxString WX_IMAGE = wxT("Image");
static const wxString WX_DISP_IMAGE_NUM = wxT("DispImageNum");
static const wxString WX_MASK = wxT("Mask");

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
      p_metrics->NumberOfDrawstopColsToDisplay(),
      true,
      1);
    bool DispSpanDrawstopColToRight = cfg.ReadBoolean(
      ODFSetting, group, wxT("DispSpanDrawstopColToRight"), true, false);

    int i = p_metrics->NumberOfDrawstopColsToDisplay() >> 1;
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
      p_metrics->GetScreenWidth(),
      false,
      0);
  }

  if (!FreeYPlacement) {
    bool DispAtTopOfDrawstopCol = cfg.ReadBoolean(
      ODFSetting, group, wxT("DispAtTopOfDrawstopCol"), true, false);

    m_DispYpos = 1;
    if (!DispAtTopOfDrawstopCol)
      m_DispYpos += -32;
  } else {
    y = cfg.ReadInteger(
      ODFSetting,
      group,
      wxT("DispYpos"),
      0,
      p_metrics->GetScreenHeight(),
      false,
      0);
  }

  x = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("PositionX"),
    0,
    p_metrics->GetScreenWidth(),
    false,
    x);
  y = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("PositionY"),
    0,
    p_metrics->GetScreenHeight(),
    false,
    y);

  m_TextColor = logicalToWxColour(cfg.ReadColor(
    ODFSetting, group, wxT("DispLabelColour"), false, wxT("BLACK")));
  if (!m_Label || !p_panel->GetOrganFile()->GetSettings().ODFCheck())
    m_Text
      = cfg.ReadString(ODFSetting, group, wxT("Name"), false, wxEmptyString);

  InitBackgroundBitmap(
    cfg.ReadInteger(
      ODFSetting,
      group,
      wxT("Width"),
      1,
      p_metrics->GetScreenWidth(),
      false,
      0),
    cfg.ReadInteger(
      ODFSetting,
      group,
      wxT("Height"),
      1,
      p_metrics->GetScreenHeight(),
      false,
      0),
    cfg.ReadStringTrim(ODFSetting, group, WX_IMAGE, false),
    cfg.ReadInteger(ODFSetting, group, WX_DISP_IMAGE_NUM, 0, 15, false, 1),
    cfg.ReadStringTrim(ODFSetting, group, WX_MASK, false));
  m_BoundingRect.SetPosition(wxPoint(x, y));
  InitFont(
    cfg.ReadStringTrim(
      ODFSetting, group, wxT("DispLabelFontName"), false, wxEmptyString),
    cfg.ReadFontSize(
      ODFSetting, group, wxT("DispLabelFontSize"), false, wxT("normal")));

  m_TileOffsetX = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("TileOffsetX"),
    0,
    m_BoundingRect.width - 1,
    false,
    0);
  m_TileOffsetY = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("TileOffsetY"),
    0,
    m_BoundingRect.height - 1,
    false,
    0);

  x = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("TextRectLeft"),
    0,
    m_BoundingRect.width - 1,
    false,
    m_TextRect.x);
  y = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("TextRectTop"),
    0,
    m_BoundingRect.height - 1,
    false,
    m_TextRect.y);

  unsigned w = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("TextRectWidth"),
    1,
    m_BoundingRect.width - x,
    false,
    m_BoundingRect.width - x);
  unsigned h = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("TextRectHeight"),
    1,
    m_BoundingRect.height - y,
    false,
    m_BoundingRect.height - y);
  m_TextRect = wxRect(x, y, w, h);
  m_TextWidth
    = cfg.ReadInteger(ODFSetting, group, wxT("TextBreakWidth"), 0, w, false, w);
}

void GOGUILabel::Layout() {
  int x = 0;
  int y = 0;

  if (m_GridType) {
    p_layout->GetBlitPosition(
      *m_GridType,
      m_GridRowN,
      m_GridColN,
      m_BoundingRect.GetWidth(),
      m_BoundingRect.GetHeight(),
      x,
      y);
  } else {
    if (m_DispXpos >= 0)
      m_DispXpos = p_layout->GetJambLeftX() + m_DispXpos;
    else
      m_DispXpos = p_layout->GetJambRightX() - m_DispXpos;

    if (m_DispYpos >= 0)
      m_DispYpos = p_layout->GetJambLeftRightY() + 1;
    else
      m_DispYpos = p_layout->GetJambLeftRightY() + 1
        + p_layout->GetJambLeftRightHeight() - 32;
    x = m_DispXpos;
    y = m_DispYpos;
  }
  SetPositionIfUnset(x, y);

  m_TextRect.Offset(m_BoundingRect.GetX(), m_BoundingRect.GetY());
}

void GOGUILabel::PrepareDraw(double scale, GOBitmap *background) {
  if (m_PBackgroundBitmap)
    m_PBackgroundBitmap->BuildTileBitmap(
      scale, m_BoundingRect, m_TileOffsetX, m_TileOffsetY, background);
}

void GOGUILabel::Draw(GODC &dc) {
  if (m_Label)
    m_Text = m_Label->GetContent();

  if (m_PBackgroundBitmap)
    dc.DrawBitmap(*m_PBackgroundBitmap, m_BoundingRect);
  if (m_TextWidth)
    dc.DrawText(m_Text, m_TextRect, m_TextColor, m_Font, m_TextWidth);

  GOGUIControl::Draw(dc);
}

bool GOGUILabel::HandleMousePress(
  int x, int y, bool right, GOGUIMouseState &state) {
  return m_BoundingRect.Contains(x, y) && m_Label && HandleMidiConfig(right);
}
