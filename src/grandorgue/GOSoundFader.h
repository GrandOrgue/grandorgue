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
 */

#ifndef GOSOUNDFADER_H_
#define GOSOUNDFADER_H_

#include <assert.h>
#include <math.h>

class GOSoundFader
{
private:
	float       m_gain;
	float       m_attack;
	float       m_decay;
	float       m_target;
	float       m_real_target;
	float       m_last_volume;
	unsigned    m_nb_attack_frames_left;

public:
	void NewAttacking(float target_gain, int duration_shift, int max_fadein_frames);
	void NewConstant(float gain);
	void StartDecay(int duration_shift);
	bool IsSilent();

	void Process(unsigned n_blocks, float *decoded_sampler_audio_frame, float volume);
};

inline
void GOSoundFader::Process(unsigned n_blocks, float *decoded_sampler_audio_frame, float volume)
{
	if (m_last_volume < 0)
	{
		m_last_volume = volume;
		m_real_target = m_target * volume;
		m_gain *= volume;
	}

	/* Multiply each of the buffer samples by the fade factor - note:
	 * FADE IS NEGATIVE. A positive fade would indicate a gain of zero.
	 * Note: this for loop has been split by an if to aide the vectorizer.
	 */
	float gain_delta = m_attack + m_decay;
	float gain = m_gain;
	float target = m_real_target;
	if (volume != m_last_volume)
	{
		gain_delta *= volume;
		gain_delta += m_target * (volume - m_last_volume) / 1024;
		float new_last_volume = m_last_volume + ((volume - m_last_volume) * n_blocks) / 1024;
		if (volume > m_last_volume)
		{
			target = m_target * volume;
			m_real_target = target;
		}
		else
		{
			target = m_target * new_last_volume;
		}
		m_last_volume = new_last_volume;
	}
	else
		gain_delta *= volume;
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
				m_decay = 0.0f;
			}
			else if (gain > m_real_target)
			{
				gain = m_real_target;
				m_attack = 0.0f;
			}

		}

		m_gain = gain;
		m_real_target = target;
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

	if (m_attack > 0.0f)
	{
		if (m_nb_attack_frames_left >= n_blocks)
			m_nb_attack_frames_left -= n_blocks;
		else
			m_attack = 0.0f;
	}

}

inline
void GOSoundFader::StartDecay(int duration_shift)
{
	assert(duration_shift < 0);
	m_decay = -scalbnf(m_target, duration_shift);
}

inline
bool GOSoundFader::IsSilent()
{
	return (m_gain <= 0.0f);
}

inline
void GOSoundFader::NewAttacking(float target_gain, int duration_shift, int max_fadein_frames)
{
	assert(duration_shift < 0);
	m_nb_attack_frames_left = max_fadein_frames;
	m_decay  = 0.0f;
	m_gain   = 0.0f;
	m_target = target_gain;
	m_last_volume = -1;
	m_attack = scalbnf(target_gain, duration_shift);
}

inline
void GOSoundFader::NewConstant(float gain)
{
	m_nb_attack_frames_left = 0;
	m_attack = m_decay = 0.0f;
	m_gain = m_target = gain;
	m_last_volume = -1;
}

#endif /* GOSOUNDFADER_H_ */
