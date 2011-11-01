/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef GORGUERELEASEALIGNTABLE_H_
#define GORGUERELEASEALIGNTABLE_H_

#include "GOSoundSampler.h"

#define PHASE_ALIGN_DERIVATIVES    2
#define PHASE_ALIGN_AMPLITUDES     32
#define PHASE_ALIGN_MIN_FREQUENCY  20 /* Hertz */

class GOrgueCache;
class GOrgueCacheWriter;

class GOrgueReleaseAlignTable
{

private:

	unsigned int m_Channels;
	int m_PhaseAlignMaxAmplitude;
	int m_PhaseAlignMaxDerivative;
	int m_PositionEntries[PHASE_ALIGN_DERIVATIVES][PHASE_ALIGN_AMPLITUDES];
	int m_HistoryEntries[PHASE_ALIGN_DERIVATIVES][PHASE_ALIGN_AMPLITUDES][BLOCK_HISTORY * MAX_OUTPUT_CHANNELS];

public:

	GOrgueReleaseAlignTable();
	~GOrgueReleaseAlignTable();

	bool Load(GOrgueCache& cache);
	bool Save(GOrgueCacheWriter& cache);

	void ComputeTable
		(const AUDIO_SECTION_T& m_release
		,const int phase_align_max_amplitude
		,const int phase_align_max_derivative
		,const unsigned int sample_rate
		,const unsigned int channels
		);

	void SetupRelease
		(GO_SAMPLER_T& release_sampler
		,const GO_SAMPLER_T& old_sampler
		) const;

};

#endif /* GORGUERELEASEALIGNTABLE_H_ */
