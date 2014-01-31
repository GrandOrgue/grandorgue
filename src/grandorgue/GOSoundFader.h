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

#ifndef GOSOUNDFADER_H_
#define GOSOUNDFADER_H_

#include <assert.h>
#include <math.h>

class GOSoundFader
{
	typedef struct
	{
		float gain;
		float gain_delta;
	} FaderState;
private:
	float       m_gain;
	float       m_attack;
	float       m_decay;
	float       m_target;
	float       m_VelocityVolume;
	float       m_real_target;
	float       m_last_volume;
	unsigned    m_nb_attack_frames_left;

public:
	void NewAttacking(float target_gain, int duration_shift, int max_fadein_frames);
	void NewConstant(float gain);
	void StartDecay(int duration_shift);
	bool IsSilent();
	void SetVelocityVolume(float volume);

	FaderState SetupProcess(unsigned n_blocks, float volume);
	void ProcessData(FaderState& state, unsigned n_blocks, float *buffer);

	void Process(unsigned n_blocks, float *buffer, float volume);
};

inline
GOSoundFader::FaderState GOSoundFader::SetupProcess(unsigned n_blocks, float volume)
{
	volume *= m_VelocityVolume;

	if (m_last_volume < 0)
	{
		m_last_volume = volume;
		m_real_target = m_target * volume;
		m_gain *= volume;
	}
	float gain = m_gain;
	float gain_delta = 0;
	if (volume != m_last_volume || m_attack + m_decay != 0)
	{
		float volume_diff = m_target * (volume - m_last_volume) * n_blocks / MAX_FRAME_SIZE;
		float fade_diff = n_blocks * (m_attack + m_decay) * volume;
		float new_last_volume = m_last_volume + ((volume - m_last_volume) * n_blocks) / MAX_FRAME_SIZE;
		m_real_target = m_target * new_last_volume;

		float end = m_gain + volume_diff + fade_diff;
		if (end < 0)
		{
			end = 0;
			m_decay = 0;
		}
		else if (end > m_real_target)
		{
			end = m_real_target;
			m_attack = 0.0f;
		}
		gain_delta = (end - m_gain) / (n_blocks);
		m_last_volume = new_last_volume;
		m_gain = end;
	}
	if (m_attack > 0.0f)
	{
		if (m_nb_attack_frames_left >= n_blocks)
			m_nb_attack_frames_left -= n_blocks;
		else
			m_attack = 0.0f;
	}
	return {gain, gain_delta};
}

inline
void GOSoundFader::ProcessData(FaderState& state, unsigned n_blocks, float *buffer)
{
	if (state.gain_delta)
	{
		for(unsigned int i = 0; i < n_blocks; i++, buffer += 2)
		{
			buffer[0] *= state.gain;
			buffer[1] *= state.gain;
			state.gain += state.gain_delta;
		}
	}
	else
	{

		for(unsigned int i = 0; i < n_blocks; i++, buffer += 2)
		{
			buffer[0] *= state.gain;
			buffer[1] *= state.gain;
		}
	}
}

inline
void GOSoundFader::Process(unsigned n_blocks, float *buffer, float volume)
{
	FaderState state = SetupProcess(n_blocks, volume);
	ProcessData(state, n_blocks, buffer);
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
	m_VelocityVolume = 1;
	
	m_attack = scalbnf(target_gain, duration_shift);
}

inline
void GOSoundFader::NewConstant(float gain)
{
	m_nb_attack_frames_left = 0;
	m_attack = m_decay = 0.0f;
	m_gain = m_target = gain;
	m_last_volume = -1;
	m_VelocityVolume = 1;
}

inline
void GOSoundFader::SetVelocityVolume(float volume)
{
	m_VelocityVolume = volume;
}

#endif /* GOSOUNDFADER_H_ */
