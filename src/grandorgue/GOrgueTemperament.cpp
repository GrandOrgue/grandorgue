/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2012 GrandOrgue contributors (see AUTHORS)
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

float GOrgueTemperament::GetOffset(bool ignorepitch, unsigned midi_number, unsigned wav_midi_number, float wav_pitch_fract, float harmonic_number, float pitch_correction, float default_tuning) const
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
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("1/4 comma Mean tone"), 10.26, -13.69, 3.42, 20.52, -3.43, 13.68, -10.27, 6.84, -17.11, 0, 17.1, -6.85));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Bach (Bradley Lehman)"), +5.9, +3.9, +2, +3.9, -1.9, +7.9, +2, +3.9, +3.9, 0, +3.9, 0));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Chaumont"), 10.265, -13.686, 3.422, 1.174, -3.421, 13.687, -10.264, 6.843, -17.108, 0, 7.43, -6.843));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Corrette (1753)"), 11.73001, -15.64, 3.91001, 7.82001, -3.909996923, 15.64001, -11.73, 7.82001, -11.73, 0, 11.73001, -7.82));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Couperin"), 10.26471, -13.68629, 3.42157, 7.36758, -3.42129, 13.68628, -10.26472, 6.84314, -6.35472, 0, 17.10785, -6.84315));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Kirnberger III"), 10.26, 0.44, 3.42, 4.39, -3.43, 8.3, 0.48, 6.84, 2.44, 0, 6.35, -1.47));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Norden"), 8.211, -6.25601, 2.737, 2.346, -2.737, 10.948, -8.21101, 5.474, -4.30101, 0, 8.993, -5.104));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Pythagorean"), -5.87, 7.82, -1.96, -11.74, 1.95, -7.83, 5.86, -3.92, 9.77, 0, -9.78, 3.91));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Stade"), 10.26471, -8.30972, 3.42157, 0, -3.42129, 8.30971, -10.26472, 6.84314, -6.35472, 0, 6.35471, -6.84315));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Valotti"), 6.13, -0.18, 2.04, 4.26, -2.05, 8.17, -2.14, 4.09, 2.31, 0, 6.22, -4.09));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Werckmeister III"), 11.73, 1.95, 3.91, 5.87, 1.96, 9.78, 0, 7.82, 3.91, 0, 7.82, 3.91));
}

