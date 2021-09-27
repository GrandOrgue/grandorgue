/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUETEMPERAMENT_H
#define GORGUETEMPERAMENT_H

#include <wx/string.h>

class GOrgueTemperament
{
protected:
	wxString m_Group;
	wxString m_Name;
	wxString m_Title;

public:
	GOrgueTemperament(wxString name, wxString group = wxEmptyString);
	GOrgueTemperament(wxString name, wxString title, wxString group);
	virtual ~GOrgueTemperament();

	virtual float GetOffset(bool ignorepitch, unsigned midi_number, unsigned wav_midi_number, float wav_pitch_fract, float harmonic_number, float pitch_correction, float default_tuning) const;
	wxString GetName() const;
	wxString GetTitle() const;
	wxString GetGroup() const;
};

#endif
