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

#ifndef GOSOUNDFADER_H_
#define GOSOUNDFADER_H_

#include <assert.h>
#include <math.h>

typedef struct
{
	float       gain;
	float       attack;
	float       decay;
	float       target;
	unsigned    nb_attack_frames_left;
} GOSoundFader;

static
inline
void FaderProcess
	(GOSoundFader    *fader_state
	,unsigned int     n_blocks
	,float           *decoded_sampler_audio_frame
	)
{

	/* Multiply each of the buffer samples by the fade factor - note:
	 * FADE IS NEGATIVE. A positive fade would indicate a gain of zero.
	 * Note: this for loop has been split by an if to aide the vectorizer.
	 */
	float gain_delta = fader_state->attack + fader_state->decay;
	float gain = fader_state->gain;
	if (gain_delta)
	{

		for(unsigned int i = 0; i < n_blocks / 2; i++, decoded_sampler_audio_frame += 4)
		{

			decoded_sampler_audio_frame[0] *= gain;
			decoded_sampler_audio_frame[1] *= gain;
			decoded_sampler_audio_frame[2] *= gain;
			decoded_sampler_audio_frame[3] *= gain;

			gain += gain_delta;
			if (gain < 0.0f)
			{
				gain = 0.0f;
				fader_state->decay = 0.0f;
			}
			else if (gain > fader_state->target)
			{
				gain = fader_state->target;
				fader_state->attack = 0.0f;
			}

		}

		fader_state->gain = gain;

	}
	else
	{

		for(unsigned int i = 0; i < n_blocks / 2; i++, decoded_sampler_audio_frame += 4)
		{

			decoded_sampler_audio_frame[0] *= gain;
			decoded_sampler_audio_frame[1] *= gain;
			decoded_sampler_audio_frame[2] *= gain;
			decoded_sampler_audio_frame[3] *= gain;

		}

	}

	if (fader_state->attack > 0.0f)
	{
		if (fader_state->nb_attack_frames_left >= n_blocks)
			fader_state->nb_attack_frames_left -= n_blocks;
		else
			fader_state->attack = 0.0f;
	}

}

static
inline
void FaderStartDecay
	(GOSoundFader    *fader_state
	,int              duration_shift
	)
{
	assert(duration_shift < 0);
	fader_state->decay =
			-scalbnf(fader_state->target
			        ,duration_shift
			        );
}

static
inline
bool FaderIsSilent
	(GOSoundFader    *fader_state)
{
	return (fader_state->gain <= 0.0f);
}

static
inline
void FaderNewAttacking
	(GOSoundFader    *fader_state
	,float            target_gain
	,int              duration_shift
	,int              max_fadein_frames
	)
{
	assert(duration_shift < 0);
	fader_state->nb_attack_frames_left = max_fadein_frames;
	fader_state->decay  = 0.0f;
	fader_state->gain   = 0.0f;
	fader_state->target = target_gain;
	fader_state->attack =
			scalbnf(target_gain
			       ,duration_shift
			       );
}

static
inline
void FaderNewConstant
	(GOSoundFader    *fader_state
	,float            gain
	)
{
	fader_state->nb_attack_frames_left = 0;
	fader_state->attack = fader_state->decay = 0.0f;
	fader_state->gain = fader_state->target = gain;
}

#endif /* GOSOUNDFADER_H_ */
