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

#ifndef GOSOUNDFADER_H_
#define GOSOUNDFADER_H_

#include <assert.h>
#include <math.h>

typedef struct
{
	float                      gain;
	float                      gain_attack;
	float                      gain_decay;
	float                      gain_target;
	unsigned                   faderemain;
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
	float gain_delta = fader_state->gain_attack + fader_state->gain_decay;
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
				fader_state->gain_decay = 0.0f;
			}
			else if (gain > fader_state->gain_target)
			{
				gain = fader_state->gain_target;
				fader_state->gain_attack = 0.0f;
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

	if (fader_state->gain_attack > 0.0f)
	{
		if (fader_state->faderemain >= n_blocks)
			fader_state->faderemain -= n_blocks;
		else
			fader_state->gain_attack = 0.0f;
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
	fader_state->gain_decay =
			-scalbnf(fader_state->gain_target
			        ,duration_shift /* results in approx 0.37s maximum decay length */
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
	fader_state->gain         = 0.0f;
	fader_state->gain_target = target_gain;
	fader_state->gain_attack =
			scalbnf(target_gain
			       ,duration_shift
			       );
	fader_state->gain_decay = 0.0f;
	fader_state->faderemain = max_fadein_frames;
}

static
inline
void FaderNewConstant
	(GOSoundFader    *fader_state
	,float            gain
	)
{
	fader_state->faderemain = 0;
	fader_state->gain_attack = fader_state->gain_decay = 0.0f;
	fader_state->gain = fader_state->gain_target = gain;
}

#endif /* GOSOUNDFADER_H_ */
