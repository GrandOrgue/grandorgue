/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundOrganEngine.h"

#include <algorithm>

#include "buffer/GOSoundBufferMutable.h"
#include "config/GOConfig.h"
#include "model/GOOrganModel.h"
#include "model/GOPipe.h"
#include "model/GOWindchest.h"
#include "playing/GOSoundReleaseAlignTable.h"
#include "playing/GOSoundSampler.h"
#include "providers/GOSoundProvider.h"
#include "scheduler/GOSoundThread.h"
#include "tasks/GOSoundGroupTask.h"
#include "tasks/GOSoundOutputTask.h"
#include "tasks/GOSoundReleaseTask.h"
#include "tasks/GOSoundTouchTask.h"
#include "tasks/GOSoundTremulantTask.h"
#include "tasks/GOSoundWindchestTask.h"
#include "threading/GOMutexLocker.h"

#include "GOEvent.h"
#include "GOSoundRecorder.h"

/*
 * Factory functions
 */

std::vector<GOSoundOrganEngine::AudioOutputConfig> GOSoundOrganEngine::
  createAudioOutputConfigs(GOConfig &config, unsigned nAudioGroups) {
  std::vector<GOAudioDeviceConfig> &audioDeviceConfig
    = config.GetAudioDeviceConfig();
  const unsigned nDevices = audioDeviceConfig.size();

  std::vector<AudioOutputConfig> result(nDevices);

  for (unsigned deviceI = 0; deviceI < nDevices; deviceI++) {
    const GOAudioDeviceConfig &deviceConfig = audioDeviceConfig[deviceI];
    const auto &deviceOutputs = deviceConfig.GetChannelOututs();
    AudioOutputConfig &outConfig = result[deviceI];

    outConfig.channels = deviceConfig.GetChannels();
    outConfig.scaleFactors.resize(outConfig.channels);

    for (unsigned channelI = 0; channelI < outConfig.channels; channelI++) {
      std::vector<float> &scaleFactors = outConfig.scaleFactors[channelI];

      scaleFactors.resize(nAudioGroups * 2);
      std::fill(
        scaleFactors.begin(),
        scaleFactors.end(),
        GOAudioDeviceConfig::MUTE_VOLUME);

      if (channelI < deviceOutputs.size()) {
        for (const auto &groupOutput : deviceOutputs[channelI]) {
          int id = config.GetStrictAudioGroupId(groupOutput.GetName());

          if (id >= 0) {
            scaleFactors[id * 2] = groupOutput.GetLeft();
            scaleFactors[id * 2 + 1] = groupOutput.GetRight();
          }
        }
      }
    }
  }
  return result;
}

std::vector<GOSoundOrganEngine::AudioOutputConfig> GOSoundOrganEngine::
  createDefaultOutputConfigs(unsigned nAudioGroups) {
  AudioOutputConfig config;

  config.channels = 2;
  config.scaleFactors.resize(2);
  config.scaleFactors[0].resize(
    nAudioGroups * 2, GOAudioDeviceConfig::MUTE_VOLUME);
  config.scaleFactors[1].resize(
    nAudioGroups * 2, GOAudioDeviceConfig::MUTE_VOLUME);

  for (unsigned groupI = 0; groupI < nAudioGroups; groupI++) {
    config.scaleFactors[0][groupI * 2] = 0.0f;
    config.scaleFactors[1][groupI * 2 + 1] = 0.0f;
  }
  return {config};
}

/*
 * Constructors and destructors
 */

GOSoundOrganEngine::GOSoundOrganEngine(
  GOOrganModel &organModel, GOMemoryPool &memoryPool)
  : r_OrganModel(organModel),
    r_MemoryPool(memoryPool),
    mp_ReleaseTask(
      std::make_unique<GOSoundReleaseTask>(*this, mp_AudioGroupTasks)),
    mp_TouchTask(std::make_unique<GOSoundTouchTask>(r_MemoryPool)),
    m_NAudioGroups(1),
    m_NAuxThreads(0),
    m_IsDownmix(false),
    m_NReleaseRepeats(1),
    m_IsPolyphonyLimiting(true),
    m_PolyphonySoftLimit(0),
    m_IsScaledReleases(true),
    m_IsReleaseAlignmentEnabled(true),
    m_IsRandomizeSpeaking(true),
    m_InterpolationType(GOSoundResample::GO_LINEAR_INTERPOLATION),
    m_ReverbConfig(GOSoundReverb::CONFIG_REVERB_DISABLED),
    m_NSamplesPerBuffer(1),
    m_SampleRate(0),
    m_LifecycleState(LifecycleState::IDLE),
    p_AudioRecorder(nullptr),
    m_CurrentTime(1),
    m_UsedPolyphony(0) {
  SetVolume(-15);
  m_SamplerPool.SetUsageLimit(2048);
  m_PolyphonySoftLimit = (m_SamplerPool.GetUsageLimit() * 3) / 4;
}

// The destructor body is empty, but it must be defined here (not in the header)
// so that std::unique_ptr can call the complete destructors of its managed
// types (GOSoundReleaseTask, GOSoundTouchTask, GOSoundThread), which are only
// forward-declared in the header file.
GOSoundOrganEngine::~GOSoundOrganEngine() {}

/*
 * Configuration getters and setters
 */

// TODO: rename to SetGain(int gain), m_volume → m_gain, m_gain → m_amplitude
void GOSoundOrganEngine::SetVolume(int volume) {
  m_volume = volume;
  m_gain = powf(10.0f, m_volume * 0.05f);
}

void GOSoundOrganEngine::SetHardPolyphony(unsigned polyphony) {
  m_SamplerPool.SetUsageLimit(polyphony);
  m_PolyphonySoftLimit = (m_SamplerPool.GetUsageLimit() * 3) / 4;
}

void GOSoundOrganEngine::SetFromConfig(GOConfig &config) {
  const unsigned nAudioGroups = config.GetAudioGroups().size();

  SetNAudioGroups(nAudioGroups >= 1 ? nAudioGroups : 1);
  SetNAuxThreads(config.Concurrency());
  SetDownmix(config.RecordDownmix());
  SetNReleaseRepeats(config.ReleaseConcurrency());
  SetPolyphonyLimiting(config.ManagePolyphony());
  SetHardPolyphony(config.PolyphonyLimit());
  SetScaledReleases(config.ScaleRelease());
  SetRandomizeSpeaking(config.RandomizeSpeaking());
  SetInterpolationType(config.m_InterpolationType());
  SetReverbConfig(GOSoundReverb::createReverbConfig(config));
}

/*
 * Lifecycle functions
 */

void GOSoundOrganEngine::BuildEngine(
  const std::vector<AudioOutputConfig> &audioOutputConfigs,
  unsigned nSamplesPerBuffer,
  unsigned sampleRate,
  GOSoundRecorder &recorder) {
  GOMutexLocker locker(m_LifecycleMutex);

  assert(m_LifecycleState.load() == LifecycleState::IDLE);

  m_NSamplesPerBuffer = nSamplesPerBuffer;
  m_SampleRate = sampleRate;
  p_AudioRecorder = &recorder;

  // [B1] Build audio group tasks
  std::vector<GOSoundBufferTaskBase *> groupOutputs;

  for (unsigned groupI = 0; groupI < m_NAudioGroups; groupI++) {
    GOSoundGroupTask *pGroupTask
      = new GOSoundGroupTask(*this, m_NSamplesPerBuffer);

    mp_AudioGroupTasks.push_back(pGroupTask);
    groupOutputs.push_back(pGroupTask);
  }

  // [B2] Build audio output tasks (per-device only)
  unsigned nTotalChannels = 0;

  for (unsigned deviceI = 0; deviceI < audioOutputConfigs.size(); deviceI++) {
    const AudioOutputConfig &devConfig = audioOutputConfigs[deviceI];
    const unsigned nChannels = devConfig.channels;
    std::vector<float> scaleFactors;

    scaleFactors.resize(m_NAudioGroups * nChannels * 2);
    std::fill(scaleFactors.begin(), scaleFactors.end(), 0.0f);
    for (unsigned channelI = 0; channelI < nChannels; channelI++) {
      for (unsigned k = 0; k < devConfig.scaleFactors[channelI].size(); k++) {
        if (k >= m_NAudioGroups * 2)
          break;
        float factor = devConfig.scaleFactors[channelI][k];
        if (factor >= -120 && factor < 40)
          factor = powf(10.0f, factor * 0.05f);
        else
          factor = 0;
        scaleFactors[channelI * m_NAudioGroups * 2 + k] = factor;
      }
    }
    mp_AudioOutputTasks.push_back(std::make_unique<GOSoundOutputTask>(
      nChannels, scaleFactors, m_NSamplesPerBuffer));
    mp_AudioOutputTasks.back()->SetOutputs(groupOutputs);
    nTotalChannels += nChannels;
  }

  // [B3] Resize meter info to match real output channels.
  // std::atomic is not copyable/movable, so we construct a fresh vector and
  // swap instead of resize.
  {
    std::vector<std::atomic<float>> newMeterInfo(nTotalChannels);

    m_MeterInfo.swap(newMeterInfo);
  }

  // [B4] Build downmix task (optional stereo mix for recorder)
  if (m_IsDownmix) {
    std::vector<float> scaleFactors;

    scaleFactors.resize(m_NAudioGroups * 2 * 2);
    std::fill(scaleFactors.begin(), scaleFactors.end(), 0.0f);
    for (unsigned groupI = 0; groupI < m_NAudioGroups; groupI++) {
      scaleFactors[groupI * 4] = 1;
      scaleFactors[groupI * 4 + 3] = 1;
    }
    mp_DownmixTask = std::make_unique<GOSoundOutputTask>(
      2, scaleFactors, m_NSamplesPerBuffer);
    mp_DownmixTask->SetOutputs(groupOutputs);
  }

  // [B5] Set up recorder outputs
  {
    std::vector<GOSoundBufferTaskBase *> recorderOutputs;

    if (mp_DownmixTask)
      recorderOutputs.push_back(mp_DownmixTask.get());
    else
      for (auto &pTask : mp_AudioOutputTasks)
        recorderOutputs.push_back(pTask.get());
    p_AudioRecorder->SetOutputs(recorderOutputs, m_NSamplesPerBuffer);
  }

  // [B6] Set up reverb
  if (mp_DownmixTask)
    mp_DownmixTask->SetupReverb(
      m_ReverbConfig, m_NSamplesPerBuffer, m_SampleRate);
  for (auto &pTask : mp_AudioOutputTasks)
    pTask->SetupReverb(m_ReverbConfig, m_NSamplesPerBuffer, m_SampleRate);

  // [B7] Build tremulant tasks
  for (unsigned n = r_OrganModel.GetTremulantCount(), tremI = 0; tremI < n;
       tremI++)
    mp_TremulantTasks.push_back(
      new GOSoundTremulantTask(*this, m_NSamplesPerBuffer));

  // [B8] Build windchest tasks
  // Special windchest task for detached releases (index 0 =
  // DETACHED_RELEASE_TASK_ID)
  mp_WindchestTasks.push_back(
    std::make_unique<GOSoundWindchestTask>(*this, nullptr));
  for (unsigned n = r_OrganModel.GetWindchestCount(), wcI = 0; wcI < n; wcI++)
    mp_WindchestTasks.push_back(std::make_unique<GOSoundWindchestTask>(
      *this, r_OrganModel.GetWindchest(wcI)));

  // [B9] Initialize windchests with tremulant tasks
  for (auto &pWcTask : mp_WindchestTasks)
    pWcTask->Init(mp_TremulantTasks);

  // [B10] Add all tasks to scheduler
  m_Scheduler.Clear();
  m_Scheduler.SetRepeatCount(m_NReleaseRepeats);
  for (GOSoundTremulantTask *pTremTask : mp_TremulantTasks)
    m_Scheduler.Add(pTremTask);
  for (auto &pWcTask : mp_WindchestTasks)
    m_Scheduler.Add(pWcTask.get());
  for (GOSoundGroupTask *pGroupTask : mp_AudioGroupTasks)
    m_Scheduler.Add(pGroupTask);
  if (mp_DownmixTask)
    m_Scheduler.Add(mp_DownmixTask.get());
  for (auto &pOutputTask : mp_AudioOutputTasks)
    m_Scheduler.Add(pOutputTask.get());
  m_Scheduler.Add(p_AudioRecorder);
  m_Scheduler.Add(mp_ReleaseTask.get());
  m_Scheduler.Add(mp_TouchTask.get());

  // [B11] Build worker threads
  for (unsigned threadI = 0; threadI < m_NAuxThreads; threadI++)
    mp_threads.push_back(std::make_unique<GOSoundThread>(&m_Scheduler));
  for (auto &pThread : mp_threads)
    pThread->Run();

  m_LifecycleState.store(LifecycleState::BUILT);
}

void GOSoundOrganEngine::DestroyEngine() {
  GOMutexLocker locker(m_LifecycleMutex);

  assert(m_LifecycleState.load() == LifecycleState::BUILT);

  // [B11] Destroy worker threads
  for (auto &pThread : mp_threads)
    pThread->Delete();
  mp_threads.clear();

  // [B10] Clear scheduler
  m_Scheduler.Clear();

  // [B9] + [B8] Destroy windchest tasks (drops Init() connections too)
  mp_WindchestTasks.clear();

  // [B7] Destroy tremulant tasks
  mp_TremulantTasks.clear();

  // [B6] Reverb — no explicit cleanup (owned by output tasks below)
  // [B5] Recorder outputs — no explicit cleanup (recorder is non-owning)

  // [B4] Destroy downmix task
  mp_DownmixTask.reset();

  // [B3] Clear meter info
  m_MeterInfo.clear();

  // [B2] Destroy audio output tasks
  mp_AudioOutputTasks.clear();

  // [B1] Destroy audio group tasks
  for (GOSoundGroupTask *pGroupTask : mp_AudioGroupTasks)
    pGroupTask->WaitAndClear();
  mp_AudioGroupTasks.clear();

  m_LifecycleState.store(LifecycleState::IDLE);
}

void GOSoundOrganEngine::ResetCounters() {
  m_UsedPolyphony.store(0);
  m_SamplerPool.ReturnAll();
  m_CurrentTime = 1;
  m_Scheduler.Reset();
}

void GOSoundOrganEngine::StartEngine() {
  assert(m_LifecycleState.load() == LifecycleState::BUILT);
  ResetCounters();
  m_Scheduler.ResumeGivingWork();
  m_LifecycleState.store(LifecycleState::WORKING);
}

void GOSoundOrganEngine::StopEngine() {
  assert(m_LifecycleState.load() == LifecycleState::WORKING);
  m_Scheduler.PauseGivingWork();
  for (auto &pThread : mp_threads)
    pThread->WaitForIdle();
  m_LifecycleState.store(LifecycleState::BUILT);
}

void GOSoundOrganEngine::BuildAndStart(
  const std::vector<AudioOutputConfig> &audioOutputConfigs,
  unsigned nSamplesPerBuffer,
  unsigned sampleRate,
  GOSoundRecorder &recorder) {
  BuildEngine(audioOutputConfigs, nSamplesPerBuffer, sampleRate, recorder);
  StartEngine();
}

void GOSoundOrganEngine::StopAndDestroy() {
  StopEngine();
  DestroyEngine();
}

void GOSoundOrganEngine::SetUsed(bool isUsed) {
  const LifecycleState oldState = m_LifecycleState.load();

  assert(
    oldState >= LifecycleState::WORKING && oldState <= LifecycleState::USED);
  (void)oldState; // suppress unused-variable warning in Release (assert is
                  // compiled out)

  m_LifecycleState.store(
    isUsed ? LifecycleState::USED : LifecycleState::WORKING);
}

/*
 * Functions called from GOSoundSystem
 */

void GOSoundOrganEngine::GetAudioOutput(
  unsigned outputIndex, bool isLast, GOSoundBufferMutable &outBuffer) {
  if (IsWorking()) {
    GOSoundOutputTask *pOutputTask = mp_AudioOutputTasks[outputIndex].get();

    pOutputTask->Finish(isLast);
    outBuffer.CopyFrom(*pOutputTask);
  } else
    outBuffer.FillWithSilence();
}

/**
 * Atomically updates maxValue to max(maxValue, value) with relaxed ordering.
 * std::atomic<T>::fetch_max is only available in C++26.
 */
template <typename T>
static void atomic_fetch_max_relaxed(std::atomic<T> &maxValue, T value) {
  T oldMax = maxValue.load(std::memory_order_relaxed);

  while (oldMax < value
         && !maxValue.compare_exchange_weak(
           oldMax, value, std::memory_order_relaxed))
    ;
}

void GOSoundOrganEngine::NextPeriod() {
  assert(IsWorking());
  m_Scheduler.Exec();

  m_CurrentTime += m_NSamplesPerBuffer;
  atomic_fetch_max_relaxed(m_UsedPolyphony, m_SamplerPool.UsedSamplerCount());

  // Accumulate per-channel peak levels from each output task into m_MeterInfo
  // for the GUI meter display. Values accumulate between GUI polls;
  // GetMeterInfo() resets them via exchange(0). Only real device outputs
  // (mp_AudioOutputTasks) are counted; mp_DownmixTask is excluded.
  // Guarded by assert(IsWorking()) above: m_MeterInfo is valid in WORKING
  // state.
  const auto meterEnd = m_MeterInfo.end();
  auto meterIt = m_MeterInfo.begin();

  for (auto &pTask : mp_AudioOutputTasks) {
    for (const float f : pTask->GetMeterInfo()) {
      // m_MeterInfo.size() == nTotalChannels [B3] == sum of channels across
      // all mp_AudioOutputTasks [B2], so the iterator never overflows.
      assert(meterIt < meterEnd);
      atomic_fetch_max_relaxed(*meterIt++, f);
    }
    pTask->ResetMeterInfo();
  }

  m_Scheduler.Reset();
}

void GOSoundOrganEngine::WakeupThreads() {
  for (auto &pThread : mp_threads)
    pThread->Wakeup();
}

/*
 * Organ interface implementation (from GOSoundOrganInterface)
 */

unsigned GOSoundOrganEngine::SamplesDiffToMs(
  uint64_t fromSamples, uint64_t toSamples) const {
  return (unsigned)std::min(
    (toSamples - fromSamples) * 1000 / m_SampleRate, (uint64_t)UINT_MAX);
}

float GOSoundOrganEngine::GetRandomFactor() const {
  float result = 1;

  if (m_IsRandomizeSpeaking) {
    const double factor = (pow(2, 1.0 / 1200.0) - 1) / (RAND_MAX / 2);
    int num = rand() - RAND_MAX / 2;

    result = 1 + num * factor;
  }
  return result;
}

void GOSoundOrganEngine::PassSampler(GOSoundSampler *sampler) {
  int taskId = sampler->m_SamplerTaskId;

  if (isWindchestTask(taskId))
    mp_AudioGroupTasks[sampler->m_AudioGroupId]->Add(sampler);
  else
    mp_TremulantTasks[tremulantTaskToIndex(taskId)]->Add(sampler);
}

void GOSoundOrganEngine::StartSampler(GOSoundSampler *sampler) {
  int taskId = sampler->m_SamplerTaskId;

  sampler->stop = 0;
  sampler->new_attack = 0;
  sampler->p_WindchestTask = isWindchestTask(taskId)
    ? mp_WindchestTasks[windchestTaskToIndex(taskId)].get()
    : nullptr;
  PassSampler(sampler);
}

bool GOSoundOrganEngine::ProcessSampler(
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
      mp_ReleaseTask->Add(sampler);
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

void GOSoundOrganEngine::ProcessRelease(GOSoundSampler *sampler) {
  if (sampler->stop) {
    CreateReleaseSampler(sampler);
    sampler->stop = 0;
  } else if (sampler->new_attack) {
    SwitchToAnotherAttack(sampler);
    sampler->new_attack = 0;
  }
  PassSampler(sampler);
}

void GOSoundOrganEngine::ReturnSampler(GOSoundSampler *sampler) {
  m_SamplerPool.ReturnSampler(sampler);
}

GOSoundSampler *GOSoundOrganEngine::CreateTaskSample(
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

void GOSoundOrganEngine::SwitchToAnotherAttack(GOSoundSampler *pSampler) {
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

void GOSoundOrganEngine::CreateReleaseSampler(GOSoundSampler *handle) {
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
    ? mp_WindchestTasks[windchestTaskToIndex(taskId)]->GetWindchestVolume()
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

uint64_t GOSoundOrganEngine::StopSample(
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

void GOSoundOrganEngine::SwitchSample(
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

void GOSoundOrganEngine::UpdateVelocity(
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

/*
 * Other functions
 */

std::vector<float> GOSoundOrganEngine::GetMeterInfo() {
  // GUI thread: m_LifecycleMutex prevents concurrent BuildEngine/DestroyEngine
  // from modifying m_MeterInfo while we read it.
  GOMutexLocker locker(m_LifecycleMutex);

  // result[0] = polyphony ratio; result[1..] = per-channel peak levels.
  // When not working, m_MeterInfo may be empty; result contains only zeros.
  std::vector<float> result(m_MeterInfo.size() + 1, 0.0f);

  if (IsWorking()) {
    const unsigned hardPolyphony = GetHardPolyphony();
    float *pResult = result.data();

    assert(hardPolyphony > 0);
    // exchange(0) reads accumulated max and resets it for the next poll
    *(pResult++)
      = m_UsedPolyphony.exchange(0) / static_cast<float>(hardPolyphony);
    for (std::atomic<float> &v : m_MeterInfo)
      *(pResult++) = v.exchange(0.0f);
  }
  return result;
}
