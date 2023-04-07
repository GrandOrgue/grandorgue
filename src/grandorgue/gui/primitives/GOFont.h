/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOFONT_H
#define GOFONT_H

#include <wx/font.h>
#include <wx/string.h>

class GOFont {
private:
  wxFont m_Font;
  wxFont m_ScaledFont;
  wxString m_Name;
  unsigned m_Points;
  double m_Scale;

public:
  GOFont();

  const wxString &GetName() const { return m_Name; }
  void SetName(const wxString &name);
  unsigned GetPoints() const { return m_Points; }
  void SetPoints(unsigned points);
  wxFont GetFont(double scale);
};

#endif
