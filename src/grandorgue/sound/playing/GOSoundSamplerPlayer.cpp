/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundSamplerPlayer.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>

#include "GOSoundReleaseAlignTable.h"
#include "GOSoundSampler.h"
#include "sound/providers/GOSoundProvider.h"
#include "sound/tasks/GOSoundGroupTask.h"
#include "sound/tasks/GOSoundReleaseTask.h"
#include "sound/tasks/GOSoundTremulantTask.h"
#include "sound/tasks/GOSoundWindchestTask.h"

/*
 * Constructor
 */

GOSoundSamplerPlayer::GOSoundSamplerPlayer(
  ptr_vector<GOSoundGroupTask> &audioGroupTasks,
  std::vector<std::unique_ptr<GOSoundWindchestTask>> &windchestTasks,
  ptr_vector<GOSoundTremulantTask> &tremulantTasks,
  std::unique_ptr<GOSoundReleaseTask> &pReleaseTask)
  : r_AudioGroupTasks(audioGroupTasks),
    r_WindchestTasks(windchestTasks),
    r_TremulantTasks(tremulantTasks),
    rp_ReleaseTask(pReleaseTask),
    m_IsScaledReleases(true),
    m_IsReleaseAlignmentEnabled(true),
    m_IsRandomizeSpeaking(true),
    m_IsPolyphonyLimiting(true),
    m_PolyphonySoftLimit(0),
    m_InterpolationType(GOSoundResample::GO_LINEAR_INTERPOLATION),
    m_SampleRate(0),
    m_CurrentTime(1),
    m_UsedPolyphony(0) {
  m_SamplerPool.SetUsageLimit(2048);
  m_PolyphonySoftLimit = (m_SamplerPool.GetUsageLimit() * 3) / 4;
}

/*
 * Configuration getters and setters
 */

void GOSoundSamplerPlayer::SetHardPolyphony(unsigned polyphony) {
  m_SamplerPool.SetUsageLimit(polyphony);
  m_PolyphonySoftLimit = (m_SamplerPool.GetUsageLimit() * 3) / 4;
}

/*
 * Lifecycle
 */

void GOSoundSamplerPlayer::Build(unsigned sampleRate) {
  assert(rp_ReleaseTask);
  assert(r_AudioGroupTasks.size() > 0);
  assert(!r_WindchestTasks.empty());
  m_SampleRate = sampleRate;
}

void GOSoundSamplerPlayer::Destroy() { m_SampleRate = 0; }

void GOSoundSamplerPlayer::Reset() {
  m_UsedPolyphony.store(0);
  m_SamplerPool.ReturnAll();
  m_CurrentTime = 1;
}

void GOSoundSamplerPlayer::AdvanceTime(unsigned nSamplesPerBuffer) {
  m_CurrentTime += nSamplesPerBuffer;

  const unsigned usedSamplers = m_SamplerPool.UsedSamplerCount();

  if (usedSamplers > m_UsedPolyphony.load())
    m_UsedPolyphony.store(usedSamplers);
}

/*
 * Organ interface (from GOSoundOrganInterface)
 */

unsigned GOSoundSamplerPlayer::SamplesDiffToMs(
  uint64_t fromSamples, uint64_t toSamples) const {
  return (unsigned)std::min(
    (toSamples - fromSamples) * 1000 / m_SampleRate, (uint64_t)UINT_MAX);
}

float GOSoundSamplerPlayer::GetRandomFactor() const {
  float result = 1;

  if (m_IsRandomizeSpeaking) {
    const double factor = (pow(2, 1.0 / 1200.0) - 1) / (RAND_MAX / 2);
    int num = rand() - RAND_MAX / 2;

    result = 1 + num * factor;
  }
  return result;
}

void GOSoundSamplerPlayer::PassSampler(GOSoundSampler *sampler) {
  int taskId = sampler->m_SamplerTaskId;

  if (isWindchestTask(taskId))
    r_AudioGroupTasks[sampler->m_AudioGroupId]->Add(sampler);
  else
    r_TremulantTasks[tremulantTaskToIndex(taskId)]->Add(sampler);
}

void GOSoundSamplerPlayer::StartSampler(GOSoundSampler *sampler) {
  int taskId = sampler->m_SamplerTaskId;

  sampler->stop = 0;
  sampler->new_attack = 0;
  sampler->p_WindchestTask = isWindchestTask(taskId)
    ? r_WindchestTasks[windchestTaskToIndex(taskId)].get()
    : nullptr;
  PassSampler(sampler);
}

bool GOSoundSamplerPlayer::ProcessSampler(
  float *output_buffer,
  GOSoundSampler *sampler,
  unsigned n_frames,
  float volume) {
  float temp[n_frames * 2];
  const bool process_sampler = (sampler->time <= m_CurrentTime);

  if (process_sampler) {
    if (sampler->is_release &&
        ((m_IsPolyphonyLimiting &&
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
      rp_ReleaseTask->Add(sampler);
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

void GOSoundSamplerPlayer::ProcessRelease(GOSoundSampler *sampler) {
  if (sampler->stop) {
    CreateReleaseSampler(sampler);
    sampler->stop = 0;
  } else if (sampler->new_attack) {
    SwitchToAnotherAttack(sampler);
    sampler->new_attack = 0;
  }
  PassSampler(sampler);
}

void GOSoundSamplerPlayer::ReturnSampler(GOSoundSampler *sampler) {
  m_SamplerPool.ReturnSampler(sampler);
}

GOSoundSampler *GOSoundSamplerPlayer::CreateTaskSample(
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
        m_InterpolationType,
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

void GOSoundSamplerPlayer::SwitchToAnotherAttack(GOSoundSampler *pSampler) {
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
          section, m_InterpolationType, &new_sampler->stream);
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

void GOSoundSamplerPlayer::CreateReleaseSampler(GOSoundSampler *handle) {
  if (!handle->p_SoundProvider)
    return;

  /* The below code creates a new sampler to playback the release, the
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
    ? r_WindchestTasks[windchestTaskToIndex(taskId)]->GetWindchestAmplitude()
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
        if (m_IsScaledReleases) {
          /* Note: "time" is in milliseconds. */
          int time = ((m_CurrentTime - handle->time) * 1000) / m_SampleRate;
          /* TODO: below code should be replaced by a more accurate model of the
           * attack to get a better estimate of the amplitude when playing very
           * short notes; estimating attack duration from pipe MIDI pitch */
          unsigned midikey_frequency = this_pipe->GetMidiKeyNumber();
          /* if MidiKeyNumber is not within the range of organ pipes (64 feet
           * to 1 foot), we assume average pipe (MIDI = 60) */
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
          /* calculate gain (gain_target) to apply to tail amplitude as a
           * function of when the note is released during the attack */
          if (time < (int)attack_duration) {
            float attack_index = (float)time / attack_duration;
            float gain_delta
              = (0.2f + (0.8f * (2.0f * attack_index - (attack_index * attack_index))));
            gain_target *= gain_delta;
          }
          /* calculate the volume decay to be applied to the release to take
           * into account the fact that reverb is not completely formed during
           * staccato. Time to full reverb is estimated as a function of release
           * length: for an organ with a release length of 5 seconds or more,
           * time_to_full_reverb is around 350 ms; for an organ with a release
           * length of 1 second or less, time_to_full_reverb is around 100 ms;
           * time_to_full_reverb is linear in between */
          int time_to_full_reverb = ((60 * release_section->GetLength())
                                     / release_section->GetSampleRate())
            + 40;
          if (time_to_full_reverb > 350)
            time_to_full_reverb = 350;
          if (time_to_full_reverb < 100)
            time_to_full_reverb = 100;
          if (time < time_to_full_reverb) {
            /* as a function of note duration, fading happens between:
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
        m_IsReleaseAlignmentEnabled
        && release_section->SupportsStreamAlignment()) {
        new_sampler->stream.InitAlignedStream(
          release_section, m_InterpolationType, &handle->stream);
      } else {
        new_sampler->stream.InitStream(
          &m_resample,
          release_section,
          m_InterpolationType,
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

GOSoundSampler *GOSoundSamplerPlayer::StartPipeSample(
  const GOSoundProvider *pipeProvider,
  unsigned windchestN,
  unsigned audioGroup,
  unsigned velocity,
  unsigned delay,
  uint64_t prevEventTime,
  bool isRelease,
  uint64_t *pStartTimeSamples) {
  return CreateTaskSample(
    pipeProvider,
    windchestN,
    audioGroup,
    velocity,
    delay,
    prevEventTime,
    isRelease,
    pStartTimeSamples);
}

GOSoundSampler *GOSoundSamplerPlayer::StartTremulantSample(
  const GOSoundProvider *tremProvider,
  unsigned tremulantN,
  uint64_t prevEventTime) {
  return CreateTaskSample(
    tremProvider, -tremulantN, 0, 0x7f, 0, prevEventTime, false, nullptr);
}

uint64_t GOSoundSamplerPlayer::StopSample(
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

void GOSoundSamplerPlayer::SwitchSample(
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

void GOSoundSamplerPlayer::UpdateVelocity(
  const GOSoundProvider *pipe, GOSoundSampler *handle, unsigned velocity) {
  assert(handle);
  assert(pipe);

  if (handle->p_SoundProvider == pipe) {
    // we've just checked that handle is still playing the same pipe;
    // maybe handle was switched to another pipe between checking and
    // SetVelocityVolume, but we don't want to lock it because this
    // functionality is not so important. Concurrent update is acceptable,
    // as it just updates a float
    handle->velocity = velocity;
    handle->fader.SetVelocityVolume(pipe->GetVelocityVolume(velocity));
  }
}
