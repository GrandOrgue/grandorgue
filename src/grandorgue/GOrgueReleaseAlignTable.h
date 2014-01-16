/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2013 GrandOrgue contributors (see AUTHORS)
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

#ifndef GORGUERELEASEALIGNTABLE_H_
#define GORGUERELEASEALIGNTABLE_H_

#include "GOSoundAudioSection.h"

class GOrgueCache;
class GOrgueCacheWriter;

#define PHASE_ALIGN_DERIVATIVES    2
#define PHASE_ALIGN_AMPLITUDES     32
#define PHASE_ALIGN_MIN_FREQUENCY  20 /* Hertz */

class GOrgueReleaseAlignTable
{

private:

	int m_PhaseAlignMaxAmplitude;
	int m_PhaseAlignMaxDerivative;
	int m_PositionEntries[PHASE_ALIGN_DERIVATIVES][PHASE_ALIGN_AMPLITUDES];

public:

	GOrgueReleaseAlignTable();
	~GOrgueReleaseAlignTable();

	bool Load(GOrgueCache& cache);
	bool Save(GOrgueCacheWriter& cache);

	void ComputeTable
		(const GOAudioSection &m_release
		,int                   phase_align_max_amplitude
		,int                   phase_align_max_derivative
		,unsigned int          sample_rate
		,unsigned              start_position
		);

	void SetupRelease
		(audio_section_stream       &release_sampler
		,const audio_section_stream &old_sampler
		) const;

};

#endif /* GORGUERELEASEALIGNTABLE_H_ */
