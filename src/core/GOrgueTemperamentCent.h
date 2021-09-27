/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
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
	GOrgueTemperamentCent(wxString name, wxString title, wxString group);

	virtual float GetOffset(bool ignorepitch, unsigned midi_number, unsigned wav_midi_number, float wav_pitch_fract, float harmonic_number, float pitch_correction, float default_tuning) const;
};

#endif
