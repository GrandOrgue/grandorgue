/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
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

private:
  bool
    m_IsTemperamentOriginalBased; // Decides the base for GetOffset(): Original
                                  // temperament or Equal temperament

public:
  GOTemperament(
    wxString name, wxString group = wxEmptyString, bool isOriginalBased = true);
  GOTemperament(
    wxString name,
    wxString title,
    wxString group,
    wxString groupTitle,
    bool isOriginalBased = true);
  virtual ~GOTemperament();

  // if m_IsTemperamentOriginalBased GetOffset() returns the deviation from
  // original temperament else the deviation from equal temperament
  virtual float GetOffset(unsigned index) const;
  wxString GetName() const;
  wxString GetTitle() const;
  wxString GetGroup() const;
  wxString GetGroupTitle() const;
  bool IsTemperamentOriginalBased() const {
    return m_IsTemperamentOriginalBased;
  }
};

#endif
