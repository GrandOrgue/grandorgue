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

#include "GOSoundEngine.h"

#include "GOSoundProvider.h"
#include "GOSoundReverb.h"
#include "GOSoundSampler.h"
#include "GOSoundGroupWorkItem.h"
#include "GOSoundTremulantWorkItem.h"
#include "GOSoundWindchestWorkItem.h"
#include "GOrgueEvent.h"
#include "GOrgueInt24.h"
#include "GOrguePipe.h"
#include "GOrgueReleaseAlignTable.h"
#include "GOrgueWindchest.h"
#include "GrandOrgueFile.h"

GOSoundEngine::GOSoundEngine() :
	m_PolyphonyLimiting(true),
	m_ScaledReleases(true),
	m_ReleaseAlignmentEnabled(true),
	m_RandomizeSpeaking(true),
	m_Volume(-15),
	m_ReleaseLength(0),
	m_Gain(1),
	m_SampleRate(0),
	m_CurrentTime(1),
	m_SamplerPool(),
	m_AudioGroupCount(1),
	m_WindchestCount(0),
	m_DetachedReleaseCount(1),
	m_TremulantCount(0),
	m_Tremulants(),
	m_Windchests(),
	m_AudioGroups(),
	m_AudioOutputs(),
	m_WorkItems(),
	m_NextItem(0)
{
	memset(&m_ResamplerCoefs, 0, sizeof(m_ResamplerCoefs));
	m_SamplerPool.SetUsageLimit(2048);
	m_PolyphonySoftLimit = (m_SamplerPool.GetUsageLimit() * 3) / 4;
	m_ReverbEngine.push_back(new GOSoundReverb(2));
	Reset();
}

GOSoundEngine::~GOSoundEngine()
{
}

void GOSoundEngine::Reset()
{
	while(m_AudioGroups.size() < m_AudioGroupCount)
		m_AudioGroups.push_back(new GOSoundGroupWorkItem(*this, 1));

	for (unsigned i = 0; i < m_Tremulants.size(); i++)
		m_Tremulants[i]->Clear();
	for (unsigned i = 0; i < m_Windchests.size(); i++)
		m_Windchests[i]->Init(m_Tremulants);
	for (unsigned i = 0; i < m_AudioGroups.size(); i++)
		m_AudioGroups[i]->Clear();
	for (unsigned i = 0; i < m_ReverbEngine.size(); i++)
		m_ReverbEngine[i]->Reset();
	m_WorkItems.resize(GetGroupCount());

	m_SamplerPool.ReturnAll();
	m_CurrentTime = 1;
	ResetDoneFlags();
}

void GOSoundEngine::SetVolume(int volume)
{
	if (volume > 20)
		volume = -15;
	m_Volume = volume;
	m_Gain = powf(10.0f, m_Volume * 0.05f);
}

float GOSoundEngine::GetGain()
{
	return m_Gain;
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

void GOSoundEngine::SetReleaseLength(unsigned reverb)
{
	m_ReleaseLength = reverb;
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

unsigned GOSoundEngine::GetFaderLength(unsigned MidiKeyNumber)
{
	unsigned fade_length = 46;
	if (MidiKeyNumber > 0 && MidiKeyNumber < 133 )
	{
		fade_length = 184 - (int)((((float)MidiKeyNumber - 42.0f) / 44.0f) * 178.0f);
		if (MidiKeyNumber < 42 )
			fade_length = 184;
		if (MidiKeyNumber > 86 )
			fade_length = 6;
	}
	return fade_length;
}

void GOSoundEngine::StartSampler(GO_SAMPLER* sampler, int sampler_group_id, unsigned audio_group)
{
	if (audio_group >= m_AudioGroupCount)
		audio_group = 0;

	sampler->sampler_group_id = sampler_group_id;
	sampler->audio_group_id = audio_group;
	sampler->stop = 0;
	sampler->new_attack = 0;

	if (sampler_group_id == 0)
	{
		sampler->windchest = m_Windchests[sampler_group_id];
		m_AudioGroups[audio_group]->Add(sampler);
	}
	else if (sampler_group_id < 0)
	{
		sampler->windchest = NULL;
		m_Tremulants[-1-sampler_group_id]->Add(sampler);
	}
	else
	{
		sampler->windchest = m_Windchests[sampler_group_id];
		m_AudioGroups[audio_group]->Add(sampler);
	}
}

void GOSoundEngine::ClearSetup()
{
	m_WindchestCount = 0;
	m_Windchests.clear();
	m_TremulantCount = 0;
	m_Tremulants.clear();
	Reset();
}

void GOSoundEngine::Setup(GrandOrgueFile* organ_file, unsigned samples_per_buffer, unsigned release_count)
{
	if (release_count < 1)
		release_count = 1;
	m_DetachedReleaseCount = release_count;
	m_WindchestCount = organ_file->GetWindchestGroupCount();
	m_TremulantCount = organ_file->GetTremulantCount();
	m_Tremulants.clear();
	for(unsigned i = 0; i < organ_file->GetTremulantCount(); i++)
		m_Tremulants.push_back(new GOSoundTremulantWorkItem(*this, samples_per_buffer));
	m_Windchests.clear();
	m_Windchests.push_back(new GOSoundWindchestWorkItem(*this, NULL));
	for(unsigned i = 0; i < organ_file->GetWindchestGroupCount(); i++)
		m_Windchests.push_back(new GOSoundWindchestWorkItem(*this, organ_file->GetWindchest(i)));
	m_AudioGroups.clear();
	for(unsigned i = 0; i < m_AudioGroupCount; i++)
		m_AudioGroups.push_back(new GOSoundGroupWorkItem(*this, samples_per_buffer));
	Reset();
}

bool GOSoundEngine::ProcessSampler(float output_buffer[GO_SOUND_BUFFER_SIZE], GO_SAMPLER* sampler, unsigned n_frames, float volume)
{
	const unsigned block_time = n_frames;
	float temp[GO_SOUND_BUFFER_SIZE];
	bool changed_sampler = false;
	const bool process_sampler = (sampler->time <= m_CurrentTime);

	if (process_sampler)
	{

		if  (
		     (m_PolyphonyLimiting) &&
		     (sampler->is_release) &&
		     (m_SamplerPool.UsedSamplerCount() >= m_PolyphonySoftLimit) &&
		     (m_CurrentTime - sampler->time > 172 * 16)
		     )
			sampler->fader.StartDecay(370, m_SampleRate); /* Approx 0.37s at 44.1kHz */

		if (sampler->stop && sampler->stop <= m_CurrentTime && sampler->stop - sampler->time <= block_time)
			sampler->pipe = NULL;

		/* The decoded sampler frame will contain values containing
		 * sampler->pipe_section->sample_bits worth of significant bits.
		 * It is the responsibility of the fade engine to bring these bits
		 * back into a sensible state. This is achieved during setup of the
		 * fade parameters. The gain target should be:
		 *
		 *     playback gain * (2 ^ -sampler->pipe_section->sample_bits)
		 */
		if (!GOAudioSection::ReadBlock(&sampler->stream, temp, n_frames))
			sampler->pipe = NULL;

		sampler->fader.Process(n_frames, temp, volume);
		
		/* Add these samples to the current output buffer shifting
		 * right by the necessary amount to bring the sample gain back
		 * to unity (this value is computed in GOrguePipe.cpp)
		 */
		for(unsigned i = 0; i < n_frames * 2; i++)
			output_buffer[i] += temp[i];

		if (sampler->stop && sampler->stop <= m_CurrentTime)
		{
			CreateReleaseSampler(sampler);
			sampler->stop = 0;
		} 
		else if (sampler->new_attack && sampler->new_attack <= m_CurrentTime)
		{
			SwitchAttackSampler(sampler);
			sampler->new_attack = 0;
			changed_sampler = true;
		}
	}

	if (!sampler->pipe || (sampler->fader.IsSilent() && process_sampler && !changed_sampler))
		return false;
	else
		return true;
}

void GOSoundEngine::ReturnSampler(GO_SAMPLER* sampler)
{
	m_SamplerPool.ReturnSampler(sampler);
}

unsigned GOSoundEngine::GetGroupCount()
{
	return m_AudioGroupCount * m_DetachedReleaseCount + m_WindchestCount + m_TremulantCount;
}

GOSoundWorkItem* GOSoundEngine::GetNextGroup()
{
	unsigned next = m_NextItem.fetch_add(1);
	if (next >= m_WorkItems.size())
		return NULL;
	return m_WorkItems[next];
}

void GOSoundEngine::ResetDoneFlags()
{
	std::vector<unsigned> counts;
	std::vector<GOSoundWorkItem*> ids;
	counts.reserve(m_AudioGroups.size());
	ids.reserve(m_AudioGroups.size());

	for(unsigned i = 0; i < m_AudioGroups.size(); i++)
	{
		counts.push_back(0);
		ids.push_back(NULL);
		unsigned pos = counts.size();
		unsigned cnt = m_AudioGroups[i]->GetCost();
		while(pos > 1 && counts[pos - 2] < cnt)
		{
			counts[pos - 1] = counts[pos - 2];
			ids[pos - 1] = ids[pos - 2];
			pos--;
		}
		counts[pos - 1] = cnt;
		ids[pos - 1] = m_AudioGroups[i];
	}
	unsigned pos = 0;
	for(unsigned i = 0; i < m_Tremulants.size(); i++)
		m_WorkItems[pos++] = m_Tremulants[i];
	for(unsigned i = 0; i < m_Windchests.size(); i++)
		m_WorkItems[pos++] = m_Windchests[i];
	for(unsigned j = 0; j < m_DetachedReleaseCount; j++)
		for(unsigned i = 0; i < ids.size(); i++)
			m_WorkItems[pos++] = ids[i];

	for (unsigned j = 0; j < m_Tremulants.size(); j++)
		m_Tremulants[j]->Reset();
	for (unsigned j = 0; j < m_Windchests.size(); j++)
		m_Windchests[j]->Reset();
	for (unsigned j = 0; j < m_AudioGroups.size(); j++)
		m_AudioGroups[j]->Reset();

	m_NextItem.exchange(0);
}

void GOSoundEngine::ProcessTremulants()
{
	for (unsigned j = 0; j < m_Tremulants.size(); j++)
		m_Tremulants[j]->Run();
}

void GOSoundEngine::SetAudioOutput(std::vector<GOAudioOutputConfiguration> audio_outputs)
{
	m_ReverbEngine.clear();
	m_ReverbEngine.push_back(new GOSoundReverb(2));
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
		m_ReverbEngine.push_back(new GOSoundReverb(m_AudioOutputs[i].channels));
	}
}

void GOSoundEngine::SetupReverb(GOrgueSettings& settings)
{
	for(unsigned i = 0; i < m_ReverbEngine.size(); i++)
		m_ReverbEngine[i]->Setup(settings);
}

void GOSoundEngine::GetAudioOutput(float *output_buffer, unsigned n_frames, unsigned audio_output)
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

			float* this_buff = m_AudioGroups[j / 2]->m_Buffer;
			m_AudioGroups[j / 2]->Finish();

			for (unsigned k = i, l = j % 2; k < n_frames * output.channels; k += output.channels, l+= 2)
				output_buffer[k] += factor * this_buff[l];
		}
	}

	m_ReverbEngine[audio_output + 1]->Process(output_buffer, n_frames);

	/* Clamp the output */
	static const float CLAMP_MIN = -1.0f;
	static const float CLAMP_MAX = 1.0f;
	for (unsigned k = 0; k < n_frames * output.channels; k++)
	{
		float f = std::min(std::max(output_buffer[k], CLAMP_MIN), CLAMP_MAX);
		output_buffer[k] = f;
	}
}


// this callback will fill the buffer with bufferSize frame
// audio is opened with 32 bit stereo, so one frame is 64bit (32bit for right 32bit for left)
// So buffer can handle 8*bufferSize char (or 2*bufferSize float)
void GOSoundEngine::GetSamples (float *output_buffer, unsigned n_frames, METER_INFO *meter_info)
{
	/* initialise the output buffer */
	float FinalBuffer[GO_SOUND_BUFFER_SIZE];
	std::fill(FinalBuffer, FinalBuffer + n_frames * 2, 0.0f);

	ProcessTremulants();

	for (unsigned j = 0; j < m_AudioGroups.size(); j++)
	{
		float* this_buff = m_AudioGroups[j]->m_Buffer;
		m_AudioGroups[j]->Finish();

		for (unsigned int k = 0; k < n_frames * 2; k++)
			FinalBuffer[k] += this_buff[k];
	}

	m_ReverbEngine[0]->Process(FinalBuffer, n_frames);

	m_CurrentTime += n_frames;

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
}


SAMPLER_HANDLE GOSoundEngine::StartSample(const GOSoundProvider* pipe, int sampler_group_id, unsigned audio_group, unsigned velocity, unsigned delay)
{
	const GOAudioSection* attack = pipe->GetAttack(velocity);
	if (!attack || attack->GetChannels() == 0)
		return NULL;
	GO_SAMPLER* sampler = m_SamplerPool.GetSampler();
	if (sampler)
	{
		sampler->pipe = pipe;
		sampler->velocity = velocity;
		attack->InitStream
			(&m_ResamplerCoefs
			,&sampler->stream
			,GetRandomFactor() * pipe->GetTuning() / (float)m_SampleRate
			);
		const float playback_gain = pipe->GetGain() * attack->GetNormGain();
		sampler->fader.NewConstant(playback_gain);
		sampler->delay = (delay * m_SampleRate) / (1000);
		sampler->time = m_CurrentTime + sampler->delay;
		sampler->fader.SetVelocityVolume(sampler->pipe->GetVelocityVolume(sampler->velocity));
		StartSampler(sampler, sampler_group_id, audio_group);
	}
	return sampler;
}

void GOSoundEngine::SwitchAttackSampler(GO_SAMPLER* handle)
{
	if (!handle->pipe)
		return;

	const GOSoundProvider* this_pipe = handle->pipe;
	const GOAudioSection* section = this_pipe->GetAttack(handle->velocity);
	if (!section)
		return;
	if (handle->is_release)
		return;

	GO_SAMPLER* new_sampler = m_SamplerPool.GetSampler();
	if (new_sampler != NULL)
	{
		*new_sampler = *handle;
		
		handle->pipe = this_pipe;
		handle->time = m_CurrentTime + 1;

		float gain_target = this_pipe->GetGain() * section->GetNormGain();
		unsigned cross_fade_len = GetFaderLength(this_pipe->GetMidiKeyNumber());
		handle->fader.NewAttacking(gain_target, cross_fade_len, m_SampleRate);

		section->InitAlignedStream(&handle->stream, &new_sampler->stream);
		handle->is_release = false;
		new_sampler->is_release = true;
		new_sampler->fader.StartDecay(cross_fade_len, m_SampleRate);
		new_sampler->fader.SetVelocityVolume(new_sampler->pipe->GetVelocityVolume(new_sampler->velocity));

		StartSampler(new_sampler, new_sampler->sampler_group_id, new_sampler->audio_group_id);
	}
}

void GOSoundEngine::CreateReleaseSampler(GO_SAMPLER* handle)
{
	if (!handle->pipe)
		return;

	const GOSoundProvider* this_pipe = handle->pipe;
	float vol = (handle->sampler_group_id < 0) ? 1.0f : m_Windchests[handle->sampler_group_id]->GetWindchestVolume();

	// FIXME: this is wrong... the intention is to not create a release for a
	// sample being played back with zero amplitude but this is a comparison
	// against a double. We should test against a minimum level.
	if (vol)
	{
		const GOAudioSection* release_section = this_pipe->GetRelease(&handle->stream, ((double)(m_CurrentTime - handle->time)) / m_SampleRate);
		if (!release_section)
			return;

		GO_SAMPLER* new_sampler = m_SamplerPool.GetSampler();
		if (new_sampler != NULL)
		{
			new_sampler->pipe = this_pipe;
			new_sampler->time = m_CurrentTime + 1;
			new_sampler->velocity = handle->velocity;

			unsigned gain_decay_length = 0;
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
					int time = ((m_CurrentTime - handle->time) * 1000) / m_SampleRate;
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
							attack_duration = 500.0f;
						else
							attack_duration = 500.0f + ( ( 24.0f - (float)midikey_frequency ) * 6.25f );
					}
					/* calculate gain (gain_target) to apply to tail amplitude in function of when the note is released during the attack */ 
					if (time < (int)attack_duration)
					{
						float attack_index = (float)time / attack_duration;
						float gain_delta = ( 0.2f + ( 0.8f * ( 2.0f * attack_index - (attack_index * attack_index) )));
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
						gain_decay_length = time_to_full_reverb + 6000 * time / time_to_full_reverb;
					}
				}
			}
			unsigned cross_fade_len = GetFaderLength(this_pipe->GetMidiKeyNumber());
			new_sampler->fader.NewAttacking(gain_target, cross_fade_len, m_SampleRate);

			if (m_ReleaseLength > 0)
			{
				if ( m_ReleaseLength < gain_decay_length || gain_decay_length == 0 )
					gain_decay_length = m_ReleaseLength;
			}

			if (gain_decay_length > 0)
				new_sampler->fader.StartDecay(gain_decay_length, m_SampleRate);

			if (m_ReleaseAlignmentEnabled && release_section->SupportsStreamAlignment())
			{
				release_section->InitAlignedStream(&new_sampler->stream, &handle->stream);
			}
			else
			{
				release_section->InitStream(&m_ResamplerCoefs, &new_sampler->stream, this_pipe->GetTuning() / (float)m_SampleRate);
			}
			new_sampler->is_release = true;

			int windchest_index;
			if (not_a_tremulant)
			{
				/* detached releases are enabled and the pipe was on a regular
				 * windchest. Play the release on the detached windchest */
				windchest_index = 0;
			}
			else
			{
				/* detached releases are disabled (or this isn't really a pipe)
				 * so put the release on the same windchest as the pipe (which
				 * means it will still be affected by tremulants - yuck). */
				windchest_index = handle->sampler_group_id;
			}
			new_sampler->fader.SetVelocityVolume(new_sampler->pipe->GetVelocityVolume(new_sampler->velocity));
			StartSampler(new_sampler, windchest_index, handle->audio_group_id);
		}

	}

	/* The above code created a new sampler to playback the release, the
	 * following code takes the active sampler for this pipe (which will be
	 * in either the attack or loop section) and sets the fadeout property
	 * which will decay this portion of the pipe. The sampler will
	 * automatically be placed back in the pool when the fade restores to
	 * zero. */
	unsigned cross_fade_len = GetFaderLength(handle->pipe->GetMidiKeyNumber());
	handle->fader.StartDecay(cross_fade_len, m_SampleRate);
	handle->is_release = true;
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

	handle->stop = m_CurrentTime + handle->delay;
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

	handle->new_attack = m_CurrentTime + handle->delay;
}

void GOSoundEngine::UpdateVelocity(SAMPLER_HANDLE handle, unsigned velocity)
{
	assert(handle);
	handle->velocity = velocity;
	/* Concurrent update possible, as it just update a float */
	handle->fader.SetVelocityVolume(handle->pipe->GetVelocityVolume(handle->velocity));
}
