/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTEMPERAMENT_H
#define GOTEMPERAMENT_H

#include <wx/string.h>

class GOTemperament {
protected:
  wxString m_Group;
  wxString m_Name;
  wxString m_Title;

public:
  GOTemperament(wxString name, wxString group = wxEmptyString);
  GOTemperament(wxString name, wxString title, wxString group);
  virtual ~GOTemperament();

  virtual float GetOffset(
    bool ignorepitch,
    unsigned midi_number,
    unsigned wav_midi_number,
    float wav_pitch_fract,
    float harmonic_number,
    float pitch_correction,
    float default_tuning) const;
  wxString GetName() const;
  wxString GetTitle() const;
  wxString GetGroup() const;
};

#endif
