/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGUIImage.h"

#include "primitives/GODC.h"

#include "GOGUIDisplayMetrics.h"
#include "GOGUIPanel.h"
#include "config/GOConfigReader.h"

GOGUIImage::GOGUIImage(GOGUIPanel *panel)
  : GOGUIControl(panel, NULL), m_TileOffsetX(0), m_TileOffsetY(0) {}

void GOGUIImage::Load(GOConfigReader &cfg, wxString group) {
  GOGUIControl::Load(cfg, group);
  int x, y, w, h;
  wxString image_mask_file;
  wxString image_file;

  image_file = cfg.ReadStringTrim(ODFSetting, group, wxT("Image"), true);
  image_mask_file
    = cfg.ReadStringTrim(ODFSetting, group, wxT("Mask"), false, wxEmptyString);

  m_Bitmap = m_panel->LoadBitmap(image_file, image_mask_file);

  x = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("PositionX"),
    0,
    m_metrics->GetScreenWidth(),
    false,
    0);
  y = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("PositionY"),
    0,
    m_metrics->GetScreenHeight(),
    false,
    0);
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
}

void GOGUIImage::PrepareDraw(double scale, GOBitmap *background) {
  m_Bitmap.PrepareTileBitmap(
    scale, m_BoundingRect, m_TileOffsetX, m_TileOffsetY, background);
}

void GOGUIImage::Draw(GODC &dc) {
  dc.DrawBitmap(m_Bitmap, m_BoundingRect);
  GOGUIControl::Draw(dc);
}
