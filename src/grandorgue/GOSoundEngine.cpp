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

#include "GOSoundEngine.h"
#include "GOSoundProvider.h"
#include "GOSoundSampler.h"
#include "GOrgueEvent.h"
#include "GOrgueInt24.h"
#include "GOrguePipe.h"
#include "GOrgueReleaseAlignTable.h"
#include "GOrgueWindchest.h"
#include "GrandOrgueFile.h"

/* This parameter determines the cross fade length. The length in samples
 * will be:
 *                    2 ^ (CROSSFADE_LEN_BITS + 1)
 * Reasonable durations
 *   8   - 512 samples    (approx 12ms @ 44.1kHz)
 *   9   - 1024 samples   (approx 23ms @ 44.1kHz)
 *   10  - 2048 samples   (approx 46ms @ 44.1kHz)
 */
#define CROSSFADE_LEN_BITS (8)

GOSoundEngine::GOSoundEngine() :
	m_PolyphonyLimiting(true),
	m_ScaledReleases(true),
	m_ReleaseAlignmentEnabled(true),
	m_RandomizeSpeaking(true),
	m_Volume(100),
	m_SampleRate(0),
	m_CurrentTime(0),
	m_SamplerPool(),
	m_DetachedRelease(1),
	m_Windchests(),
	m_Tremulants()
{
	memset(&m_ResamplerCoefs, 0, sizeof(m_ResamplerCoefs));
	m_SamplerPool.SetUsageLimit(2048);
	m_PolyphonySoftLimit = (m_SamplerPool.GetUsageLimit() * 3) / 4;
	Reset();
}

void GOSoundEngine::Reset()
{
	for (unsigned i = 0; i < m_Windchests.size(); i++)
	{
		m_Windchests[i].new_sampler = 0;
		m_Windchests[i].sampler = 0;
		m_Windchests[i].count = 0;
	}
	for (unsigned i = 0; i < m_Tremulants.size(); i++)
	{
		m_Tremulants[i].new_sampler = 0;
		m_Tremulants[i].sampler = 0;
		m_Tremulants[i].count = 0;
	}
	for (unsigned i = 0; i < m_DetachedRelease.size(); i++)
	{
		m_DetachedRelease[i].new_sampler = 0;
		m_DetachedRelease[i].sampler = 0;
		m_DetachedRelease[i].count = 0;
	}
	m_SamplerPool.ReturnAll();
	m_CurrentTime = 0;
	ResetDoneFlags();
}

void GOSoundEngine::SetVolume(int volume)
{
	m_Volume = volume;
}

void GOSoundEngine::SetSampleRate(unsigned sample_rate)
{
	m_SampleRate = sample_rate;
	resampler_coefs_init(&m_ResamplerCoefs, m_SampleRate, m_ResamplerCoefs.interpolation);
}

void GOSoundEngine::SetInterpolationType(unsigned type)
{
	m_ResamplerCoefs.interpolation = (interpolation_type) type;
}

unsigned GOSoundEngine::GetSampleRate()
{
	return m_SampleRate;
}

void GOSoundEngine::SetHardPolyphony(unsigned polyphony)
{
	m_SamplerPool.SetUsageLimit(polyphony);
	m_PolyphonySoftLimit = (m_SamplerPool.GetUsageLimit() * 3) / 4;
}

void GOSoundEngine::SetPolyphonyLimiting(bool limiting)
{
	m_PolyphonyLimiting = limiting;
}

unsigned GOSoundEngine::GetHardPolyphony() const
{
	return m_SamplerPool.GetUsageLimit();
}

int GOSoundEngine::GetVolume() const
{
	return m_Volume;
}

void GOSoundEngine::SetScaledReleases(bool enable)
{
	m_ScaledReleases = enable;
}

void GOSoundEngine::SetRandomizeSpeaking(bool enable)
{
	m_RandomizeSpeaking = enable;
}

float GOSoundEngine::GetRandomFactor()
{
	if (m_RandomizeSpeaking)
	{
		const double factor = (pow(2, 2.0 / 1200.0) - 1) / (RAND_MAX / 2);
		int num = rand() - RAND_MAX / 2;
		return  1 + num * factor;
	}
	return 1;
}

void GOSoundEngine::StartSampler(GO_SAMPLER* sampler, int sampler_group_id)
{
	GOSamplerEntry* state;

	if (sampler_group_id == 0)
		state = &m_DetachedRelease[0];
	else if (sampler_group_id < 0)
		state = &m_Tremulants[-1-sampler_group_id];
	else if (sampler_group_id > (int) m_Windchests.size())
		state = &m_DetachedRelease[sampler_group_id - m_Windchests.size()];
	else
		state = &m_Windchests[sampler_group_id - 1];

	wxCriticalSectionLocker locker(state->lock);
	sampler->sampler_group_id = sampler_group_id;
	sampler->next = state->new_sampler;
	state->new_sampler = sampler;
	state->count++;
}

void GOSoundEngine::Setup(GrandOrgueFile* organ_file, unsigned release_count)
{
	m_Windchests.resize(organ_file->GetWinchestGroupCount());
	if (release_count < 1)
		release_count = 1;
	m_DetachedRelease.resize(release_count);
	for (unsigned i = 0; i < m_Windchests.size(); i++)
		m_Windchests[i].windchest = organ_file->GetWindchest(i);
	m_Tremulants.resize(organ_file->GetTremulantCount());
	Reset();
}

inline
void GOSoundEngine::ReadSamplerFrames
	(GO_SAMPLER* sampler
	,unsigned int n_blocks
	,float* decoded_sampler_audio_frame
	)
{

	for (unsigned int i = 0
	    ;i < n_blocks
	    ;i += BLOCKS_PER_FRAME
	    ,decoded_sampler_audio_frame += BLOCKS_PER_FRAME * 2
	    )
	{

		if (!sampler->pipe)
		{
			std::fill
				(decoded_sampler_audio_frame
				,decoded_sampler_audio_frame + (BLOCKS_PER_FRAME * 2)
				,0.0f
				);
			continue;
		}

		if (!GOAudioSection::ReadBlock(&sampler->stream, decoded_sampler_audio_frame))
		{
			sampler->pipe = NULL;
		}

	}

}

void GOSoundEngine::ProcessAudioSamplers(GOSamplerEntry& state, unsigned int n_frames, bool tremulant)
{
	{
		wxCriticalSectionLocker locker(state.lock);
		if (state.new_sampler)
		{
			GO_SAMPLER* new_sampler = state.new_sampler;
			while(new_sampler->next)
				new_sampler = new_sampler->next;
			new_sampler->next = state.sampler;
			state.sampler = state.new_sampler;
			state.new_sampler = 0;
		}
	}

	if (state.sampler == NULL)
		return;

	assert((n_frames & (BLOCKS_PER_FRAME - 1)) == 0);
	assert(n_frames > BLOCKS_PER_FRAME);
	float* output_buffer = state.buff;
	std::fill(output_buffer, output_buffer + n_frames * 2, tremulant ? 1.0f : 0.0f);
	GO_SAMPLER* previous_sampler = NULL, *next_sampler = NULL;
	for (GO_SAMPLER* sampler = state.sampler; sampler; sampler = next_sampler)
	{

		const bool process_sampler = (sampler->time <= m_CurrentTime);
		if (process_sampler)
		{

			if  (
					(m_PolyphonyLimiting) &&
					(m_SamplerPool.UsedSamplerCount() >= m_PolyphonySoftLimit) &&
					(sampler->stream.audio_section == sampler->pipe->GetRelease()) &&
					(m_CurrentTime - sampler->time > 172)
				)
				FaderStartDecay(&sampler->fader, -13); /* Approx 0.37s at 44.1kHz */

			/* The decoded sampler frame will contain values containing
			 * sampler->pipe_section->sample_bits worth of significant bits.
			 * It is the responsibility of the fade engine to bring these bits
			 * back into a sensible state. This is achieved during setup of the
			 * fade parameters. The gain target should be:
			 *
			 *     playback gain * (2 ^ -sampler->pipe_section->sample_bits)
			 */
			ReadSamplerFrames
				(sampler
				,n_frames
				,state.temp
				);

			FaderProcess
				(&sampler->fader
				,n_frames
				,state.temp
				);

			if (sampler->stop)
			{
				CreateReleaseSampler(sampler);

				/* The above code created a new sampler to playback the release, the
				 * following code takes the active sampler for this pipe (which will be
				 * in either the attack or loop section) and sets the fadeout property
				 * which will decay this portion of the pipe. The sampler will
				 * automatically be placed back in the pool when the fade restores to
				 * zero. */
				FaderStartDecay(&sampler->fader, -CROSSFADE_LEN_BITS);
				sampler->stop = false;
			}

			/* Add these samples to the current output buffer shifting
			 * right by the necessary amount to bring the sample gain back
			 * to unity (this value is computed in GOrguePipe.cpp)
			 */
			float* write_iterator = output_buffer;
			float* decode_pos = state.temp;
			for(unsigned int i = 0; i < n_frames / 2; i++, write_iterator += 4, decode_pos += 4)
			{
				write_iterator[0] += decode_pos[0];
				write_iterator[1] += decode_pos[1];
				write_iterator[2] += decode_pos[2];
				write_iterator[3] += decode_pos[3];
			}

		}

		next_sampler = sampler->next;
		/* if this sampler's pipe has been set to null or the fade value is
		 * zero, the sample is no longer required and can be removed from the
		 * linked list. If it was still supplying audio, we must update the
		 * previous valid sampler. */
		if (!sampler->pipe || (FaderIsSilent(&sampler->fader) && process_sampler))
		{
			/* sampler needs to be removed from the list */
			if (sampler == state.sampler)
				state.sampler = sampler->next;
			else
				previous_sampler->next = sampler->next;
			m_SamplerPool.ReturnSampler(sampler);
			state.count--;
		}
		else
			previous_sampler = sampler;

	}

	if (!tremulant)
	{
		float f = m_Volume * 0.01f;
		if (state.windchest)
			f *= state.windchest->GetVolume();
		for (unsigned int i = 0; i < n_frames * 2; i++)
			output_buffer[i] *= f;
	}

	state.done = true;
}

void GOSoundEngine::Process(unsigned sampler_group_id, unsigned n_frames)
{
	GOSamplerEntry* state;

	if (sampler_group_id == 0)
		state = &m_DetachedRelease[0];
	else if (sampler_group_id > m_Windchests.size())
		state = &m_DetachedRelease[sampler_group_id - m_Windchests.size()];
	else
		state = &m_Windchests[sampler_group_id - 1];

	if (state->sampler == NULL && state->new_sampler == NULL)
		return;

	wxCriticalSectionLocker locker(state->mutex);

	if (state->done)
		return;

	ProcessAudioSamplers(*state, n_frames, false);
}

void GOSoundEngine::ResetDoneFlags()
{
	for (unsigned j = 0; j < m_Windchests.size(); j++)
	{
		wxCriticalSectionLocker locker(m_Windchests[j].mutex);
		m_Windchests[j].done = false;
	}
	for (unsigned j = 0; j < m_DetachedRelease.size(); j++)
	{
		wxCriticalSectionLocker locker(m_DetachedRelease[j].mutex);
		m_DetachedRelease[j].done = false;
	}
}

// this callback will fill the buffer with bufferSize frame
// audio is opened with 32 bit stereo, so one frame is 64bit (32bit for right 32bit for left)
// So buffer can handle 8*bufferSize char (or 2*bufferSize float)
int GOSoundEngine::GetSamples
	(float      *output_buffer
	,unsigned    n_frames
	,double      stream_time
	,METER_INFO *meter_info
	)
{
	/* initialise the output buffer */
	std::fill(m_FinalBuffer, m_FinalBuffer + GO_SOUND_BUFFER_SIZE, 0.0f);

	for (unsigned j = 0; j < m_Tremulants.size(); j++)
		ProcessAudioSamplers(m_Tremulants[j], n_frames, true);

	for (unsigned j = 0; j < m_Windchests.size(); j++)
	{

		float* this_buff = m_Windchests[j].buff;

		wxCriticalSectionLocker locker(m_Windchests[j].mutex);

		if (!m_Windchests[j].done)
			ProcessAudioSamplers(m_Windchests[j], n_frames, false);

		if (!m_Windchests[j].done)
			continue;

		GOrgueWindchest* current_windchest = m_Windchests[j].windchest;
		for (unsigned i = 0; i < current_windchest->GetTremulantCount(); i++)
		{
			unsigned tremulant_pos = current_windchest->GetTremulantId(i);
			if (m_Tremulants[tremulant_pos].done)
			{
				const float *ptr = m_Tremulants[tremulant_pos].buff;
				for (unsigned int k = 0; k < n_frames * 2; k++)
					this_buff[k] *= ptr[k];
			}
		}
		for (unsigned int k = 0; k < n_frames * 2; k++)
			m_FinalBuffer[k] += this_buff[k];
	}

	for (unsigned j = 0; j < m_DetachedRelease.size(); j++)
	{
		float* this_buff = m_DetachedRelease[j].buff;

		wxCriticalSectionLocker locker(m_DetachedRelease[j].mutex);

		if (!m_DetachedRelease[j].done)
			ProcessAudioSamplers(m_DetachedRelease[j], n_frames, false);

		if (!m_DetachedRelease[j].done)
			continue;

		for (unsigned int k = 0; k < n_frames * 2; k++)
			m_FinalBuffer[k] += this_buff[k];
	}

	m_CurrentTime += n_frames / BLOCKS_PER_FRAME;

	/* Clamp the output */
	static const float CLAMP_MIN = -1.0f;
	static const float CLAMP_MAX = 1.0f;
	if (meter_info)
	{
		unsigned used_samplers = m_SamplerPool.UsedSamplerCount();
		if (used_samplers > meter_info->current_polyphony)
			meter_info->current_polyphony = used_samplers;
		for (unsigned int k = 0; k < n_frames * 2; k += 2)
		{
			float f = std::min(std::max(m_FinalBuffer[k + 0], CLAMP_MIN), CLAMP_MAX);
			output_buffer[k + 0] = f;
			meter_info->meter_left  = (meter_info->meter_left > f)
			                        ? meter_info->meter_left
			                        : f;
			f = std::min(std::max(m_FinalBuffer[k + 1], CLAMP_MIN), CLAMP_MAX);
			output_buffer[k + 1] = f;
			meter_info->meter_right = (meter_info->meter_right > f)
			                        ? meter_info->meter_right
			                        : f;
		}
	}
	else
	{
		for (unsigned int k = 0; k < n_frames * 2; k += 2)
		{
			float f = std::min(std::max(m_FinalBuffer[k + 0], CLAMP_MIN), CLAMP_MAX);
			output_buffer[k + 0] = f;
			f = std::min(std::max(m_FinalBuffer[k + 1], CLAMP_MIN), CLAMP_MAX);
			output_buffer[k + 1] = f;
		}
	}

	ResetDoneFlags();

	return 0;
}


SAMPLER_HANDLE GOSoundEngine::StartSample(const GOSoundProvider* pipe, int sampler_group_id)
{
	if (pipe->GetAttack()->GetChannels() == 0)
		return NULL;
	GO_SAMPLER* sampler = m_SamplerPool.GetSampler();
	if (sampler)
	{
		sampler->pipe = pipe;
		pipe->GetAttack()->InitStream
			(&m_ResamplerCoefs
			,&sampler->stream
			,GetRandomFactor() * pipe->GetTuning() / (float)m_SampleRate
			);
		const float playback_gain = pipe->GetGain() * pipe->GetAttack()->GetNormGain();
		FaderNewConstant(&sampler->fader, playback_gain);
		sampler->time = m_CurrentTime;
		StartSampler(sampler, sampler_group_id);
	}
	return sampler;
}

void GOSoundEngine::CreateReleaseSampler(const GO_SAMPLER* handle)
{
	if (!handle->pipe)
		return;

	const GOSoundProvider* this_pipe = handle->pipe;
	float vol = (handle->sampler_group_id < 0)
	          ? 1.0f
	          : m_Windchests[handle->sampler_group_id - 1].windchest->GetVolume();

	// FIXME: this is wrong... the intention is to not create a release for a
	// sample being played back with zero amplitude but this is a comparison
	// against a double. We should test against a minimum level.
	if (vol)
	{
		GO_SAMPLER* new_sampler = m_SamplerPool.GetSampler();
		if (new_sampler != NULL)
		{

			const GOAudioSection* release_section = this_pipe->GetRelease();
			new_sampler->pipe = this_pipe;
			new_sampler->time = m_CurrentTime + 1;

			int gain_decay_rate = 0;
			float gain_target = this_pipe->GetGain() * release_section->GetNormGain();
			const bool not_a_tremulant = (handle->sampler_group_id >= 0);
			if (not_a_tremulant)
			{
				/* Because this sampler is about to be moved to a detached
				 * windchest, we must apply the gain of the existing windchest
				 * to the gain target for this fader - otherwise the playback
				 * volume on the detached chest will not match the volume on
				 * the existing chest. */
				gain_target *= vol;
				if (m_ScaledReleases)
				{
					/* Note: "time" is in milliseconds. */
					int time = ((m_CurrentTime - handle->time) * (10 * BLOCKS_PER_FRAME)) / (m_SampleRate / 100);
					/* TODO: below code should be replaced by a more accurate model of the attack to get a better estimate of the amplitude when playing very short notes */
					/* estimating attack duration from pipe midi pitch */
					unsigned midikey_frequency = this_pipe->GetMidiKeyNumber();
					/* if MidiKeyNumber is not within an organ 64 feet to 1 foot pipes, we assume average pipe (MIDI = 60)*/
					if (midikey_frequency >133 || midikey_frequency == 0 )
						midikey_frequency = 60;
					/* attack duration is assumed 50 ms above MIDI 96, 800 ms below MIDI 24 and linear in between */	
					float attack_duration = 50.0f; 
					if (midikey_frequency < 96 )
					{
						if (midikey_frequency < 24)
							attack_duration = 800.0f;
						else
							attack_duration = 800.0f + ( ( 24.0f - (float)midikey_frequency ) * 10.4f );
					}
					/* calculate gain (gain_target) to apply to tail amplitude in function of when the note is released during the attack */ 
					if (time < (int)attack_duration)
					{
						float attack_index = (float)time / attack_duration;
						gain_target *= ( 0.1f + ( 0.9f * ( 2.0f * attack_index - (attack_index * attack_index) )));
					}
					/* calculate the volume decay to be applied to the release to take into account the fact reverb is not completely formed during staccato */
					/* TODO time_to_full_reverb is a constant that should be replaced by an estimation of the time to full reverb which could be approximated by dividing release length (in ms) by fixed number e.g. 5 */
					int time_to_full_reverb = 600; 
					if (time < time_to_full_reverb)
					{
						gain_decay_rate = -14  + (int)( (-3.0f * (float)time / (float)time_to_full_reverb ) - 0.5f ); 
					}
				}
			}

			FaderNewAttacking
				(&new_sampler->fader
				,gain_target
				,-(CROSSFADE_LEN_BITS)
				,1 << (CROSSFADE_LEN_BITS + 1)
				);
			if (gain_decay_rate < 0)
				FaderStartDecay(&new_sampler->fader, gain_decay_rate);

			if (m_ReleaseAlignmentEnabled && release_section->SupportsStreamAlignment())
			{
				this_pipe->GetRelease()->InitAlignedStream
					(&new_sampler->stream
					,&handle->stream
					);
			}
			else
			{
				this_pipe->GetRelease()->InitStream
					(&m_ResamplerCoefs
					,&new_sampler->stream
					,this_pipe->GetTuning() / (float)m_SampleRate
					);
			}

			int windchest_index;
			if (not_a_tremulant)
			{
				/* detached releases are enabled and the pipe was on a regular
				 * windchest. Play the release on the detached windchest */
				int detached_windchest_index = 0;
				for(unsigned i = 1; i < m_DetachedRelease.size(); i++)
					if (m_DetachedRelease[i].count < m_DetachedRelease[detached_windchest_index].count)
						detached_windchest_index = i;
				if (detached_windchest_index)
					detached_windchest_index += m_Windchests.size();
				windchest_index = detached_windchest_index;
			}
			else
			{
				/* detached releases are disabled (or this isn't really a pipe)
				 * so put the release on the same windchest as the pipe (which
				 * means it will still be affected by tremulants - yuck). */
				windchest_index = handle->sampler_group_id;
			}
			StartSampler(new_sampler, windchest_index);
		}

	}
}


void GOSoundEngine::StopSample(const GOSoundProvider *pipe, SAMPLER_HANDLE handle)
{

	assert(handle);
	assert(pipe);

	// The following condition could arise if a one-shot sample is played,
	// decays away (and hence the sampler is discarded back into the pool), and
	// then the user releases a key. If the sampler had already been reused
	// with another pipe, that sample would erroneously be told to decay.
	if (pipe != handle->pipe)
		return;

	handle->stop = true;
}
