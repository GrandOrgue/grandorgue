/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOFont.h"

#include <wx/gdicmn.h>

GOFont::GOFont() : m_Name(), m_Points(0), m_Scale(0) {
  m_Font = *wxNORMAL_FONT;
}

void GOFont::SetName(const wxString &name) {
  if (name == wxEmptyString)
    return;
  wxFont new_font = m_Font;
  if (new_font.SetFaceName(name)) {
    m_Font = new_font;
    m_Name = name;
    m_Scale = 0;
  }
}
void GOFont::SetPoints(unsigned points) {
  m_Font.SetPointSize(points);
  m_Points = points;
  m_Scale = 0;
}

wxFont GOFont::GetFont(double scale) {
  if (m_Scale != scale) {
    m_ScaledFont = m_Font;
    m_ScaledFont.SetPointSize(m_Points * scale);
    m_Scale = scale;
  }
  return m_ScaledFont;
}
