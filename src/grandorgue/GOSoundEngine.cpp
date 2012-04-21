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
#include "GOLock.h"
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
 *   11   -4096 samples   (approx 92ms @ 44.1kHz)
 *   12   -9192 samples   (approx 184ms @ 44.1kHz)
 *   13  - 18384 samples  (approx 368ms @ 44.1kHz)
 *   14  - 36768 samples  (approx 736ms @ 44.1kHz)
 *   15  - 73536 samples  (approx 1472ms @ 44.1kHz)
 */
#define CROSSFADE_LEN_BITS (8)

GOSoundEngine::GOSoundEngine() :
	m_PolyphonyLimiting(true),
	m_ScaledReleases(true),
	m_ReleaseAlignmentEnabled(true),
	m_RandomizeSpeaking(true),
	m_Volume(-15),
	m_Reverb(0),
	m_Gain(1),
	m_SampleRate(0),
	m_CurrentTime(0),
	m_SamplerPool(),
	m_AudioGroupCount(1),
	m_WindchestCount(0),
	m_DetachedReleaseCount(1),
	m_DetachedRelease(1),
	m_Windchests(),
	m_Tremulants(),
	m_OutputGroups(1),
	m_AudioOutputs(),
	m_WorkItems(),
	m_NextItem(0)
{
	memset(&m_ResamplerCoefs, 0, sizeof(m_ResamplerCoefs));
	m_SamplerPool.SetUsageLimit(2048);
	m_PolyphonySoftLimit = (m_SamplerPool.GetUsageLimit() * 3) / 4;
	Reset();
}

void GOSoundEngine::Reset()
{
	m_DetachedRelease.resize(m_DetachedReleaseCount * m_AudioGroupCount);
	m_Windchests.resize(m_WindchestCount * m_AudioGroupCount);
	m_OutputGroups.resize(m_AudioGroupCount);
	for (unsigned i = 0; i < m_Windchests.size(); i++)
	{
		m_Windchests[i].new_sampler = 0;
		m_Windchests[i].end_new_sampler = 0;
		m_Windchests[i].sampler = 0;
		m_Windchests[i].count = 0;
	}
	for (unsigned i = 0; i < m_Tremulants.size(); i++)
	{
		m_Tremulants[i].new_sampler = 0;
		m_Tremulants[i].end_new_sampler = 0;
		m_Tremulants[i].sampler = 0;
		m_Tremulants[i].count = 0;
	}
	for (unsigned i = 0; i < m_DetachedRelease.size(); i++)
	{
		m_DetachedRelease[i].new_sampler = 0;
		m_DetachedRelease[i].end_new_sampler = 0;
		m_DetachedRelease[i].sampler = 0;
		m_DetachedRelease[i].count = 0;
	}
	m_SamplerPool.ReturnAll();
	m_CurrentTime = 0;
	ResetDoneFlags();
}

void GOSoundEngine::SetVolume(int volume)
{
	if (volume > 20)
		volume = -15;
	m_Volume = volume;
	m_Gain = powf(10.0f, m_Volume * 0.05f);
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

int GOSoundEngine::GetReverb() const
{
	return m_Reverb;
}

void GOSoundEngine::SetReverb(int reverb)
{
	m_Reverb = reverb;
}

void GOSoundEngine::SetPolyphonyLimiting(bool limiting)
{
	m_PolyphonyLimiting = limiting;
}

unsigned GOSoundEngine::GetHardPolyphony() const
{
	return m_SamplerPool.GetUsageLimit();
}

void GOSoundEngine::SetAudioGroupCount(unsigned groups)
{
	if (groups < 1)
		groups = 1;
	m_AudioGroupCount = groups;
}

unsigned GOSoundEngine::GetAudioGroupCount()
{
	return m_AudioGroupCount;
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
		const double factor = (pow(2, 1.0 / 1200.0) - 1) / (RAND_MAX / 2);
		int num = rand() - RAND_MAX / 2;
		return  1 + num * factor;
	}
	return 1;
}

void GOSoundEngine::StartSampler(GO_SAMPLER* sampler, int sampler_group_id, unsigned audio_group)
{
	GOSamplerEntry* state;

	if (audio_group >= m_AudioGroupCount)
		audio_group = 0;

	if (sampler_group_id == 0)
		state = &m_DetachedRelease[0 + audio_group * m_DetachedReleaseCount];
	else if (sampler_group_id < 0)
		state = &m_Tremulants[-1-sampler_group_id];
	else if (sampler_group_id > (int) m_Windchests.size())
		state = &m_DetachedRelease[sampler_group_id - m_Windchests.size() + audio_group * m_DetachedReleaseCount];
	else
		state = &m_Windchests[sampler_group_id - 1 + audio_group * m_WindchestCount];

	sampler->sampler_group_id = sampler_group_id;
	sampler->audio_group_id = audio_group;
	sampler->stop = false;
	sampler->new_attack = false;

	{
		GOMutexLocker locker(state->lock);
		if (!state->end_new_sampler)
			state->end_new_sampler = &sampler->next;
		sampler->next = state->new_sampler;
		state->new_sampler = sampler;
		state->count++;
	}
}

void GOSoundEngine::Setup(GrandOrgueFile* organ_file, unsigned release_count)
{
	if (release_count < 1)
		release_count = 1;
	m_DetachedReleaseCount = release_count;
	m_WindchestCount = organ_file->GetWindchestGroupCount();
	m_DetachedRelease.resize(m_DetachedReleaseCount * m_AudioGroupCount);
	m_Windchests.resize(m_WindchestCount * m_AudioGroupCount);
	m_OutputGroups.resize(m_AudioGroupCount);
	for (unsigned i = 0; i < m_WindchestCount; i++)
		for(unsigned j = 0; j < m_AudioGroupCount; j++)
			m_Windchests[m_WindchestCount * j + i].windchest = organ_file->GetWindchest(i);
	m_Tremulants.resize(organ_file->GetTremulantCount());
	for (unsigned i = 0; i < m_Tremulants.size(); i++)
		m_Tremulants[i].is_tremulant = true;
	m_WorkItems.resize(GetGroupCount());
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

void GOSoundEngine::ProcessAudioSamplers(GOSamplerEntry& state, unsigned int n_frames, bool depend)
{
	GOMutexLocker locker(state.mutex, !depend);

	if (!locker.IsLocked())
		return;

	if (state.done)
		return;
	{
		GOMutexLocker locker(state.lock);
		if (state.new_sampler)
		{
			*state.end_new_sampler = state.sampler;
			state.sampler = state.new_sampler;
			state.new_sampler = 0;
			state.end_new_sampler = 0;
		}
	}

	if (state.sampler == NULL)
	{
		state.done = 2;
		return;
	}

	assert((n_frames & (BLOCKS_PER_FRAME - 1)) == 0);
	assert(n_frames > BLOCKS_PER_FRAME);
	float* output_buffer = state.buff;
	std::fill(output_buffer, output_buffer + n_frames * 2, state.is_tremulant ? 1.0f : 0.0f);
	GO_SAMPLER* previous_sampler = NULL, *next_sampler = NULL;
	for (GO_SAMPLER* sampler = state.sampler; sampler; sampler = next_sampler)
	{
		float temp[GO_SOUND_BUFFER_SIZE];
		bool changed_sampler = false;

		const bool process_sampler = (sampler->time <= m_CurrentTime);
		if (process_sampler)
		{

			if  (
					(m_PolyphonyLimiting) &&
					(sampler->is_release) &&
					(m_SamplerPool.UsedSamplerCount() >= m_PolyphonySoftLimit) &&
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
				,temp
				);

			FaderProcess
				(&sampler->fader
				,n_frames
				,temp
				);

			/* Add these samples to the current output buffer shifting
			 * right by the necessary amount to bring the sample gain back
			 * to unity (this value is computed in GOrguePipe.cpp)
			 */
			for(unsigned i = 0; i < n_frames * 2; i++)
				output_buffer[i] += temp[i];

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
				sampler->is_release = true;
				sampler->stop = false;
			} 
			else if (sampler->new_attack)
			{
				SwitchAttackSampler(sampler);
				sampler->new_attack = false;
				changed_sampler = true;
			}
		}

		next_sampler = sampler->next;
		/* if this sampler's pipe has been set to null or the fade value is
		 * zero, the sample is no longer required and can be removed from the
		 * linked list. If it was still supplying audio, we must update the
		 * previous valid sampler. */
		if (!sampler->pipe || (FaderIsSilent(&sampler->fader) && process_sampler && !changed_sampler))
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

	if (!state.is_tremulant)
	{
		float f = m_Gain;
		if (state.windchest)
			f *= state.windchest->GetVolume();
		for (unsigned int i = 0; i < n_frames * 2; i++)
			output_buffer[i] *= f;

		if (state.windchest)
		{
			GOrgueWindchest* current_windchest = state.windchest;
			for (unsigned i = 0; i < current_windchest->GetTremulantCount(); i++)
			{
				unsigned tremulant_pos = current_windchest->GetTremulantId(i);
				if (!m_Tremulants[tremulant_pos].done)
					ProcessAudioSamplers(m_Tremulants[tremulant_pos], n_frames);

				if (m_Tremulants[tremulant_pos].done == 1)
				{
					const float *ptr = m_Tremulants[tremulant_pos].buff;
					for (unsigned int k = 0; k < n_frames * 2; k++)
						output_buffer[k] *= ptr[k];
				}
			}
		}
	}

	state.done = 1;
}

unsigned GOSoundEngine::GetGroupCount()
{
	return (m_DetachedReleaseCount + m_WindchestCount) * m_AudioGroupCount;
}

int GOSoundEngine::GetNextGroup()
{
	unsigned next = m_NextItem.fetch_add(1);
	if (next >= m_WorkItems.size())
		return -1;
	return m_WorkItems[next];
}

void GOSoundEngine::Process(unsigned group_id, unsigned n_frames)
{
	GOSamplerEntry* state;

	if (group_id >= GetGroupCount())
		return;

	if (group_id < m_DetachedReleaseCount * m_AudioGroupCount)
		state = &m_DetachedRelease[group_id];
	else
		state = &m_Windchests[group_id - m_DetachedReleaseCount * m_AudioGroupCount];

	if (!state->done)
		ProcessAudioSamplers(*state, n_frames, false);
}

void GOSoundEngine::ResetDoneFlags()
{
	std::vector<unsigned> counts;
	std::vector<unsigned> ids;
	counts.reserve(GetGroupCount());
	ids.reserve(GetGroupCount());

	for(unsigned i = 0; i < m_DetachedRelease.size(); i++)
	{
		counts.push_back(0);
		ids.push_back(0);
		unsigned pos = counts.size();
		while(pos > 1 && counts[pos - 2] < m_DetachedRelease[i].count)
		{
			counts[pos - 1] = counts[pos - 2];
			ids[pos - 1] = ids[pos - 2];
			pos--;
		}
		counts[pos - 1] = m_DetachedRelease[i].count;
		ids[pos - 1] = i;
	}
	for(unsigned i = 0; i < m_Windchests.size(); i++)
	{
		counts.push_back(0);
		ids.push_back(0);
		unsigned pos = counts.size();
		while(pos > 1 && counts[pos - 2] < m_Windchests[i].count)
		{
			counts[pos - 1] = counts[pos - 2];
			ids[pos - 1] = ids[pos - 2];
			pos--;
		}
		counts[pos - 1] = m_Windchests[i].count;
		ids[pos - 1] = i + m_DetachedRelease.size();
	}
	for(unsigned i = 0; i < m_WorkItems.size(); i++)
		m_WorkItems[i] = ids[i];

	for (unsigned j = 0; j < m_Tremulants.size(); j++)
	{
		GOMutexLocker locker(m_Tremulants[j].mutex);
		m_Tremulants[j].done = 0;
	}
	for (unsigned j = 0; j < m_Windchests.size(); j++)
	{
		GOMutexLocker locker(m_Windchests[j].mutex);
		m_Windchests[j].done = 0;
	}
	for (unsigned j = 0; j < m_DetachedRelease.size(); j++)
	{
		GOMutexLocker locker(m_DetachedRelease[j].mutex);
		m_DetachedRelease[j].done = 0;
	}
	for (unsigned j = 0; j < m_OutputGroups.size(); j++)
	{
		GOMutexLocker locker(m_OutputGroups[j].mutex);
		m_OutputGroups[j].done = false;
	}

	m_NextItem.exchange(0);
}

void GOSoundEngine::ProcessTremulants(unsigned n_frames)
{
	for (unsigned j = 0; j < m_Tremulants.size(); j++)
		if (!m_Tremulants[j].done)
			ProcessAudioSamplers(m_Tremulants[j], n_frames);
}

void GOSoundEngine::ProcessOutputGroup(unsigned audio_group, unsigned n_frames)
{
	GOOutputGroup& output = m_OutputGroups[audio_group];
	if (output.done)
		return;

	GOMutexLocker locker(output.mutex);

	if (output.done)
		return;

	std::fill(output.buff, output.buff + n_frames * 2, 0.0f);

	unsigned groupidx = m_WindchestCount * audio_group;
	for (unsigned j = 0; j < m_WindchestCount; j++)
	{
		float* this_buff = m_Windchests[j + groupidx].buff;

		if (!m_Windchests[j].done)
			ProcessAudioSamplers(m_Windchests[j + groupidx], n_frames);

		if (m_Windchests[j + groupidx].done != 1)
			continue;

		for (unsigned int k = 0; k < n_frames * 2; k++)
			output.buff[k] += this_buff[k];
	}

	groupidx = m_DetachedReleaseCount * audio_group;
	for (unsigned j = 0; j < m_DetachedReleaseCount; j++)
	{
		float* this_buff = m_DetachedRelease[j + groupidx].buff;

		if (!m_DetachedRelease[j + groupidx].done)
			ProcessAudioSamplers(m_DetachedRelease[j + groupidx], n_frames);

		if (m_DetachedRelease[j + groupidx].done != 1)
			continue;

		for (unsigned int k = 0; k < n_frames * 2; k++)
			output.buff[k] += this_buff[k];
	}
	output.done = true;
}

void GOSoundEngine::SetAudioOutput(std::vector<GOAudioOutputConfiguration> audio_outputs)
{
	m_AudioOutputs.resize(audio_outputs.size());
	for(unsigned i = 0; i < audio_outputs.size(); i++)
	{
		m_AudioOutputs[i].channels = audio_outputs[i].channels;
		m_AudioOutputs[i].scale_factors.resize(m_AudioGroupCount * audio_outputs[i].channels * 2);
		std::fill(m_AudioOutputs[i].scale_factors.begin(), m_AudioOutputs[i].scale_factors.end(), 0.0f);
		for(unsigned j = 0; j < audio_outputs[i].channels; j++)
			for(unsigned k = 0; k < audio_outputs[i].scale_factors[j].size(); k++)
			{
				if (k >= m_AudioGroupCount * 2)
					continue;
				float factor = audio_outputs[i].scale_factors[j][k];
				if (factor >= -120 && factor < 40)
					factor = powf(10.0f, factor * 0.05f);
				else
					factor = 0;
				m_AudioOutputs[i].scale_factors[j * m_AudioGroupCount * 2 + k] = factor;
			}
	}
}

int GOSoundEngine::GetAudioOutput(float *output_buffer, unsigned n_frames, unsigned audio_output)
{
	GOAudioOutput& output = m_AudioOutputs[audio_output];

	/* initialise the output buffer */
	std::fill(output_buffer, output_buffer + n_frames * output.channels, 0.0f);

	for(unsigned i = 0; i < output.channels; i++)
	{
		for(unsigned j = 0; j < m_AudioGroupCount * 2; j++)
		{
			float factor = output.scale_factors[i * m_AudioGroupCount * 2 + j];
			if (factor == 0)
				continue;

			float* this_buff = m_OutputGroups[j / 2].buff;
			ProcessOutputGroup(j / 2, n_frames);

			for (unsigned k = i, l = j % 2; k < n_frames * output.channels; k += output.channels, l+= 2)
				output_buffer[k] += factor * this_buff[l];
		}
	}

	/* Clamp the output */
	static const float CLAMP_MIN = -1.0f;
	static const float CLAMP_MAX = 1.0f;
	for (unsigned k = 0; k < n_frames * output.channels; k++)
	{
		float f = std::min(std::max(output_buffer[k], CLAMP_MIN), CLAMP_MAX);
		output_buffer[k] = f;
	}

	return 0;
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
	float FinalBuffer[GO_SOUND_BUFFER_SIZE];
	std::fill(FinalBuffer, FinalBuffer + n_frames * 2, 0.0f);

	ProcessTremulants(n_frames);

	for (unsigned j = 0; j < m_OutputGroups.size(); j++)
	{
		float* this_buff = m_OutputGroups[j].buff;

		ProcessOutputGroup(j, n_frames);

		for (unsigned int k = 0; k < n_frames * 2; k++)
			FinalBuffer[k] += this_buff[k];
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
			float f = std::min(std::max(FinalBuffer[k + 0], CLAMP_MIN), CLAMP_MAX);
			output_buffer[k + 0] = f;
			meter_info->meter_left  = (meter_info->meter_left > f)
			                        ? meter_info->meter_left
			                        : f;
			f = std::min(std::max(FinalBuffer[k + 1], CLAMP_MIN), CLAMP_MAX);
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
			float f = std::min(std::max(FinalBuffer[k + 0], CLAMP_MIN), CLAMP_MAX);
			output_buffer[k + 0] = f;
			f = std::min(std::max(FinalBuffer[k + 1], CLAMP_MIN), CLAMP_MAX);
			output_buffer[k + 1] = f;
		}
	}

	ResetDoneFlags();

	return 0;
}


SAMPLER_HANDLE GOSoundEngine::StartSample(const GOSoundProvider* pipe, int sampler_group_id, unsigned audio_group)
{
	const GOAudioSection* attack = pipe->GetAttack();
	if (!attack || attack->GetChannels() == 0)
		return NULL;
	GO_SAMPLER* sampler = m_SamplerPool.GetSampler();
	if (sampler)
	{
		sampler->pipe = pipe;
		attack->InitStream
			(&m_ResamplerCoefs
			,&sampler->stream
			,GetRandomFactor() * pipe->GetTuning() / (float)m_SampleRate
			);
		const float playback_gain = pipe->GetGain() * attack->GetNormGain();
		FaderNewConstant(&sampler->fader, playback_gain);
		sampler->time = m_CurrentTime;
		StartSampler(sampler, sampler_group_id, audio_group);
	}
	return sampler;
}

void GOSoundEngine::SwitchAttackSampler(GO_SAMPLER* handle)
{
	if (!handle->pipe)
		return;

	const GOSoundProvider* this_pipe = handle->pipe;
	const GOAudioSection* section = this_pipe->GetAttack();
	if (!section)
		return;
	if (handle->is_release)
		return;

	GO_SAMPLER* new_sampler = m_SamplerPool.GetSampler();
	if (new_sampler != NULL)
	{
		GO_SAMPLER* next = handle->next;
		*new_sampler = *handle;
		new_sampler->next = 0;
		
		handle->next = next;
		handle->pipe = this_pipe;
		handle->time = m_CurrentTime + 1;

		float gain_target = this_pipe->GetGain() * section->GetNormGain();

		FaderNewAttacking
			(&handle->fader
			 ,gain_target
			 ,-(CROSSFADE_LEN_BITS)
			 ,1 << (CROSSFADE_LEN_BITS + 1)
			 );

		section->InitAlignedStream(&handle->stream, &new_sampler->stream);
		handle->is_release = false;
		new_sampler->is_release = true;

		FaderStartDecay(&new_sampler->fader, -CROSSFADE_LEN_BITS);

		StartSampler(new_sampler, new_sampler->sampler_group_id, new_sampler->audio_group_id);
	}
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
		const GOAudioSection* release_section = this_pipe->GetRelease(&handle->stream, ((double)(m_CurrentTime - handle->time) * BLOCKS_PER_FRAME) / m_SampleRate);
		if (!release_section)
			return;

		GO_SAMPLER* new_sampler = m_SamplerPool.GetSampler();
		if (new_sampler != NULL)
		{

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
					/* TODO: below code should be replaced by a more accurate model of the attack to get a better estimate of the amplitude when playing very short notes
					* estimating attack duration from pipe midi pitch */
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
						float gain_delta = ( 0.05f + ( 0.95f * ( 2.0f * attack_index - (attack_index * attack_index) )));
						gain_target *= gain_delta;
					}
					/* calculate the volume decay to be applied to the release to take into account the fact reverb is not completely formed during staccato
					* time to full reverb is estimated in function of release length
					* for an organ with a release length of 5 seconds or more, time_to_full_reverb is around 350 ms 
					* for an organ with a release length of 1 second or less, time_to_full_reverb is around 100 ms 
					* time_to_full_reverb is linear in between */
					int time_to_full_reverb = ((60 * release_section->GetLength()) / release_section->GetSampleRate()) + 40;
					if (time_to_full_reverb > 350 ) time_to_full_reverb = 350;
					if (time_to_full_reverb < 100 ) time_to_full_reverb = 100;
					if (time < time_to_full_reverb)
					{
						/* in function of note duration, fading happens between: 
						* 200 ms and 6 s for release with little reverberation e.g. short release 
						* 700 ms and 6 s for release with large reverberation e.g. long release */ 
						int reverb_mini = time_to_full_reverb / 100;
						gain_decay_rate = -11  - reverb_mini - ( ( ( ( (6 - reverb_mini) * time * 2 ) / time_to_full_reverb ) + 1 ) / 2 ); 
					}
				}
			}

			FaderNewAttacking
				(&new_sampler->fader
				,gain_target
				,-(CROSSFADE_LEN_BITS)
				,1 << (CROSSFADE_LEN_BITS + 1)
				);

			int reverb = GetReverb();
			if ( reverb < 0 )
			{
				if ( reverb > gain_decay_rate || gain_decay_rate == 0 )
					gain_decay_rate = reverb;
			}

			if (gain_decay_rate < 0)
				FaderStartDecay(&new_sampler->fader, gain_decay_rate);

			if (m_ReleaseAlignmentEnabled && release_section->SupportsStreamAlignment())
			{
				release_section->InitAlignedStream
					(&new_sampler->stream
					,&handle->stream
					);
			}
			else
			{
				release_section->InitStream
					(&m_ResamplerCoefs
					,&new_sampler->stream
					,this_pipe->GetTuning() / (float)m_SampleRate
					);
			}
			new_sampler->is_release = true;

			int windchest_index;
			if (not_a_tremulant)
			{
				/* detached releases are enabled and the pipe was on a regular
				 * windchest. Play the release on the detached windchest */
				int detached_windchest_index = 0;
				unsigned groupidx = handle->audio_group_id * m_DetachedReleaseCount;
				for(unsigned i = 1; i < m_DetachedReleaseCount; i++)
					if (m_DetachedRelease[i + groupidx].count < m_DetachedRelease[detached_windchest_index + groupidx].count)
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
			StartSampler(new_sampler, windchest_index, handle->audio_group_id);
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

void GOSoundEngine::SwitchSample(const GOSoundProvider *pipe, SAMPLER_HANDLE handle)
{

	assert(handle);
	assert(pipe);

	// The following condition could arise if a one-shot sample is played,
	// decays away (and hence the sampler is discarded back into the pool), and
	// then the user releases a key. If the sampler had already been reused
	// with another pipe, that sample would erroneously be told to decay.
	if (pipe != handle->pipe)
		return;

	handle->new_attack = true;
}
