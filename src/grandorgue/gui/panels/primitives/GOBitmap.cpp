/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOBitmap.h"

#include <wx/bitmap.h>
#include <wx/dcmemory.h>
#include <wx/image.h>

GOBitmap::GOBitmap()
  : p_SourceImage(nullptr),
    m_Scale(0),
    m_ResultWidth(0),
    m_ResultHeight(0),
    m_ResultXOffset(0),
    m_ResultYOffset(0) {}

GOBitmap::GOBitmap(const wxImage *pSourceImg)
  : p_SourceImage(pSourceImg),
    m_Scale(0),
    m_ResultWidth(0),
    m_ResultHeight(0),
    m_ResultXOffset(0),
    m_ResultYOffset(0) {}

unsigned GOBitmap::GetSourceWidth() const { return p_SourceImage->GetWidth(); }

unsigned GOBitmap::GetSourceHeight() const {
  return p_SourceImage->GetHeight();
}

void GOBitmap::BuildBitmapFrom(
  const wxImage &img, double scale, const wxRect &rect, GOBitmap *background) {
  if (background && img.HasAlpha()) {
    wxBitmap bmp(img.GetWidth(), img.GetHeight());
    wxBitmap orig(img);
    wxMemoryDC dc;

    dc.SelectObject(bmp);
    dc.DrawBitmap(
      background->GetResultBitmap(), -rect.GetX(), -rect.GetY(), false);
    dc.DrawBitmap(orig, 0, 0, true);
    bmp.SetMask(orig.GetMask());
    wxImage img_result = bmp.ConvertToImage();
    if (!img_result.HasAlpha())
      img_result.InitAlpha();
    memcpy(
      img_result.GetAlpha(), img.GetAlpha(), img.GetWidth() * img.GetHeight());

    m_ResultBitmap = (wxBitmap)img_result.Scale(
      img.GetWidth() * scale, img.GetHeight() * scale, wxIMAGE_QUALITY_BICUBIC);
  } else
    m_ResultBitmap = (wxBitmap)img.Scale(
      img.GetWidth() * scale, img.GetHeight() * scale, wxIMAGE_QUALITY_BICUBIC);
  m_Scale = scale;
}

void GOBitmap::BuildScaledBitmap(
  double scale, const wxRect &rect, GOBitmap *background) {
  if (scale != m_Scale || m_ResultWidth || m_ResultHeight) {
    BuildBitmapFrom(*p_SourceImage, scale, rect, background);
    m_ResultWidth = 0;
    m_ResultHeight = 0;
  }
}

void GOBitmap::BuildTileBitmap(
  double scale,
  const wxRect &rect,
  unsigned xo,
  unsigned yo,
  GOBitmap *background) {
  if (
    scale != m_Scale || m_ResultWidth != rect.GetWidth()
    || m_ResultHeight != rect.GetHeight() || xo != m_ResultXOffset
    || yo != m_ResultYOffset) {
    wxImage img(rect.GetWidth(), rect.GetHeight());

    for (int y = -yo; y < img.GetHeight(); y += GetSourceHeight())
      for (int x = -xo; x < img.GetWidth(); x += GetSourceWidth())
        img.Paste(*p_SourceImage, x, y);
    BuildBitmapFrom(img, scale, rect, background);
    m_ResultWidth = rect.GetWidth();
    m_ResultHeight = rect.GetHeight();
    m_ResultXOffset = xo;
    m_ResultYOffset = yo;
  }
}
