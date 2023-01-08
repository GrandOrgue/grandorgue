/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTEMPERAMENT_H
#define GOTEMPERAMENT_H

#include <wx/string.h>

class GOTemperament {
protected:
  wxString m_Group;
  wxString m_GroupTitle;
  wxString m_Name;
  wxString m_Title;
  bool m_respectPitchTuning;

public:
  GOTemperament(wxString name, wxString group = wxEmptyString);
  GOTemperament(
    wxString name, wxString title, wxString group, wxString groupTitle);
  virtual ~GOTemperament();

  virtual float GetOffset(unsigned index) const;
  wxString GetName() const;
  wxString GetTitle() const;
  wxString GetGroup() const;
  wxString GetGroupTitle() const;
  bool GetRespectPitchTuning() const;
};

#endif
