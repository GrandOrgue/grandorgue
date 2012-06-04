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

#ifndef GORGUETEMPERAMENTCENT_H
#define GORGUETEMPERAMENTCENT_H

#include "GOrgueTemperament.h"

class GOrgueTemperamentCent : public GOrgueTemperament
{
protected:
	float m_Tuning[12];

public:
	GOrgueTemperamentCent(wxString name, float i1, float i2, float i3, float i4, float i5, float i6, float i7, float i8, float i9, float i10, float i11, float i12);
	GOrgueTemperamentCent(wxString name, wxString group, float i1, float i2, float i3, float i4, float i5, float i6, float i7, float i8, float i9, float i10, float i11, float i12);

	virtual float GetOffset(bool ignorepitch, unsigned midi_number, unsigned wav_midi_number, float wav_pitch_fract, float harmonic_number, float pitch_correction, float default_tuning) const;
};

#endif
