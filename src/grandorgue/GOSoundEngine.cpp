/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOSoundEngine.h"

#include "GOSoundProvider.h"
#include "GOSoundRecorder.h"
#include "GOSoundSampler.h"
#include "GOSoundGroupWorkItem.h"
#include "GOSoundOutputWorkItem.h"
#include "GOSoundTouchWorkItem.h"
#include "GOSoundTremulantWorkItem.h"
#include "GOSoundReleaseWorkItem.h"
#include "GOSoundWindchestWorkItem.h"
#include "GOrgueEvent.h"
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
	m_SamplesPerBuffer(1),
	m_Gain(1),
	m_SampleRate(0),
	m_CurrentTime(1),
	m_SamplerPool(),
	m_AudioGroupCount(1),
	m_UsedPolyphony(0),
	m_WorkerSlots(0),
	m_MeterInfo(1),
	m_Tremulants(),
	m_Windchests(),
	m_AudioGroups(),
	m_AudioOutputs(),
	m_AudioRecorder(NULL),
	m_TouchProcessor(),
	m_HasBeenSetup(false)
{
	memset(&m_ResamplerCoefs, 0, sizeof(m_ResamplerCoefs));
	m_SamplerPool.SetUsageLimit(2048);
	m_PolyphonySoftLimit = (m_SamplerPool.GetUsageLimit() * 3) / 4;
	m_ReleaseProcessor = new GOSoundReleaseWorkItem(*this, m_AudioGroups);
	Reset();
}

GOSoundEngine::~GOSoundEngine()
{
	if (m_ReleaseProcessor)
		delete m_ReleaseProcessor;
}

void GOSoundEngine::Reset()
{
  if (m_HasBeenSetup)
  {
	for (unsigned i = 0; i < m_Windchests.size(); i++)
		m_Windchests[i]->Init(m_Tremulants);
  }

  m_Scheduler.Clear();

  if (m_HasBeenSetup)
  {
	for (unsigned i = 0; i < m_Tremulants.size(); i++)
		m_Scheduler.Add(m_Tremulants[i]);
	for (unsigned i = 0; i < m_Windchests.size(); i++)
		m_Scheduler.Add(m_Windchests[i]);
	for (unsigned i = 0; i < m_AudioGroups.size(); i++)
		m_Scheduler.Add(m_AudioGroups[i]);
	for (unsigned i = 0; i < m_AudioOutputs.size(); i++)
		m_Scheduler.Add(m_AudioOutputs[i]);
	m_Scheduler.Add(m_AudioRecorder);
	m_Scheduler.Add(m_ReleaseProcessor);
	if (m_TouchProcessor)
		m_Scheduler.Add(m_TouchProcessor.get());
  }
	m_UsedPolyphony = 0;

	m_SamplerPool.ReturnAll();
	m_CurrentTime = 1;
	m_Scheduler.Reset();
}

void GOSoundEngine::SetVolume(int volume)
{
	m_Volume = volume;
	m_Gain = powf(10.0f, m_Volume * 0.05f);
}

float GOSoundEngine::GetGain()
{
	return m_Gain;
}

void GOSoundEngine::SetSamplesPerBuffer(unsigned samples_per_buffer)
{
	m_SamplesPerBuffer = samples_per_buffer;
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
	m_AudioGroups.clear();
	for(unsigned i = 0; i < m_AudioGroupCount; i++)
		m_AudioGroups.push_back(new GOSoundGroupWorkItem(*this, m_SamplesPerBuffer));
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

void GOSoundEngine::PassSampler(GO_SAMPLER* sampler)
{
	if (sampler->sampler_group_id == 0)
	{
		m_AudioGroups[sampler->audio_group_id]->Add(sampler);
	}
	else if (sampler->sampler_group_id < 0)
	{
		m_Tremulants[-1-sampler->sampler_group_id]->Add(sampler);
	}
	else
	{
		m_AudioGroups[sampler->audio_group_id]->Add(sampler);
	}
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
	m_HasBeenSetup = false;
	m_Scheduler.Clear();
	m_Windchests.clear();
	m_Tremulants.clear();
	m_TouchProcessor = NULL;
	Reset();
}

void GOSoundEngine::Setup(GrandOrgueFile* organ_file, unsigned release_count)
{
	m_Scheduler.Clear();
	if (release_count < 1)
		release_count = 1;
	m_Scheduler.SetRepeatCount(release_count);
	m_Tremulants.clear();
	for(unsigned i = 0; i < organ_file->GetTremulantCount(); i++)
		m_Tremulants.push_back(new GOSoundTremulantWorkItem(*this, m_SamplesPerBuffer));
	m_Windchests.clear();
	m_Windchests.push_back(new GOSoundWindchestWorkItem(*this, NULL));
	for(unsigned i = 0; i < organ_file->GetWindchestGroupCount(); i++)
		m_Windchests.push_back(new GOSoundWindchestWorkItem(*this, organ_file->GetWindchest(i)));
	m_TouchProcessor = std::unique_ptr<GOSoundTouchWorkItem>(new GOSoundTouchWorkItem(organ_file->GetMemoryPool()));
	m_HasBeenSetup = true;
	Reset();
}

bool GOSoundEngine::ProcessSampler(float *output_buffer, GO_SAMPLER* sampler, unsigned n_frames, float volume)
{
	const unsigned block_time = n_frames;
	float temp[n_frames * 2];
	const bool process_sampler = (sampler->time <= m_CurrentTime);

	if (process_sampler)
	{

		if  (sampler->is_release &&
		     ((m_PolyphonyLimiting && m_SamplerPool.UsedSamplerCount() >= m_PolyphonySoftLimit && m_CurrentTime - sampler->time > 172 * 16) ||
		      sampler->drop_counter > 1))
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

		if ((sampler->stop && sampler->stop <= m_CurrentTime) ||
		    (sampler->new_attack && sampler->new_attack <= m_CurrentTime))
		{
			m_ReleaseProcessor->Add(sampler);
			return false;
		}
	}

	if (!sampler->pipe || (sampler->fader.IsSilent() && process_sampler))
	{
		ReturnSampler(sampler);
		return false;
	}
	else
		return true;
}

void GOSoundEngine::ProcessRelease(GO_SAMPLER* sampler)
{
	if (sampler->stop)
	{
		CreateReleaseSampler(sampler);
		sampler->stop = 0;
	}
	else if (sampler->new_attack)
	{
		SwitchAttackSampler(sampler);
		sampler->new_attack = 0;
	}
	PassSampler(sampler);
}

void GOSoundEngine::ReturnSampler(GO_SAMPLER* sampler)
{
	m_SamplerPool.ReturnSampler(sampler);
}

GOSoundScheduler& GOSoundEngine::GetScheduler()
{
	return m_Scheduler;
}

void GOSoundEngine::SetAudioOutput(std::vector<GOAudioOutputConfiguration> audio_outputs)
{
	m_AudioOutputs.clear();
	{
		std::vector<float> scale_factors;
		scale_factors.resize(m_AudioGroupCount * 2 * 2);
		std::fill(scale_factors.begin(), scale_factors.end(), 0.0f);
		for(unsigned i = 0; i < m_AudioGroupCount; i++)
		{
			scale_factors[i * 4] = 1;
			scale_factors[i * 4 + 3] = 1;
		}
		m_AudioOutputs.push_back(new GOSoundOutputWorkItem(2, scale_factors, m_SamplesPerBuffer));
	}
	unsigned channels = 0;
	for(unsigned i = 0; i < audio_outputs.size(); i++)
	{
		std::vector<float> scale_factors;
		scale_factors.resize(m_AudioGroupCount * audio_outputs[i].channels * 2);
		std::fill(scale_factors.begin(), scale_factors.end(), 0.0f);
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
				scale_factors[j * m_AudioGroupCount * 2 + k] = factor;
			}
		m_AudioOutputs.push_back(new GOSoundOutputWorkItem(audio_outputs[i].channels, scale_factors, m_SamplesPerBuffer));
		channels += audio_outputs[i].channels;
	}
	std::vector<GOSoundBufferItem*> outputs;
	for (unsigned i = 0; i < m_AudioGroups.size(); i++)
		outputs.push_back(m_AudioGroups[i]);
	for (unsigned i = 0; i < m_AudioOutputs.size(); i++)
		m_AudioOutputs[i]->SetOutputs(outputs);
	m_MeterInfo.resize(channels + 1);
}

void GOSoundEngine::SetAudioRecorder(GOSoundRecorder* recorder, bool downmix)
{
	m_AudioRecorder = recorder;
	std::vector<GOSoundBufferItem*> outputs;
	if (downmix)
		outputs.push_back(m_AudioOutputs[0]);
	else
	{
		m_Scheduler.Remove(m_AudioOutputs[0]);
		delete m_AudioOutputs[0];
		m_AudioOutputs[0] = NULL;
		for (unsigned i = 1; i < m_AudioOutputs.size(); i++)
			outputs.push_back(m_AudioOutputs[i]);
	}
	m_AudioRecorder->SetOutputs(outputs, m_SamplesPerBuffer);
}

void GOSoundEngine::SetupReverb(GOrgueSettings& settings)
{
	for(unsigned i = 0; i < m_AudioOutputs.size(); i++)
		if (m_AudioOutputs[i])
			m_AudioOutputs[i]->SetupReverb(settings);
}

void GOSoundEngine::GetAudioOutput(float *output_buffer, unsigned n_frames, unsigned audio_output, bool last)
{
	m_AudioOutputs[audio_output + 1]->Finish(last);
	memcpy(output_buffer, m_AudioOutputs[audio_output + 1]->m_Buffer, sizeof(float) * n_frames * m_AudioOutputs[audio_output + 1]->GetChannels());
}

void GOSoundEngine::NextPeriod()
{
	m_Scheduler.Exec();

	m_CurrentTime += m_SamplesPerBuffer;
	unsigned used_samplers = m_SamplerPool.UsedSamplerCount();
	if (used_samplers > m_UsedPolyphony)
			m_UsedPolyphony = used_samplers;

	m_Scheduler.Reset();
}


GO_SAMPLER* GOSoundEngine::StartSample(const GOSoundProvider* pipe, int sampler_group_id, unsigned audio_group, unsigned velocity, unsigned delay, uint64_t last_stop)
{
	unsigned delay_samples = (delay * m_SampleRate) / (1000);
	uint64_t start_time = m_CurrentTime + delay_samples;
	uint64_t released_time = ((start_time - last_stop) * 1000) / m_SampleRate;
	if (released_time > (unsigned)-1)
		released_time = (unsigned)-1;

	const GOAudioSection* attack = pipe->GetAttack(velocity, released_time);
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
		sampler->delay = delay_samples;
		sampler->time = start_time;
		sampler->fader.SetVelocityVolume(sampler->pipe->GetVelocityVolume(sampler->velocity));
		StartSampler(sampler, sampler_group_id, audio_group);
	}
	return sampler;
}

void GOSoundEngine::SwitchAttackSampler(GO_SAMPLER* handle)
{
	if (!handle->pipe)
		return;

	unsigned time = 1000;

	const GOSoundProvider* this_pipe = handle->pipe;
	const GOAudioSection* section = this_pipe->GetAttack(handle->velocity, time);
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
		unsigned cross_fade_len = this_pipe->GetReleaseCrossfadeLength();
		handle->fader.NewAttacking(gain_target, cross_fade_len, m_SampleRate);

		section->InitAlignedStream(&handle->stream, &new_sampler->stream);
		handle->is_release = false;
		new_sampler->is_release = true;
		new_sampler->time = m_CurrentTime;
		new_sampler->fader.StartDecay(cross_fade_len, m_SampleRate);
		new_sampler->fader.SetVelocityVolume(new_sampler->pipe->GetVelocityVolume(new_sampler->velocity));

		StartSampler(new_sampler, new_sampler->sampler_group_id, new_sampler->audio_group_id);
	}
}

void GOSoundEngine::CreateReleaseSampler(GO_SAMPLER* handle)
{
	if (!handle->pipe)
		return;

	/* The beloow code creates a new sampler to playback the release, the
	 * following code takes the active sampler for this pipe (which will be
	 * in either the attack or loop section) and sets the fadeout property
	 * which will decay this portion of the pipe. The sampler will
	 * automatically be placed back in the pool when the fade restores to
	 * zero. */
	unsigned cross_fade_len = handle->pipe->GetReleaseCrossfadeLength();
	handle->fader.StartDecay(cross_fade_len, m_SampleRate);
	handle->is_release = true;

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

          				/* Release Sample Truncation Settings */

      				unsigned truncation_fade_len = this_pipe->GetReleaseTruncationLength();

					// Initialize Value For Maximum Release Length.
					int time_to_full_reverb = 0;

					/* If (Pipe999ReleaseTruncationLength) release truncation length values exist in organ settings or ODF...
					Maximum Release Length = Release Truncation Length */
					if (truncation_fade_len > 0)
					{
						time_to_full_reverb = truncation_fade_len;
					}

					/* Else if release truncation length values are absent from organ settings or ODF...
					* Maximum Release Length = /* Calculated Length of WAV Release Samples* */
          				else
          				{
              					time_to_full_reverb = ((60 * release_section->GetLength()) / release_section->GetSampleRate()) + 40;
          				}

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

			unsigned cross_fade_len = this_pipe->GetReleaseCrossfadeLength();
			unsigned truncation_fade_len = this_pipe->GetReleaseTruncationLength();
			new_sampler->fader.NewAttacking(gain_target, cross_fade_len, m_SampleRate);

			/*
			 * Algorithm Determines Which Release Truncation or Scaling Method to activate. */

			// If release length set in GO GUI toolbar is larger than 0...
      			if (m_ReleaseLength > 0)
			{
				// If Gain Decay Length is greater than toolbar value, or equal to 0, gain_decay_length = toolbar value.
				if (m_ReleaseLength < gain_decay_length || gain_decay_length == 0)
					gain_decay_length = m_ReleaseLength;
			}
			// If truncation values exist and release scaling has not been enabled, gain_decay_length = truncation value.
			else if (truncation_fade_len > 0 && gain_decay_length == 0)
			{
				gain_decay_length = truncation_fade_len;
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
			handle->time = m_CurrentTime;
		}

	}
}


uint64_t GOSoundEngine::StopSample(const GOSoundProvider *pipe, GO_SAMPLER* handle)
{

	assert(handle);
	assert(pipe);

	// The following condition could arise if a one-shot sample is played,
	// decays away (and hence the sampler is discarded back into the pool), and
	// then the user releases a key. If the sampler had already been reused
	// with another pipe, that sample would erroneously be told to decay.
	if (pipe != handle->pipe)
		return 0;

	handle->stop = m_CurrentTime + handle->delay;
	return handle->stop;
}

void GOSoundEngine::SwitchSample(const GOSoundProvider *pipe, GO_SAMPLER* handle)
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

void GOSoundEngine::UpdateVelocity(GO_SAMPLER* handle, unsigned velocity)
{
	assert(handle);
	handle->velocity = velocity;
	/* Concurrent update possible, as it just update a float */
	handle->fader.SetVelocityVolume(handle->pipe->GetVelocityVolume(handle->velocity));
}

const std::vector<double>& GOSoundEngine::GetMeterInfo()
{
	m_MeterInfo[0] = m_UsedPolyphony / (double)GetHardPolyphony();
	m_UsedPolyphony = 0;

	for(unsigned i = 1; i < m_MeterInfo.size(); i++)
		m_MeterInfo[i] = 0;
	for(unsigned i = 0, nr = 1; i < m_AudioOutputs.size(); i++)
	{
		if (!m_AudioOutputs[i])
			continue;
		const std::vector<float>& info = m_AudioOutputs[i]->GetMeterInfo();
		for(unsigned j = 0; j < info.size(); j++)
			m_MeterInfo[nr++] = info[j];
		m_AudioOutputs[i]->ResetMeterInfo();
	}
	return m_MeterInfo;
}