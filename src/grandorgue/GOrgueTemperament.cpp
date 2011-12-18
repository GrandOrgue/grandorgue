/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2011 GrandOrgue contributors (see AUTHORS)
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
 * MA 02111-1307, USA.
 */

#include "GOrgueTemperament.h"
#include "GOrgueTemperamentCent.h"

GOrgueTemperament::GOrgueTemperament(wxString name) :
	m_Name(name)
{
}

GOrgueTemperament::~GOrgueTemperament()
{
}

float GOrgueTemperament::GetOffset(unsigned midi_number, unsigned wav_midi_number, float wav_pitch_fract, float harmonic_number, float default_tuning) const
{
	return 0;
}

wxString GOrgueTemperament::GetName() const
{
	return m_Name;
}

std::vector<wxString> GOrgueTemperament::GetNames()
{
	InitTemperaments();

	std::vector<wxString> names;
	for(unsigned i = 0; i < m_Temperaments.size(); i++)
		names.push_back(m_Temperaments[i]->GetName());

	return names;
}

const GOrgueTemperament& GOrgueTemperament::GetTemperament(wxString Name)
{
	InitTemperaments();

	for(unsigned i = 0; i < m_Temperaments.size(); i++)
		if (Name == m_Temperaments[i]->GetName())
			return *m_Temperaments[i];

	/* else return original temperament */
	return *m_Temperaments[0];
}

ptr_vector<GOrgueTemperament> GOrgueTemperament::m_Temperaments;

void GOrgueTemperament::InitTemperaments()
{
	if (m_Temperaments.size())
		return;

	m_Temperaments.push_back(new GOrgueTemperament(wxTRANSLATE("Original temperament")));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Equal temperament"), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
}

