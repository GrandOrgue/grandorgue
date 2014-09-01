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

#include "GOSoundProviderSynthedTrem.h"

#include "GOrgueMemoryPool.h"

GOSoundProviderSynthedTrem::GOSoundProviderSynthedTrem(GOrgueMemoryPool& pool) :
	GOSoundProvider(pool)
{
	m_Gain = 1.0f;
}

inline
short SynthTrem(double amp, double angle)
{
	return (short)(amp * sin(angle));
}

inline
short SynthTrem(double amp, double angle, double fade)
{
	if (fade > 1.0)
		fade = 1.0;
	if (fade < 0.0)
		fade = 0.0;
	return (short)(fade * amp * sin(angle));
}

void GOSoundProviderSynthedTrem::Create
	(int period
	,int start_rate
	,int stop_rate
	,int amp_mod_depth
	)
{

	ClearData();

	const double trem_freq         = 1000.0 / period;
	const int sample_freq          = 44100;

	const unsigned attack_samples  = sample_freq / start_rate;
	const unsigned loop_samples    = sample_freq / trem_freq;
	const unsigned release_samples = sample_freq / stop_rate;
	const unsigned total_samples   = attack_samples + loop_samples + release_samples;

	/* Synthesize tremulant */
	const double pi                = 3.14159265358979323846;
	const double trem_amp          = (0x7FF0 * amp_mod_depth / 100);
	const double trem_param        = 2 * pi  / loop_samples;
	double trem_angle              = 0.0;

	wxInt16 *data = (wxInt16*)malloc(total_samples * sizeof(wxInt16));
	if (!data)
		throw GOrgueOutOfMemory();

	wxInt16 *write_iterator = data;

	/* Synthesize attack */
	double trem_fade = 0.0;
	double trem_inc = 1.0 / attack_samples;
	for (unsigned i = 0
	    ;i < attack_samples
	    ;i++
	    ,write_iterator++
	    )
	{
		*write_iterator  = SynthTrem(trem_amp, trem_angle, trem_fade);
		trem_angle      += trem_param * trem_fade;
		trem_fade       += trem_inc;
	}

	/* Synthesize loop */
	for (unsigned i = 0
	    ;i < loop_samples
	    ;i++
	    ,write_iterator++
	    )
	{
		*write_iterator  = SynthTrem(trem_amp, trem_angle);
		trem_angle      += trem_param;
	}

	/* Synthesize release */
	trem_inc = 1.0 / (double)release_samples;
	trem_fade = 1.0 - trem_inc;
	for (unsigned i = 0
	    ;i < release_samples
	    ;i++
	    ,write_iterator++
	    )
	{
		*write_iterator  = SynthTrem(trem_amp, trem_angle, trem_fade);
		trem_angle      += trem_param * trem_fade;
		trem_fade       -= trem_inc;
	}

	/* Attack sustain section */
	GO_WAVE_LOOP trem_loop;
	trem_loop.start_sample = attack_samples;
	trem_loop.end_sample   = (attack_samples + loop_samples) - 1;
	std::vector<GO_WAVE_LOOP> trem_loops;
	trem_loops.push_back(trem_loop);
	attack_section_info attack_info;
	attack_info.sample_group = -1;
	attack_info.min_attack_velocity = 0;
	m_AttackInfo.push_back(attack_info);
	m_Attack.push_back(new GOAudioSection(m_pool));
	m_Attack[0]->Setup(data, GOrgueWave::SF_SIGNEDSHORT_16, 1, sample_freq, trem_loop.end_sample, &trem_loops, false, 0);

	/* Release section */
	release_section_info release_info;
	release_info.sample_group = -1;
	release_info.max_playback_time = -1;
	m_ReleaseInfo.push_back(release_info);
	m_Release.push_back(new GOAudioSection(m_pool));
	m_Release[0]->Setup(&data[attack_samples + loop_samples], GOrgueWave::SF_SIGNEDSHORT_16, 1, sample_freq, release_samples, NULL, false, 0);

	free(data);

	ComputeReleaseAlignmentInfo();

}
