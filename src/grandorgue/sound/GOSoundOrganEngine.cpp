/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundOrganEngine.h"

#include <algorithm>
#include <set>

#include "buffer/GOSoundBufferMutable.h"
#include "config/GOConfig.h"
#include "model/GOOrganModel.h"
#include "model/GOWindchest.h"
#include "scheduler/GOSchedulerThread.h"
#include "tasks/GOSoundGroupTask.h"
#include "tasks/GOSoundOutputTask.h"
#include "tasks/GOSoundReleaseTask.h"
#include "tasks/GOSoundTouchTask.h"
#include "tasks/GOSoundTremulantTask.h"
#include "tasks/GOSoundWindchestGroupTask.h"
#include "tasks/GOSoundWindchestTask.h"
#include "threading/GOMutexLocker.h"

#include "GOEvent.h"

/*
 * Factory functions
 */

std::vector<float> GOSoundOrganEngine::createDownmixGains(
  unsigned nAudioGroups) {
  // GOSoundOutputTask::DoRun() reads the linear form of this (see
  // convertGainToScaleFactor()) as a row-major [outChannelI][groupI * 2 +
  // groupChannelI] matrix with row stride nAudioGroups * 2 (not a fixed 4,
  // which only matches that row stride when nAudioGroups == 1): row 0 (L)
  // takes each group's left channel (even groupChannelI), row 1 (R),
  // starting at the row-1 offset nOutputCount, takes each group's right
  // channel.
  const unsigned nOutputCount = nAudioGroups * 2;
  std::vector<float> gains(nOutputCount * 2, GOAudioDeviceConfig::MUTE_VOLUME);

  for (unsigned groupI = 0; groupI < nAudioGroups; groupI++) {
    gains[groupI * 2] = 0.0f;
    gains[nOutputCount + groupI * 2 + 1] = 0.0f;
  }
  return gains;
}

float GOSoundOrganEngine::convertGainToScaleFactor(float gain) {
  return gain >= -120 && gain < 40 ? powf(10.0f, gain * 0.05f) : 0.0f;
}

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
  const std::vector<float> gains = createDownmixGains(nAudioGroups);
  const unsigned nOutputCount = nAudioGroups * 2;
  AudioOutputConfig config;

  config.channels = 2;
  config.scaleFactors.resize(2);
  for (unsigned channelI = 0; channelI < 2; channelI++)
    config.scaleFactors[channelI].assign(
      gains.begin() + channelI * nOutputCount,
      gains.begin() + (channelI + 1) * nOutputCount);
  return {config};
}

/*
 * Constructors and destructors
 */

GOSoundOrganEngine::OutputState::OutputState()
  : condition(mutex), isFinishedCurrPeriod(false) {}

GOSoundOrganEngine::OutputState::OutputState(OutputState &&other) noexcept
  : mp_task(std::move(other.mp_task)),
    condition(mutex),
    isFinishedCurrPeriod(other.isFinishedCurrPeriod) {}

GOSoundOrganEngine::OutputState::~OutputState() = default;

GOSoundOrganEngine::GOSoundOrganEngine(
  GOOrganModel &organModel, GOMemoryPool &memoryPool)
  : r_OrganModel(organModel),
    r_MemoryPool(memoryPool),
    mp_TouchTask(std::make_unique<GOSoundTouchTask>(r_MemoryPool)),
    m_SamplerPlayer(
      m_WindchestGroupTaskGrid,
      mp_WindchestTasks,
      mp_TremulantTasks,
      mp_ReleaseTask),
    m_NAudioGroups(1),
    m_NAuxThreads(0),
    m_IsDownmix(false),
    m_NReleaseRepeats(1),
    m_ReverbConfig(GOSoundReverb::CONFIG_REVERB_DISABLED),
    m_NSamplesPerBuffer(1),
    m_LifecycleState(LifecycleState::IDLE),
    m_NCallbacksEnteredCurrPeriod(0),
    m_NCallbacksFinishedCurrPeriod(0) {
  SetGain(-15);
  mp_ReleaseTask
    = std::make_unique<GOSoundReleaseTask>(m_SamplerPlayer, mp_AudioGroupTasks);
}

// The destructor body is empty, but it must be defined here (not in the header)
// so that std::unique_ptr can call the complete destructors of its managed
// types (GOSoundReleaseTask, GOSoundTouchTask, GOSchedulerThread), which are
// only forward-declared in the header file.
GOSoundOrganEngine::~GOSoundOrganEngine() {}

/*
 * Configuration getters and setters
 */

void GOSoundOrganEngine::SetGain(int gain) {
  m_gain = gain;
  m_amplitude = powf(10.0f, m_gain * 0.05f);
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
  SetNBytesPerSoundItem(config.WaveFormatBytesPerSample());
}

/*
 * Lifecycle functions
 */

void GOSoundOrganEngine::BuildEngine(
  const std::vector<AudioOutputConfig> &audioOutputConfigs,
  unsigned nSamplesPerBuffer,
  unsigned sampleRate) {
  GOMutexLocker locker(m_LifecycleMutex);

  assert(m_LifecycleState.load() == LifecycleState::IDLE);

  // Fill out the start parameters
  m_NSamplesPerBuffer = nSamplesPerBuffer;

  // [B1] Build tremulant tasks
  for (unsigned n = r_OrganModel.GetTremulantCount(), tremI = 0; tremI < n;
       tremI++)
    mp_TremulantTasks.push_back(
      new GOSoundTremulantTask(m_SamplerPlayer, m_NSamplesPerBuffer));

  // [B2] Build windchest tasks
  // Special windchest task for detached releases (index 0 =
  // DETACHED_RELEASE_TASK_ID)
  mp_WindchestTasks.push_back(
    std::make_unique<GOSoundWindchestTask>(*this, nullptr));
  for (unsigned n = r_OrganModel.GetWindchestCount(), wcI = 0; wcI < n; wcI++)
    mp_WindchestTasks.push_back(std::make_unique<GOSoundWindchestTask>(
      *this, r_OrganModel.GetWindchest(wcI)));

  // [B3] Initialize windchests with tremulant tasks
  for (auto &pWcTask : mp_WindchestTasks)
    pWcTask->Init(mp_TremulantTasks);

  // [B4] Build the windchest-group task grid
  const unsigned nWindchests = r_OrganModel.GetWindchestCount() + 1;

  m_WindchestGroupTaskGrid.Resize(nWindchests, m_NAudioGroups);
  {
    // main cells: GetUsedWindchestGroupPairs() is a std::set, so each pair -
    // and so each cell - is visited at most once here
    std::set<unsigned> usedAudioGroupIds;

    for (auto &pair : r_OrganModel.GetUsedWindchestGroupPairs()) {
      m_WindchestGroupTaskGrid.BuildWindchestGroupTask(
        pair.first,
        pair.second,
        m_scheduler.GetRoundCounter(),
        m_SamplerPlayer,
        GetWindchestTaskAt(pair.first),
        m_NSamplesPerBuffer);
      usedAudioGroupIds.insert(pair.second);
    }

    // detached-release row: one task per audio group actually used, not per
    // pair - several windchests may feed the same group, but its release
    // cell must only be built once, see
    // GOSoundWindchestGroupTaskGrid::BuildWindchestGroupTask()
    for (unsigned audioGroupId : usedAudioGroupIds)
      m_WindchestGroupTaskGrid.BuildWindchestGroupTask(
        0,
        audioGroupId,
        m_scheduler.GetRoundCounter(),
        m_SamplerPlayer,
        GetWindchestTaskAt(0),
        m_NSamplesPerBuffer);
  }

  // [B5] Build audio group tasks
  std::vector<GOSoundBufferTaskBase *> groupOutputs;

  for (unsigned groupI = 0; groupI < m_NAudioGroups; groupI++) {
    GOSoundGroupTask *pGroupTask = new GOSoundGroupTask(m_NSamplesPerBuffer);

    pGroupTask->SetInputs(
      m_WindchestGroupTaskGrid.GetInputsForGroup(groupI, nWindchests));
    mp_AudioGroupTasks.push_back(pGroupTask);
    groupOutputs.push_back(pGroupTask);
  }

  // [B6] Build audio output states (per-device output task + callback sync)
  unsigned nTotalChannels = 0;

  m_OutputStates.resize(audioOutputConfigs.size());
  for (unsigned deviceI = 0; deviceI < audioOutputConfigs.size(); deviceI++) {
    OutputState &outputState = m_OutputStates[deviceI];
    const AudioOutputConfig &devConfig = audioOutputConfigs[deviceI];
    const unsigned nChannels = devConfig.channels;
    std::vector<float> scaleFactors;

    scaleFactors.resize(m_NAudioGroups * nChannels * 2);
    std::fill(scaleFactors.begin(), scaleFactors.end(), 0.0f);
    for (unsigned channelI = 0; channelI < nChannels; channelI++) {
      for (unsigned k = 0; k < devConfig.scaleFactors[channelI].size(); k++) {
        if (k >= m_NAudioGroups * 2)
          break;
        scaleFactors[channelI * m_NAudioGroups * 2 + k]
          = convertGainToScaleFactor(devConfig.scaleFactors[channelI][k]);
      }
    }
    outputState.mp_task = std::make_unique<GOSoundOutputTask>(
      nChannels, scaleFactors, m_NSamplesPerBuffer);
    outputState.mp_task->SetOutputs(groupOutputs);
    nTotalChannels += nChannels;
  }

  // [B7] Resize meter info to match real output channels.
  // std::atomic is not copyable/movable, so we construct a fresh vector and
  // swap instead of resize.
  {
    std::vector<std::atomic<float>> newMeterInfo(nTotalChannels);

    m_MeterInfo.swap(newMeterInfo);
  }

  // [B8] Build downmix task (optional stereo mix for recorder)
  if (m_IsDownmix) {
    const std::vector<float> gains = createDownmixGains(m_NAudioGroups);
    std::vector<float> scaleFactors(gains.size());

    for (unsigned i = 0, n = gains.size(); i < n; i++)
      scaleFactors[i] = convertGainToScaleFactor(gains[i]);

    mp_DownmixTask = std::make_unique<GOSoundOutputTask>(
      2, scaleFactors, m_NSamplesPerBuffer);
    mp_DownmixTask->SetOutputs(groupOutputs);
  }

  // [B9] Set up recorder outputs
  {
    std::vector<GOSoundBufferTaskBase *> recorderOutputs;

    if (mp_DownmixTask)
      recorderOutputs.push_back(mp_DownmixTask.get());
    else
      for (OutputState &state : m_OutputStates)
        recorderOutputs.push_back(state.mp_task.get());
    m_RecorderTask.SetSampleRate(sampleRate);
    m_RecorderTask.SetOutputs(recorderOutputs, m_NSamplesPerBuffer);
  }

  // [B10] Set up reverb
  if (mp_DownmixTask)
    mp_DownmixTask->SetupReverb(
      m_ReverbConfig, m_NSamplesPerBuffer, sampleRate);
  for (OutputState &state : m_OutputStates)
    state.mp_task->SetupReverb(m_ReverbConfig, m_NSamplesPerBuffer, sampleRate);

  // [B11] Add all tasks to scheduler
  m_scheduler.Clear();
  m_scheduler.SetRepeatCount(m_NReleaseRepeats);
  for (GOSoundTremulantTask *pTremTask : mp_TremulantTasks)
    m_scheduler.Add(pTremTask);
  for (auto &pWcTask : mp_WindchestTasks)
    m_scheduler.Add(pWcTask.get());
  m_WindchestGroupTaskGrid.ForEachTask(
    [this](GOSoundWindchestGroupTask *pTask) { m_scheduler.Add(pTask); });
  for (GOSoundGroupTask *pGroupTask : mp_AudioGroupTasks)
    m_scheduler.Add(pGroupTask);
  if (mp_DownmixTask)
    m_scheduler.Add(mp_DownmixTask.get());
  for (OutputState &state : m_OutputStates)
    m_scheduler.Add(state.mp_task.get());
  m_scheduler.Add(&m_RecorderTask);
  m_scheduler.Add(mp_ReleaseTask.get());
  m_scheduler.Add(mp_TouchTask.get());

  // [B12] Build worker threads
  for (unsigned threadI = 0; threadI < m_NAuxThreads; threadI++)
    mp_threads.push_back(std::make_unique<GOSchedulerThread>(&m_scheduler));
  for (auto &pThread : mp_threads)
    pThread->Run();

  m_SamplerPlayer.Build(sampleRate);
  m_SamplerPlayer.Reset();
  m_LifecycleState.store(LifecycleState::BUILT);
}

void GOSoundOrganEngine::DestroyEngine() {
  GOMutexLocker locker(m_LifecycleMutex);

  assert(m_LifecycleState.load() == LifecycleState::BUILT);

  // [B12] Destroy worker threads
  for (auto &pThread : mp_threads)
    pThread->Delete();
  mp_threads.clear();

  // [B11] Clear scheduler
  m_scheduler.Clear();

  // [B10] Reverb — no explicit cleanup (owned by output tasks below)
  // [B9] Recorder outputs — no explicit cleanup (recorder is non-owning)

  // [B8] Destroy downmix task
  mp_DownmixTask.reset();

  // [B7] Clear meter info
  m_MeterInfo.clear();

  // [B6] Destroy audio output states
  m_OutputStates.clear();

  // [B5] Destroy audio group tasks: unwire before the grid (built at [B4])
  // is destroyed, so no live task is left holding a dangling pointer into it
  for (GOSoundGroupTask *pGroupTask : mp_AudioGroupTasks) {
    pGroupTask->SetInputs({});
    pGroupTask->DiscardContent();
  }
  mp_AudioGroupTasks.clear();

  // [B4] Destroy the windchest-group task grid. Every GOSoundProcessorState
  // owned by a grid cell's chain state must be destroyed before the
  // GOSoundProcessor it was created from (owned by the windchest tasks,
  // destroyed below at [B2]) — this is exactly why windchest/tremulant
  // construction was moved ahead of the grid at [B1]-[B3]: as the mirror of
  // that build order, this step runs before [B2] destroys the windchest
  // tasks, automatically.
  m_WindchestGroupTaskGrid.ForEachTask(
    [](GOSoundWindchestGroupTask *pTask) { pTask->WaitAndDiscardContent(); });
  m_WindchestGroupTaskGrid.Clear();

  // [B3] Init() — nothing to explicitly undo; its connections drop with the
  // windchest tasks at [B2] below

  // [B2] Destroy windchest tasks
  mp_WindchestTasks.clear();

  // [B1] Destroy tremulant tasks
  mp_TremulantTasks.clear();

  m_SamplerPlayer.Destroy();
  m_LifecycleState.store(LifecycleState::IDLE);
}

void GOSoundOrganEngine::StartEngine() {
  assert(m_LifecycleState.load() == LifecycleState::BUILT);
  m_scheduler.NewRound();
  m_scheduler.ResumeGivingWork();

  // Mirrors the end-of-period wakeup in ProcessAudioCallback(): aux worker
  // threads are parked (WaitForIdle()'d) by StopEngine() and only ever woken
  // there or at the end of a period. Without this, the round just made
  // available above sits unclaimed by any aux thread until the first
  // post-resume period happens to complete one on its own - so the entire
  // first period after any Stop/Start cycle would run synchronously on the
  // audio callback thread alone.
  for (auto &pThread : mp_threads)
    pThread->Wakeup();

  m_LifecycleState.store(LifecycleState::WORKING);
}

void GOSoundOrganEngine::StopEngine() {
  assert(m_LifecycleState.load() == LifecycleState::WORKING);

  m_scheduler.PauseGivingWork();
  for (auto &pThread : mp_threads)
    pThread->WaitForIdle();
  m_LifecycleState.store(LifecycleState::BUILT);
}

GOSoundOrganEngine::AudioGroupRoutingChange GOSoundOrganEngine::
  PrepareSoundRoutingFor(const std::set<std::pair<unsigned, unsigned>> &pairs) {
  AudioGroupRoutingChange change;
  std::set<unsigned> newAudioGroupIds;

  // main cells: pairs is already unique by (windchestN, audioGroupId), so
  // each cell is visited at most once in this loop
  for (const auto &[windchestN, audioGroupId] : pairs)
    if (!m_WindchestGroupTaskGrid.HasWindchestGroupTask(
          windchestN, audioGroupId)) {
      change.newTasks.push_back(
        m_WindchestGroupTaskGrid.BuildWindchestGroupTask(
          windchestN,
          audioGroupId,
          m_scheduler.GetRoundCounter(),
          m_SamplerPlayer,
          GetWindchestTaskAt(windchestN),
          m_NSamplesPerBuffer));
      newAudioGroupIds.insert(audioGroupId);
    }

  // detached-release row: only for groups that just gained a new main cell -
  // a group with no new main task keeps whatever release routing it already
  // had, nothing to check or rebuild for it. Has() still gates the build: a
  // group can be "new" here on windchest X while its release row already
  // exists from windchest Y's earlier routing.
  for (unsigned audioGroupId : newAudioGroupIds)
    if (!m_WindchestGroupTaskGrid.HasWindchestGroupTask(0, audioGroupId))
      change.newTasks.push_back(
        m_WindchestGroupTaskGrid.BuildWindchestGroupTask(
          0,
          audioGroupId,
          m_scheduler.GetRoundCounter(),
          m_SamplerPlayer,
          GetWindchestTaskAt(0),
          m_NSamplesPerBuffer));

  // precompute the input list for every group that gained a cell above, so
  // CommitSoundRoutingFor() only has to install it, not scan the grid while
  // the engine is quiesced
  for (unsigned audioGroupId : newAudioGroupIds)
    change.groupInputs[audioGroupId]
      = m_WindchestGroupTaskGrid.GetInputsForGroup(
        audioGroupId, mp_WindchestTasks.size());

  return change;
}

void GOSoundOrganEngine::CommitSoundRoutingFor(
  AudioGroupRoutingChange &&change) {
  for (GOSoundWindchestGroupTask *pTask : change.newTasks)
    m_scheduler.Add(pTask);
  for (auto &[audioGroupId, inputs] : change.groupInputs)
    mp_AudioGroupTasks[audioGroupId]->SetInputs(std::move(inputs));
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

void GOSoundOrganEngine::SetStreaming(bool isActive) {
  // Load first so the assert catches bad transitions before the exchange.
  LifecycleState oldState = m_LifecycleState.load();

  assert(
    oldState >= LifecycleState::USED && oldState <= LifecycleState::STREAMING);

  const LifecycleState newState
    = isActive ? LifecycleState::STREAMING : LifecycleState::USED;

  // Atomically transition; re-read the actual previous state from exchange
  // so that the side effects below are based on the real transition.
  oldState = m_LifecycleState.exchange(newState);

  if (newState != oldState) {
    if (isActive) {
      // USED → STREAMING: reset period counters and per-output wait flags so
      // the first callback of each output in the new streaming session is not
      // blocked at [W1]. Counters must be reset here (not just in the
      // constructor) because the engine may be reconnected without a full
      // rebuild, leaving counters dirty from the previous streaming session.
      m_NCallbacksEnteredCurrPeriod.store(0);
      m_NCallbacksFinishedCurrPeriod.store(0);
      for (OutputState &state : m_OutputStates) {
        GOMutexLocker locker(state.mutex);

        state.isFinishedCurrPeriod = false;
      }
    } else {
      // STREAMING → USED: unblock any callbacks waiting at [W1] so they can
      // check IsStreaming() and exit gracefully.
      for (OutputState &state : m_OutputStates) {
        GOMutexLocker locker(state.mutex);

        state.condition.Broadcast();
      }
    }
  }
}

/*
 * Functions called from GOSoundSystem
 */

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
  m_scheduler.CompleteRound();

  // AdvanceTime advances m_CurrentTime and records peak used polyphony
  // (both previously done inline here; now delegated to GOSoundSamplerPlayer).
  m_SamplerPlayer.AdvanceTime(m_NSamplesPerBuffer);

  // Accumulate per-channel peak levels from each output task into m_MeterInfo
  // for the GUI meter display. Values accumulate between GUI polls;
  // GetMeterInfo() resets them via exchange(0). Only real device outputs
  // (m_OutputStates tasks) are counted; mp_DownmixTask is excluded.
  // Guarded by assert(IsWorking()) above: m_MeterInfo is valid in WORKING
  // state.
  const auto meterEnd = m_MeterInfo.end();
  auto meterIt = m_MeterInfo.begin();

  for (auto &state : m_OutputStates) {
    for (const float f : state.mp_task->GetMeterInfo()) {
      // m_MeterInfo.size() == nTotalChannels [B7], accumulated while
      // building m_OutputStates [B6], so the iterator never overflows.
      assert(meterIt < meterEnd);
      atomic_fetch_max_relaxed(*meterIt++, f);
    }
    state.mp_task->ResetMeterInfo();
  }

  m_scheduler.NewRound();
}

bool GOSoundOrganEngine::ProcessAudioCallback(
  unsigned outputIndex, GOSoundBufferMutable &outBuffer) {
  assert(IsWorking());

  const unsigned nOutputs = m_OutputStates.size();

  assert(outputIndex < nOutputs);

  bool isNewPeriod = false;
  OutputState &state = m_OutputStates[outputIndex];

  // Only one callback for this output may hold this mutex at a time.
  GOMutexLocker locker(state.mutex);

  // [W1] Wait until this output has not yet been processed in the current
  // period. Exits immediately if the engine leaves STREAMING (e.g.
  // SetStreaming(false) was called during disconnect).
  while (IsStreaming() && state.isFinishedCurrPeriod)
    state.condition.Wait();

  if (IsStreaming()) {
    /*
     * The main callback critical section. Only one callback per output may
     * enter here, and only once per period.
     */

    // Number of callbacks that have entered the critical section this period.
    unsigned nEntered = ++m_NCallbacksEnteredCurrPeriod; // atomic
    bool isLastEntered = nEntered >= nOutputs;

    // Finish computing the output task and copy the result into the buffer.
    GOSoundOutputTask &outputTask = *m_OutputStates[outputIndex].mp_task;

    outputTask.EnsureBufferReady(isLastEntered);
    outBuffer.CopyFrom(outputTask);

    // Mark this output as done for the current period so that future callbacks
    // for this output will block at [W1] until the period advances.
    state.isFinishedCurrPeriod = true;

    unsigned nFinished = ++m_NCallbacksFinishedCurrPeriod; // atomic
    bool isLastFinished = nFinished >= nOutputs;

    // The last output to enter may not be the last to finish.
    if (isLastFinished) {
      // Advance to the next period.
      NextPeriod();

      // Wake up worker threads to start processing the new period.
      for (auto &pThread : mp_threads)
        pThread->Wakeup();

      // Reset per-period counters.
      m_NCallbacksEnteredCurrPeriod.store(0);
      m_NCallbacksFinishedCurrPeriod.store(0);

      // Mark all outputs as not yet processed for the new period and wake up
      // callbacks waiting at [W1]. Each output's mutex must be held when
      // writing isFinishedCurrPeriod, because another thread may be
      // reading it at [W1] under that mutex.
      for (OutputState &otherState : m_OutputStates) {
        // The current output's mutex is already held (locker above), so
        // try_lock=true prevents re-locking and deadlocking; all other outputs
        // are locked unconditionally (try_lock=false).
        GOMutexLocker otherLocker(otherState.mutex, &otherState == &state);

        otherState.isFinishedCurrPeriod = false;
        otherState.condition.Signal();
      }
      isNewPeriod = true;
    }
  } else
    // SetStreaming(false) unblocked [W1]; engine is no longer STREAMING.
    outBuffer.FillWithSilence();

  return isNewPeriod;
}

/*
 * Other functions
 */

std::vector<float> GOSoundOrganEngine::GetMeterInfo() {
  /* GUI thread: m_LifecycleMutex prevents concurrent BuildEngine/DestroyEngine
     from modifying m_MeterInfo while we read it.
     The lock is taken with try_lock because the GUI thread may re-enter this
     method while it already holds m_LifecycleMutex: BuildEngine() and
     DestroyEngine() hold the mutex for their whole duration, and any wxLog*
     call made from inside them reaches GOLogWindow::LogMsg, which re-enters
     the event loop via wxApp::Yield() and dispatches the pending wxEVT_METERS.
     m_LifecycleMutex is not recursive, so a blocking lock would deadlock the
     application (issue #2606).
     When the mutex is occupied, an empty result is returned and
     GOAppWindow::OnMeters skips this meter frame. Nothing is lost: the audio
     thread keeps accumulating peaks in m_MeterInfo until the next poll. */
  GOMutexLocker locker(
    m_LifecycleMutex, true, "GOSoundOrganEngine::GetMeterInfo");
  std::vector<float> result;

  if (locker.IsLocked()) {
    // result[0] = polyphony ratio; result[1..] = per-channel peak levels.
    // When not working, m_MeterInfo may be empty; result contains only zeros.
    result.assign(m_MeterInfo.size() + 1, 0.0f);

    if (IsWorking()) {
      const unsigned hardPolyphony = GetHardPolyphony();
      float *pResult = result.data();

      assert(hardPolyphony > 0);
      // GetAndResetUsedPolyphony() reads accumulated peak polyphony and resets
      // it
      *(pResult++) = m_SamplerPlayer.GetAndResetUsedPolyphony()
        / static_cast<float>(hardPolyphony);
      for (std::atomic<float> &v : m_MeterInfo)
        *(pResult++) = v.exchange(0.0f);
    }
  }
  return result;
}
