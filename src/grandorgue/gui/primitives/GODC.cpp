/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GODC.h"

#include <wx/dc.h>

#include "GOBitmap.h"
#include "GOFont.h"

GODC::GODC(wxDC *dc, double scale, double fontScale)
  : m_DC(dc), m_Scale(scale), m_FontScale(fontScale) {}

wxRect GODC::ScaleRect(const wxRect &rect) {
  return wxRect(
    rect.GetX() * m_Scale + 0.5,
    rect.GetY() * m_Scale + 0.5,
    rect.GetWidth() * m_Scale + 0.5,
    rect.GetHeight() * m_Scale + 0.5);
}

void GODC::DrawBitmap(GOBitmap &bitmap, const wxRect &target) {
  unsigned xpos = target.GetX() * m_Scale + 0.5;
  unsigned ypos = target.GetY() * m_Scale + 0.5;
  m_DC->DrawBitmap(bitmap.GetBitmap(), xpos, ypos, true);
}

wxString GODC::WrapText(const wxString &string, unsigned width) {
  wxString str, line, work;
  wxCoord cx, cy;

  /* string.Length() + 1 iterations */
  for (unsigned i = 0; i <= string.Length(); i++) {
    bool maybreak = false;
    if (string[i] == wxT(' ') || string[i] == wxT('\n')) {
      if (work.length() < 2)
        maybreak = false;
      else
        maybreak = true;
    }
    if (maybreak || i == string.Length()) {
      if (!work.Length())
        continue;
      m_DC->GetTextExtent(line + wxT(' ') + work, &cx, &cy);
      if (cx > (int)width) {
        if (!str.Length())
          str = line;
        else
          str = str + wxT('\n') + line;
        line = wxT("");
      }

      if (!line.Length())
        line = work;
      else
        line = line + wxT(' ') + work;

      work = wxT("");
    } else {
      if (string[i] == wxT(' ') || string[i] == wxT('\n')) {
        if (work.Length() && work[work.Length() - 1] != wxT(' '))
          work += wxT(' ');
      } else
        work += string[i];
    }
  }
  if (!str.Length())
    str = line;
  else
    str = str + wxT('\n') + line;
  return str;
}

void GODC::DrawText(
  const wxString &text,
  const wxRect &rect,
  const wxColour &color,
  GOFont &font,
  unsigned text_width,
  bool align_top) {
  m_DC->SetTextForeground(color);
  m_DC->SetFont(font.GetFont(m_Scale * m_FontScale));
  m_DC->DrawLabel(
    WrapText(text, m_Scale * text_width),
    ScaleRect(rect),
    (align_top ? 0 : wxALIGN_CENTER_VERTICAL) | wxALIGN_CENTER_HORIZONTAL);
}
