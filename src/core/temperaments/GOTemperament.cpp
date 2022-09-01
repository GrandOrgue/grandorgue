/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTemperament.h"

#include <wx/intl.h>

GOTemperament::GOTemperament(wxString name, wxString group)
  : m_Group(group),
    m_GroupTitle(wxEmptyString),
    m_Name(name),
    m_Title(wxEmptyString) {}

GOTemperament::GOTemperament(
  wxString name, wxString title, wxString group, wxString groupTitle)
  : m_Group(group), m_GroupTitle(groupTitle), m_Name(name), m_Title(title) {}

GOTemperament::~GOTemperament() {}

float GOTemperament::GetOffset(
  bool ignorepitch,
  unsigned midi_number,
  unsigned wav_midi_number,
  float wav_pitch_fract,
  float harmonic_number,
  float pitch_correction,
  float default_tuning) const {
  return 0;
}

wxString GOTemperament::GetName() const { return m_Name; }

wxString GOTemperament::GetTitle() const {
  return m_Title.IsEmpty() ? wxGetTranslation(m_Name) : m_Title;
}

wxString GOTemperament::GetGroup() const { return m_Group; }

wxString GOTemperament::GetGroupTitle() const {
  return m_GroupTitle.IsEmpty() ? wxGetTranslation(m_Group) : m_GroupTitle;
}
