/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GODC_H
#define GODC_H

#include <wx/string.h>

class GOBitmap;
class GOFont;
class wxDC;
class wxColour;
class wxRect;

class GODC {
private:
  wxDC *m_DC;
  double m_Scale;
  double m_FontScale;

  wxString WrapText(const wxString &string, unsigned width);

public:
  GODC(wxDC *dc, double scale, double fontScale);

  void DrawBitmap(GOBitmap &bitmap, const wxRect &target);
  void DrawText(
    const wxString &text,
    const wxRect &rect,
    const wxColour &color,
    GOFont &font,
    unsigned text_width,
    bool top = false);

  wxRect ScaleRect(const wxRect &rect);
};

#endif
