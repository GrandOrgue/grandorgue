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

#include "GOrgueTemperamentCent.h"

GOrgueTemperamentCent::GOrgueTemperamentCent(wxString name, float i1, float i2, float i3, float i4, float i5, float i6, float i7, float i8, float i9, float i10, float i11, float i12) :
	GOrgueTemperament(name)
{
	m_Tuning[0] = i1;
	m_Tuning[1] = i2;
	m_Tuning[2] = i3;
	m_Tuning[3] = i4;
	m_Tuning[4] = i5;
	m_Tuning[5] = i6;
	m_Tuning[6] = i7;
	m_Tuning[7] = i8;
	m_Tuning[8] = i9;
	m_Tuning[9] = i10;
	m_Tuning[10] = i11;
	m_Tuning[11] = i12;
}

float GOrgueTemperamentCent::GetOffset(unsigned midi_number, unsigned wav_midi_number, float wav_pitch_fract, float harmonic_number, float default_tuning) const
{
	double val = wav_pitch_fract;
	if (wav_midi_number)
		val += 100.0 * wav_midi_number - 100.0 * midi_number + log(8.0 / harmonic_number) / log(2) * 1200;
	return m_Tuning[midi_number % 12] - val - default_tuning;
}

