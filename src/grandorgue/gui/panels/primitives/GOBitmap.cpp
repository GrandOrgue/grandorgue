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

unsigned GOBitmap::GetSourceWidth() const {
  return p_SourceImage ? p_SourceImage->GetWidth() : 0;
}

unsigned GOBitmap::GetSourceHeight() const {
  return p_SourceImage ? p_SourceImage->GetHeight() : 0;
}

void GOBitmap::BuildBitmapFrom(
  const wxImage &img, double scale, const wxRect &rect, GOBitmap *background) {
  const int imgHeight = img.GetHeight();
  const int imgWidth = img.GetWidth();
  const int newHeight = imgHeight * scale;
  const int newWidth = imgWidth * scale;

  m_ResultValid = newHeight > 0 && newWidth > 0;
  if (m_ResultValid) {
    const wxBitmap *pBackgroundBitmap
      = background ? background->GetResultBitmap() : nullptr;

    if (pBackgroundBitmap && img.HasAlpha()) {
      wxBitmap bmp(imgWidth, imgHeight);
      wxBitmap orig(img);
      wxMemoryDC dc;

      dc.SelectObject(bmp);
      dc.DrawBitmap(*pBackgroundBitmap, -rect.GetX(), -rect.GetY(), false);
      dc.DrawBitmap(orig, 0, 0, true);
      bmp.SetMask(orig.GetMask());

      wxImage img_result = bmp.ConvertToImage();

      if (!img_result.HasAlpha())
        img_result.InitAlpha();
      memcpy(img_result.GetAlpha(), img.GetAlpha(), imgWidth * imgHeight);
      m_ResultBitmap = (wxBitmap)img_result.Scale(
        newWidth, newHeight, wxIMAGE_QUALITY_BICUBIC);
    } else
      m_ResultBitmap
        = (wxBitmap)img.Scale(newWidth, newHeight, wxIMAGE_QUALITY_BICUBIC);
  }
  m_Scale = scale;
}

void GOBitmap::BuildScaledBitmap(
  double scale, const wxRect &rect, GOBitmap *background) {
  if (p_SourceImage && (scale != m_Scale || m_ResultWidth || m_ResultHeight)) {
    BuildBitmapFrom(*p_SourceImage, scale, rect, background);
    m_ResultWidth = 0;
    m_ResultHeight = 0;
  }
}

void GOBitmap::BuildTileBitmap(
  double scale,
  const wxRect &newRect,
  unsigned newXOffset,
  unsigned newYOffset,
  GOBitmap *background) {
  const int tgtHeight = newRect.GetHeight();
  const int tgtWidth = newRect.GetWidth();

  if (
    p_SourceImage
    && (scale != m_Scale || m_ResultWidth != tgtHeight || m_ResultHeight != tgtWidth || newXOffset != m_ResultXOffset || newYOffset != m_ResultYOffset)) {
    const int srcHeight = p_SourceImage->GetHeight();
    const int srcWidth = p_SourceImage->GetWidth();
    wxImage img(tgtWidth, tgtHeight);

    for (int y = -newYOffset; y < tgtHeight; y += srcHeight)
      for (int x = -newXOffset; x < tgtWidth; x += srcWidth) {
        // Calculate source starting position using std::max(0, -offset)
        const int srcX = std::max(0, -x);
        const int srcY = std::max(0, -y);

        // Ensure copy stays within target bounds using std::min
        const int copyWidth = std::min(srcWidth - srcX, tgtWidth - x);
        const int copyHeight = std::min(srcHeight - srcY, tgtHeight - y);

        // Paste only if valid copy region exists
        if (copyWidth > 0 && copyHeight > 0) {
          if (copyWidth != (int)srcWidth || copyHeight != (int)srcHeight) {
            // Partial copy - use GetSubImage
            wxImage tile = p_SourceImage->GetSubImage(
              wxRect(srcX, srcY, copyWidth, copyHeight));
            img.Paste(tile, x, y);
          } else {
            // Full tile copy - use direct Paste
            img.Paste(*p_SourceImage, x, y);
          }
        }
      }
    BuildBitmapFrom(img, scale, newRect, background);
    m_ResultHeight = tgtHeight;
    m_ResultWidth = tgtWidth;
    m_ResultXOffset = newXOffset;
    m_ResultYOffset = newYOffset;
  }
}
