/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOBITMAP_H
#define GOBITMAP_H

#include <wx/bitmap.h>

class wxImage;

class GOBitmap {
private:
  wxImage *m_img;
  wxBitmap m_bmp;
  double m_Scale;
  int m_ResultWidth;
  int m_ResultHeight;
  unsigned m_ResultXOffset;
  unsigned m_ResultYOffset;

  void ScaleBMP(
    wxImage &img, double scale, const wxRect &rect, GOBitmap *background);

public:
  GOBitmap();
  GOBitmap(wxImage *img);

  void PrepareBitmap(double scale, const wxRect &rect, GOBitmap *background);
  void PrepareTileBitmap(
    double scale,
    const wxRect &rect,
    unsigned xo,
    unsigned yo,
    GOBitmap *background);

  unsigned GetWidth();
  unsigned GetHeight();

  const wxBitmap &GetBitmap();
};

#endif
