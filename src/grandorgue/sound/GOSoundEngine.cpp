/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundEngine.h"

#include <algorithm>

#include "model/GOPipe.h"
#include "model/GOWindchest.h"
#include "sound/scheduler/GOSoundGroupTask.h"
#include "sound/scheduler/GOSoundOutputTask.h"
#include "sound/scheduler/GOSoundReleaseTask.h"
#include "sound/scheduler/GOSoundTouchTask.h"
#include "sound/scheduler/GOSoundTremulantTask.h"
#include "sound/scheduler/GOSoundWindchestTask.h"

#include "GOEvent.h"
#include "GOOrganController.h"
#include "GOSoundProvider.h"
#include "GOSoundRecorder.h"
#include "GOSoundReleaseAlignTable.h"
#include "GOSoundSampler.h"

GOSoundEngine::GOSoundEngine()
  : m_PolyphonyLimiting(true),
    m_ScaledReleases(true),
    m_ReleaseAlignmentEnabled(true),
    m_RandomizeSpeaking(true),
    m_Volume(-15),
    m_SamplesPerBuffer(1),
    m_Gain(1),
    m_SampleRate(0),
    m_CurrentTime(1),
    m_SamplerPool(),
    m_AudioGroupCount(1),
    m_UsedPolyphony(0),
    m_WorkerSlots(0),
    m_MeterInfo(1),
    m_TremulantTasks(),
    m_WindchestTasks(),
    m_AudioGroupTasks(),
    m_AudioOutputTasks(),
    m_AudioRecorder(NULL),
    m_TouchTask(),
    m_HasBeenSetup(false) {
  m_SamplerPool.SetUsageLimit(2048);
  m_PolyphonySoftLimit = (m_SamplerPool.GetUsageLimit() * 3) / 4;
  m_ReleaseProcessor = new GOSoundReleaseTask(*this, m_AudioGroupTasks);
  Reset();
}

GOSoundEngine::~GOSoundEngine() {
  if (m_ReleaseProcessor)
    delete m_ReleaseProcessor;
}

void GOSoundEngine::Reset() {
  if (m_HasBeenSetup.load()) {
    for (unsigned i = 0; i < m_WindchestTasks.size(); i++)
      m_WindchestTasks[i]->Init(m_TremulantTasks);
  }

  m_Scheduler.Clear();

  if (m_HasBeenSetup.load()) {
    for (unsigned i = 0; i < m_TremulantTasks.size(); i++)
      m_Scheduler.Add(m_TremulantTasks[i]);
    for (unsigned i = 0; i < m_WindchestTasks.size(); i++)
      m_Scheduler.Add(m_WindchestTasks[i]);
    for (unsigned i = 0; i < m_AudioGroupTasks.size(); i++)
      m_Scheduler.Add(m_AudioGroupTasks[i]);
    for (unsigned i = 0; i < m_AudioOutputTasks.size(); i++)
      m_Scheduler.Add(m_AudioOutputTasks[i]);
    m_Scheduler.Add(m_AudioRecorder);
    m_Scheduler.Add(m_ReleaseProcessor);
    if (m_TouchTask)
      m_Scheduler.Add(m_TouchTask.get());
  }
  m_UsedPolyphony.store(0);

  m_SamplerPool.ReturnAll();
  m_CurrentTime = 1;
  m_Scheduler.Reset();
}

void GOSoundEngine::SetVolume(int volume) {
  m_Volume = volume;
  m_Gain = powf(10.0f, m_Volume * 0.05f);
}

float GOSoundEngine::GetGain() { return m_Gain; }

void GOSoundEngine::SetSamplesPerBuffer(unsigned samples_per_buffer) {
  m_SamplesPerBuffer = samples_per_buffer;
}

void GOSoundEngine::SetSampleRate(unsigned sample_rate) {
  m_SampleRate = sample_rate;
}

void GOSoundEngine::SetInterpolationType(unsigned type) {
  m_interpolation = (GOSoundResample::InterpolationType)type;
}

unsigned GOSoundEngine::GetSampleRate() { return m_SampleRate; }

void GOSoundEngine::SetHardPolyphony(unsigned polyphony) {
  m_SamplerPool.SetUsageLimit(polyphony);
  m_PolyphonySoftLimit = (m_SamplerPool.GetUsageLimit() * 3) / 4;
}

void GOSoundEngine::SetPolyphonyLimiting(bool limiting) {
  m_PolyphonyLimiting = limiting;
}

unsigned GOSoundEngine::GetHardPolyphony() const {
  return m_SamplerPool.GetUsageLimit();
}

void GOSoundEngine::SetAudioGroupCount(unsigned groups) {
  if (groups < 1)
    groups = 1;
  m_AudioGroupCount = groups;
  m_AudioGroupTasks.clear();
  for (unsigned i = 0; i < m_AudioGroupCount; i++)
    m_AudioGroupTasks.push_back(
      new GOSoundGroupTask(*this, m_SamplesPerBuffer));
}

unsigned GOSoundEngine::GetAudioGroupCount() { return m_AudioGroupCount; }

int GOSoundEngine::GetVolume() const { return m_Volume; }

void GOSoundEngine::SetScaledReleases(bool enable) {
  m_ScaledReleases = enable;
}

void GOSoundEngine::SetRandomizeSpeaking(bool enable) {
  m_RandomizeSpeaking = enable;
}

float GOSoundEngine::GetRandomFactor() {
  if (m_RandomizeSpeaking) {
    const double factor = (pow(2, 1.0 / 1200.0) - 1) / (RAND_MAX / 2);
    int num = rand() - RAND_MAX / 2;
    return 1 + num * factor;
  }
  return 1;
}

void GOSoundEngine::PassSampler(GOSoundSampler *sampler) {
  int taskId = sampler->m_SamplerTaskId;

  if (isWindchestTask(taskId))
    m_AudioGroupTasks[sampler->m_AudioGroupId]->Add(sampler);
  else
    m_TremulantTasks[tremulantTaskToIndex(taskId)]->Add(sampler);
}

void GOSoundEngine::StartSampler(GOSoundSampler *sampler) {
  int taskId = sampler->m_SamplerTaskId;

  sampler->stop = 0;
  sampler->new_attack = 0;
  sampler->p_WindchestTask = isWindchestTask(taskId)
    ? m_WindchestTasks[windchestTaskToIndex(taskId)]
    : nullptr;
  PassSampler(sampler);
}

void GOSoundEngine::ClearSetup() {
  m_HasBeenSetup.store(false);

  // the winchests may be still used from audio callbacks.
  // clear the pending sound before destroying the windchests
  for (unsigned i = 0; i < m_AudioGroupTasks.size(); i++)
    m_AudioGroupTasks[i]->WaitAndClear();

  m_Scheduler.Clear();
  m_WindchestTasks.clear();
  m_TremulantTasks.clear();
  m_TouchTask = NULL;
  Reset();
}

void GOSoundEngine::Setup(
  GOOrganController *organController, unsigned release_count) {
  m_Scheduler.Clear();
  if (release_count < 1)
    release_count = 1;
  m_Scheduler.SetRepeatCount(release_count);
  m_TremulantTasks.clear();
  for (unsigned i = 0; i < organController->GetTremulantCount(); i++)
    m_TremulantTasks.push_back(
      new GOSoundTremulantTask(*this, m_SamplesPerBuffer));
  m_WindchestTasks.clear();
  // a special windchest task for detached releases
  m_WindchestTasks.push_back(new GOSoundWindchestTask(*this, NULL));
  for (unsigned i = 0; i < organController->GetWindchestCount(); i++)
    m_WindchestTasks.push_back(
      new GOSoundWindchestTask(*this, organController->GetWindchest(i)));
  m_TouchTask = std::unique_ptr<GOSoundTouchTask>(
    new GOSoundTouchTask(organController->GetMemoryPool()));
  m_HasBeenSetup.store(true);
  Reset();
}

bool GOSoundEngine::ProcessSampler(
  float *output_buffer,
  GOSoundSampler *sampler,
  unsigned n_frames,
  float volume) {
  float temp[n_frames * 2];
  const bool process_sampler = (sampler->time <= m_CurrentTime);

  if (process_sampler) {
    if (sampler->is_release &&
        ((m_PolyphonyLimiting &&
          m_SamplerPool.UsedSamplerCount() >= m_PolyphonySoftLimit &&
          m_CurrentTime - sampler->time > 172 * 16) ||
         sampler->drop_counter > 1))
      sampler->fader.StartDecreasingVolume(MsToSamples(370));

    /* The decoded sampler frame will contain values containing
     * sampler->pipe_section->sample_bits worth of significant bits.
     * It is the responsibility of the fade engine to bring these bits
     * back into a sensible state. This is achieved during setup of the
     * fade parameters. The gain target should be:
     *
     *     playback gain * (2 ^ -sampler->pipe_section->sample_bits)
     */
    if (!sampler->stream.ReadBlock(temp, n_frames))
      sampler->p_SoundProvider = NULL;

    sampler->fader.Process(n_frames, temp, volume);
    if (sampler->toneBalanceFilterState.IsToApply())
      sampler->toneBalanceFilterState.ProcessBuffer(n_frames, temp);

    /* Add these samples to the current output buffer shifting
     * right by the necessary amount to bring the sample gain back
     * to unity (this value is computed in GOPipe.cpp)
     */
    for (unsigned i = 0; i < n_frames * 2; i++)
      output_buffer[i] += temp[i];

    if (
      (sampler->stop && sampler->stop <= m_CurrentTime)
      || (sampler->new_attack && sampler->new_attack <= m_CurrentTime)) {
      m_ReleaseProcessor->Add(sampler);
      return false;
    }
  }

  if (
    !sampler->p_SoundProvider
    || (sampler->fader.IsSilent() && process_sampler)) {
    ReturnSampler(sampler);
    return false;
  } else
    return true;
}

void GOSoundEngine::ProcessRelease(GOSoundSampler *sampler) {
  if (sampler->stop) {
    CreateReleaseSampler(sampler);
    sampler->stop = 0;
  } else if (sampler->new_attack) {
    SwitchToAnotherAttack(sampler);
    sampler->new_attack = 0;
  }
  PassSampler(sampler);
}

void GOSoundEngine::ReturnSampler(GOSoundSampler *sampler) {
  m_SamplerPool.ReturnSampler(sampler);
}

GOSoundScheduler &GOSoundEngine::GetScheduler() { return m_Scheduler; }

void GOSoundEngine::SetAudioOutput(
  std::vector<GOAudioOutputConfiguration> audio_outputs) {
  m_AudioOutputTasks.clear();
  {
    std::vector<float> scale_factors;
    scale_factors.resize(m_AudioGroupCount * 2 * 2);
    std::fill(scale_factors.begin(), scale_factors.end(), 0.0f);
    for (unsigned i = 0; i < m_AudioGroupCount; i++) {
      scale_factors[i * 4] = 1;
      scale_factors[i * 4 + 3] = 1;
    }
    m_AudioOutputTasks.push_back(
      new GOSoundOutputTask(2, scale_factors, m_SamplesPerBuffer));
  }
  unsigned channels = 0;
  for (unsigned i = 0; i < audio_outputs.size(); i++) {
    std::vector<float> scale_factors;
    scale_factors.resize(m_AudioGroupCount * audio_outputs[i].channels * 2);
    std::fill(scale_factors.begin(), scale_factors.end(), 0.0f);
    for (unsigned j = 0; j < audio_outputs[i].channels; j++)
      for (unsigned k = 0; k < audio_outputs[i].scale_factors[j].size(); k++) {
        if (k >= m_AudioGroupCount * 2)
          continue;
        float factor = audio_outputs[i].scale_factors[j][k];
        if (factor >= -120 && factor < 40)
          factor = powf(10.0f, factor * 0.05f);
        else
          factor = 0;
        scale_factors[j * m_AudioGroupCount * 2 + k] = factor;
      }
    m_AudioOutputTasks.push_back(new GOSoundOutputTask(
      audio_outputs[i].channels, scale_factors, m_SamplesPerBuffer));
    channels += audio_outputs[i].channels;
  }
  std::vector<GOSoundBufferItem *> outputs;
  for (unsigned i = 0; i < m_AudioGroupTasks.size(); i++)
    outputs.push_back(m_AudioGroupTasks[i]);
  for (unsigned i = 0; i < m_AudioOutputTasks.size(); i++)
    m_AudioOutputTasks[i]->SetOutputs(outputs);
  m_MeterInfo.resize(channels + 1);
}

void GOSoundEngine::SetAudioRecorder(GOSoundRecorder *recorder, bool downmix) {
  m_AudioRecorder = recorder;
  std::vector<GOSoundBufferItem *> outputs;
  if (downmix)
    outputs.push_back(m_AudioOutputTasks[0]);
  else {
    m_Scheduler.Remove(m_AudioOutputTasks[0]);
    delete m_AudioOutputTasks[0];
    m_AudioOutputTasks[0] = NULL;
    for (unsigned i = 1; i < m_AudioOutputTasks.size(); i++)
      outputs.push_back(m_AudioOutputTasks[i]);
  }
  m_AudioRecorder->SetOutputs(outputs, m_SamplesPerBuffer);
}

void GOSoundEngine::SetupReverb(GOConfig &settings) {
  for (unsigned i = 0; i < m_AudioOutputTasks.size(); i++)
    if (m_AudioOutputTasks[i])
      m_AudioOutputTasks[i]->SetupReverb(settings);
}

unsigned GOSoundEngine::GetBufferSizeFor(
  unsigned outputIndex, unsigned nFrames) {
  return sizeof(float) * nFrames
    * m_AudioOutputTasks[outputIndex + 1]->GetChannels();
}

void GOSoundEngine::GetEmptyAudioOutput(
  unsigned outputIndex, unsigned nFrames, float *pOutputBuffer) {
  memset(pOutputBuffer, 0, GetBufferSizeFor(outputIndex, nFrames));
}

void GOSoundEngine::GetAudioOutput(
  float *output_buffer, unsigned n_frames, unsigned audio_output, bool last) {
  if (m_HasBeenSetup.load()) {
    m_AudioOutputTasks[audio_output + 1]->Finish(last);
    memcpy(
      output_buffer,
      m_AudioOutputTasks[audio_output + 1]->m_Buffer,
      GetBufferSizeFor(audio_output, n_frames));
  } else
    GetEmptyAudioOutput(audio_output, n_frames, output_buffer);
}

void GOSoundEngine::NextPeriod() {
  m_Scheduler.Exec();

  m_CurrentTime += m_SamplesPerBuffer;
  unsigned used_samplers = m_SamplerPool.UsedSamplerCount();
  if (used_samplers > m_UsedPolyphony.load())
    m_UsedPolyphony.store(used_samplers);

  m_Scheduler.Reset();
}

unsigned GOSoundEngine::SamplesDiffToMs(
  uint64_t fromSamples, uint64_t toSamples) const {
  return (unsigned)std::min(
    (toSamples - fromSamples) * 1000 / m_SampleRate, (uint64_t)UINT_MAX);
}

GOSoundSampler *GOSoundEngine::CreateTaskSample(
  const GOSoundProvider *pSoundProvider,
  int samplerTaskId,
  unsigned audioGroup,
  unsigned velocity,
  unsigned delay,
  uint64_t prevEventTime,
  bool isRelease,
  uint64_t *pStartTimeSamples) {
  unsigned delay_samples = (delay * m_SampleRate) / (1000);
  uint64_t start_time = m_CurrentTime + delay_samples;
  unsigned eventIntervalMs = SamplesDiffToMs(prevEventTime, start_time);

  GOSoundSampler *sampler = nullptr;
  const GOSoundAudioSection *section = isRelease
    ? pSoundProvider->GetRelease(BOOL3_DEFAULT, eventIntervalMs)
    : pSoundProvider->GetAttack(velocity, eventIntervalMs);

  if (pStartTimeSamples) {
    *pStartTimeSamples = start_time;
  }
  if (section && section->GetChannels()) {
    sampler = m_SamplerPool.GetSampler();
    if (sampler) {
      sampler->p_SoundProvider = pSoundProvider;
      sampler->m_WaveTremulantStateFor = section->GetWaveTremulantStateFor();
      sampler->velocity = velocity;
      sampler->stream.InitStream(
        &m_resample,
        section,
        m_interpolation,
        GetRandomFactor() * pSoundProvider->GetTuning() / (float)m_SampleRate);

      const float playback_gain
        = pSoundProvider->GetGain() * section->GetNormGain();

      sampler->fader.Setup(
        playback_gain, pSoundProvider->GetVelocityVolume(velocity));
      sampler->delay = delay_samples;
      sampler->time = start_time;
      sampler->toneBalanceFilterState.Init(
        sampler->p_SoundProvider->GetToneBalance()->GetFilter());
      sampler->is_release = isRelease;
      sampler->m_SamplerTaskId = samplerTaskId;
      sampler->m_AudioGroupId = audioGroup;
      StartSampler(sampler);
    }
  }
  return sampler;
}

void GOSoundEngine::SwitchToAnotherAttack(GOSoundSampler *pSampler) {
  const GOSoundProvider *pProvider = pSampler->p_SoundProvider;

  if (pProvider && !pSampler->is_release) {
    const GOSoundAudioSection *section
      = pProvider->GetAttack(pSampler->velocity, 1000);

    if (section) {
      GOSoundSampler *new_sampler = m_SamplerPool.GetSampler();

      if (new_sampler != NULL) {
        float gain_target = pProvider->GetGain() * section->GetNormGain();
        unsigned crossFadeSamples
          = MsToSamples(pProvider->GetAttackSwitchCrossfadeLength());

        // copy old sampler to the new one
        *new_sampler = *pSampler;

        // start decay in the new sampler
        new_sampler->is_release = true;
        new_sampler->time = m_CurrentTime;
        new_sampler->fader.StartDecreasingVolume(crossFadeSamples);

        // start new section stream in the old sampler
        pSampler->m_WaveTremulantStateFor = section->GetWaveTremulantStateFor();
        pSampler->stream.InitAlignedStream(
          section, m_interpolation, &new_sampler->stream);
        pSampler->p_SoundProvider = pProvider;
        pSampler->time = m_CurrentTime + 1;

        pSampler->fader.Setup(
          gain_target,
          new_sampler->fader.GetVelocityVolume(),
          crossFadeSamples);
        pSampler->is_release = false;

        new_sampler->toneBalanceFilterState.Init(
          new_sampler->p_SoundProvider->GetToneBalance()->GetFilter());

        StartSampler(new_sampler);
      }
    }
  }
}

void GOSoundEngine::CreateReleaseSampler(GOSoundSampler *handle) {
  if (!handle->p_SoundProvider)
    return;

  /* The beloow code creates a new sampler to playback the release, the
   * following code takes the active sampler for this pipe (which will be
   * in either the attack or loop section) and sets the fadeout property
   * which will decay this portion of the pipe. The sampler will
   * automatically be placed back in the pool when the fade restores to
   * zero. */
  const GOSoundProvider *this_pipe = handle->p_SoundProvider;
  const GOSoundAudioSection *release_section = this_pipe->GetRelease(
    handle->m_WaveTremulantStateFor,
    SamplesDiffToMs(handle->time, m_CurrentTime));
  unsigned crossFadeSamples = MsToSamples(
    release_section ? release_section->GetReleaseCrossfadeLength()
                    : this_pipe->GetAttackSwitchCrossfadeLength());

  handle->fader.StartDecreasingVolume(crossFadeSamples);
  handle->is_release = true;

  int taskId = handle->m_SamplerTaskId;
  float vol = isWindchestTask(taskId)
    ? m_WindchestTasks[windchestTaskToIndex(taskId)]->GetWindchestVolume()
    : 1.0f;

  // FIXME: this is wrong... the intention is to not create a release for a
  // sample being played back with zero amplitude but this is a comparison
  // against a double. We should test against a minimum level.
  if (vol && release_section) {
    GOSoundSampler *new_sampler = m_SamplerPool.GetSampler();
    if (new_sampler != NULL) {
      new_sampler->p_SoundProvider = this_pipe;
      new_sampler->time = m_CurrentTime + 1;
      new_sampler->m_WaveTremulantStateFor
        = release_section->GetWaveTremulantStateFor();

      unsigned gain_decay_length = 0;
      float gain_target = this_pipe->GetGain() * release_section->GetNormGain();
      const bool not_a_tremulant = isWindchestTask(handle->m_SamplerTaskId);

      if (not_a_tremulant) {
        /* Because this sampler is about to be moved to a detached
         * windchest, we must apply the gain of the existing windchest
         * to the gain target for this fader - otherwise the playback
         * volume on the detached chest will not match the volume on
         * the existing chest. */
        gain_target *= vol;
        if (m_ScaledReleases) {
          /* Note: "time" is in milliseconds. */
          int time = ((m_CurrentTime - handle->time) * 1000) / m_SampleRate;
          /* TODO: below code should be replaced by a more accurate model of the
           * attack to get a better estimate of the amplitude when playing very
           * short notes estimating attack duration from pipe midi pitch */
          unsigned midikey_frequency = this_pipe->GetMidiKeyNumber();
          /* if MidiKeyNumber is not within an organ 64 feet to 1 foot pipes, we
           * assume average pipe (MIDI = 60)*/
          if (midikey_frequency > 133 || midikey_frequency == 0)
            midikey_frequency = 60;
          /* attack duration is assumed 50 ms above MIDI 96, 800 ms below MIDI
           * 24 and linear in between */
          float attack_duration = 50.0f;
          if (midikey_frequency < 96) {
            if (midikey_frequency < 24)
              attack_duration = 500.0f;
            else
              attack_duration
                = 500.0f + ((24.0f - (float)midikey_frequency) * 6.25f);
          }
          /* calculate gain (gain_target) to apply to tail amplitude in function
           * of when the note is released during the attack */
          if (time < (int)attack_duration) {
            float attack_index = (float)time / attack_duration;
            float gain_delta
              = (0.2f + (0.8f * (2.0f * attack_index - (attack_index * attack_index))));
            gain_target *= gain_delta;
          }
          /* calculate the volume decay to be applied to the release to take
           * into account the fact reverb is not completely formed during
           * staccato time to full reverb is estimated in function of release
           * length for an organ with a release length of 5 seconds or more,
           * time_to_full_reverb is around 350 ms for an organ with a release
           * length of 1 second or less, time_to_full_reverb is around 100 ms
           * time_to_full_reverb is linear in between */
          int time_to_full_reverb = ((60 * release_section->GetLength())
                                     / release_section->GetSampleRate())
            + 40;
          if (time_to_full_reverb > 350)
            time_to_full_reverb = 350;
          if (time_to_full_reverb < 100)
            time_to_full_reverb = 100;
          if (time < time_to_full_reverb) {
            /* in function of note duration, fading happens between:
             * 200 ms and 6 s for release with little reverberation e.g. short
             * release
             * 700 ms and 6 s for release with large reverberation e.g. long
             * release */
            gain_decay_length
              = time_to_full_reverb + 6000 * time / time_to_full_reverb;
          }
        }
      }

      const unsigned releaseLength = this_pipe->GetReleaseTail();

      new_sampler->fader.Setup(
        gain_target, handle->fader.GetVelocityVolume(), crossFadeSamples);

      if (
        releaseLength > 0
        && (releaseLength < gain_decay_length || gain_decay_length == 0))
        gain_decay_length = releaseLength;

      if (gain_decay_length > 0)
        new_sampler->fader.StartDecreasingVolume(
          MsToSamples(gain_decay_length));

      if (
        m_ReleaseAlignmentEnabled
        && release_section->SupportsStreamAlignment()) {
        new_sampler->stream.InitAlignedStream(
          release_section, m_interpolation, &handle->stream);
      } else {
        new_sampler->stream.InitStream(
          &m_resample,
          release_section,
          m_interpolation,
          this_pipe->GetTuning() / (float)m_SampleRate);
      }
      new_sampler->is_release = true;

      new_sampler->m_SamplerTaskId = not_a_tremulant
        ? /* detached releases are enabled and the pipe was on a regular
           * windchest. Play the release on the detached windchest */
        DETACHED_RELEASE_TASK_ID
        /* detached releases are disabled (or this isn't really a pipe)
         * so put the release on the same windchest as the pipe (which
         * means it will still be affected by tremulants - yuck). */
        : handle->m_SamplerTaskId;
      new_sampler->m_AudioGroupId = handle->m_AudioGroupId;
      new_sampler->toneBalanceFilterState.Init(
        new_sampler->p_SoundProvider->GetToneBalance()->GetFilter());
      StartSampler(new_sampler);
      handle->time = m_CurrentTime;
    }
  }
}

uint64_t GOSoundEngine::StopSample(
  const GOSoundProvider *pipe, GOSoundSampler *handle) {
  assert(handle);
  assert(pipe);

  // The following condition could arise if a one-shot sample is played,
  // decays away (and hence the sampler is discarded back into the pool), and
  // then the user releases a key. If the sampler had already been reused
  // with another pipe, that sample would erroneously be told to decay.
  if (pipe != handle->p_SoundProvider)
    return 0;

  handle->stop = m_CurrentTime + handle->delay;
  return handle->stop;
}

void GOSoundEngine::SwitchSample(
  const GOSoundProvider *pipe, GOSoundSampler *handle) {
  assert(handle);
  assert(pipe);

  // The following condition could arise if a one-shot sample is played,
  // decays away (and hence the sampler is discarded back into the pool), and
  // then the user releases a key. If the sampler had already been reused
  // with another pipe, that sample would erroneously be told to decay.
  if (pipe != handle->p_SoundProvider)
    return;

  handle->new_attack = m_CurrentTime + handle->delay;
}

void GOSoundEngine::UpdateVelocity(
  const GOSoundProvider *pipe, GOSoundSampler *handle, unsigned velocity) {
  assert(handle);
  assert(pipe);

  if (handle->p_SoundProvider == pipe) {
    // we've just checked that handle is still playing the same pipe
    // may be handle was switched to another pipe between checking and
    // SetVelocityVolume but we don't want to lock it because this functionality
    // is not so important Concurrent update possible, as it just update a float
    handle->velocity = velocity;
    handle->fader.SetVelocityVolume(pipe->GetVelocityVolume(velocity));
  }
}

const std::vector<double> &GOSoundEngine::GetMeterInfo() {
  m_MeterInfo[0] = m_UsedPolyphony.load() / (double)GetHardPolyphony();
  m_UsedPolyphony.store(0);

  for (unsigned i = 1; i < m_MeterInfo.size(); i++)
    m_MeterInfo[i] = 0;
  for (unsigned i = 0, nr = 1; i < m_AudioOutputTasks.size(); i++) {
    if (!m_AudioOutputTasks[i])
      continue;
    const std::vector<float> &info = m_AudioOutputTasks[i]->GetMeterInfo();
    for (unsigned j = 0; j < info.size(); j++)
      m_MeterInfo[nr++] = info[j];
    m_AudioOutputTasks[i]->ResetMeterInfo();
  }
  return m_MeterInfo;
}
