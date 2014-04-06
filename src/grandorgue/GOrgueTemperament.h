/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2014 GrandOrgue contributors (see AUTHORS)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
	virtual ~GOrgueTemperament();

	virtual float GetOffset(bool ignorepitch, unsigned midi_number, unsigned wav_midi_number, float wav_pitch_fract, float harmonic_number, float pitch_correction, float default_tuning) const;
	wxString GetName() const;
	wxString GetTitle() const;
	wxString GetGroup() const;
};

#endif
