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
#include "model/GOWindchest.h"
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
    mp_TouchTask(std::make_unique<GOSoundTouchTask>(r_MemoryPool)),
    m_SamplerPlayer(
      mp_AudioGroupTasks, mp_WindchestTasks, mp_TremulantTasks, mp_ReleaseTask),
    m_NAudioGroups(1),
    m_NAuxThreads(0),
    m_IsDownmix(false),
    m_NReleaseRepeats(1),
    m_ReverbConfig(GOSoundReverb::CONFIG_REVERB_DISABLED),
    m_NSamplesPerBuffer(1),
    m_LifecycleState(LifecycleState::IDLE),
    p_AudioRecorder(nullptr) {
  SetVolume(-15);
  mp_ReleaseTask
    = std::make_unique<GOSoundReleaseTask>(m_SamplerPlayer, mp_AudioGroupTasks);
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
  p_AudioRecorder = &recorder;

  // [B1] Build audio group tasks
  std::vector<GOSoundBufferTaskBase *> groupOutputs;

  for (unsigned groupI = 0; groupI < m_NAudioGroups; groupI++) {
    GOSoundGroupTask *pGroupTask
      = new GOSoundGroupTask(m_SamplerPlayer, m_NSamplesPerBuffer);

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
      m_ReverbConfig, m_NSamplesPerBuffer, sampleRate);
  for (auto &pTask : mp_AudioOutputTasks)
    pTask->SetupReverb(m_ReverbConfig, m_NSamplesPerBuffer, sampleRate);

  // [B7] Build tremulant tasks
  for (unsigned n = r_OrganModel.GetTremulantCount(), tremI = 0; tremI < n;
       tremI++)
    mp_TremulantTasks.push_back(
      new GOSoundTremulantTask(m_SamplerPlayer, m_NSamplesPerBuffer));

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

  m_SamplerPlayer.Build(sampleRate);
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

  m_SamplerPlayer.Destroy();
  m_LifecycleState.store(LifecycleState::IDLE);
}

void GOSoundOrganEngine::ResetCounters() {
  m_SamplerPlayer.Reset();
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

  // AdvanceTime advances m_CurrentTime and records peak used polyphony
  // (both previously done inline here; now delegated to GOSoundSamplerPlayer).
  m_SamplerPlayer.AdvanceTime(m_NSamplesPerBuffer);

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
    // GetAndResetUsedPolyphony() reads accumulated peak polyphony and resets it
    *(pResult++) = m_SamplerPlayer.GetAndResetUsedPolyphony()
      / static_cast<float>(hardPolyphony);
    for (std::atomic<float> &v : m_MeterInfo)
      *(pResult++) = v.exchange(0.0f);
  }
  return result;
}
