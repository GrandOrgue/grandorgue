/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOBITMAP_H
#define GOBITMAP_H

#include <wx/bitmap.h>

class wxImage;
class wxRect;

/**
 * This class is designed for building a result wxBitmap instance from a source
 * wxImage instance. It supports both scaling and tiling
 */

class GOBitmap {
private:
  const wxImage *p_SourceImage = nullptr;
  wxBitmap m_ResultBitmap;
  double m_Scale = 0.0;
  int m_ResultWidth = 0;
  int m_ResultHeight = 0;
  unsigned m_ResultXOffset = 0;
  unsigned m_ResultYOffset = 0;

  void BuildBitmapFrom(
    const wxImage &img, double scale, const wxRect &rect, GOBitmap *background);

public:
  void SetSourceImage(const wxImage *pSourceImg) { p_SourceImage = pSourceImg; }

  unsigned GetSourceWidth() const;
  unsigned GetSourceHeight() const;

  void BuildScaledBitmap(
    double scale, const wxRect &rect, GOBitmap *background);
  void BuildTileBitmap(
    double scale,
    const wxRect &rect,
    unsigned xo,
    unsigned yo,
    GOBitmap *background);

  const wxBitmap &GetResultBitmap() const { return m_ResultBitmap; }
};

#endif
