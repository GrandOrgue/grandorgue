/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDORGANENGINE_H
#define GOSOUNDORGANENGINE_H

#include <atomic>
#include <memory>
#include <vector>

#include "playing/GOSoundSamplerPlayer.h"
#include "reverb/GOSoundReverb.h"
#include "scheduler/GOScheduler.h"
#include "tasks/GOSoundRecorderTask.h"
#include "threading/GOCondition.h"
#include "threading/GOMutex.h"

class GOConfig;
class GOMemoryPool;
class GOOrganModel;
class GOSchedulerTask;
class GOSchedulerThread;
class GOSoundBufferMutable;
class GOSoundGroupTask;
class GOSoundOutputTask;
class GOSoundReleaseTask;
class GOSoundTouchTask;
class GOSoundTremulantTask;
class GOSoundWindchestTask;
class GOWindchest;

/**
 * @brief Sound engine for one loaded organ.
 *
 * Lifecycle (steps 3-4 are repeatable, e.g. to pause and resume playback
 * without losing sounding notes):
 *
 *   1. Constructor: GOSoundOrganEngine(organModel, memoryPool)
 *   2. Configuration: SetFromConfig(config) or manual setters
 *   3. BuildEngine(audioOutputConfigs, nSamplesPerBuffer, sampleRate) —
 *      builds tasks and the sampler pool. IDLE → BUILT.
 *   4. StartEngine() — starts dispatching work to worker threads. BUILT →
 *      WORKING.
 *      ... ProcessAudioCallback() is called from the audio thread ...
 *   5. StopEngine() — halts work dispatch without touching any task's
 *      content, so a subsequent StartEngine() resumes rather than restarts.
 *      WORKING → BUILT. Repeat from step 4 to resume.
 *   6. DestroyEngine() — destroys tasks and reclaims the sampler pool. Only
 *      legal from BUILT (i.e. after StopEngine()). BUILT → IDLE.
 */
class GOSoundOrganEngine {
public:
  /*
   * Nested type
   */

  /**
   * @brief Configuration for one audio output device.
   *
   * scaleFactors[ch][groupI*2+ch] = 0.0f means direct output of group groupI
   * into channel ch. Other values = GOAudioDeviceConfig::MUTE_VOLUME (-121.0f).
   */
  struct AudioOutputConfig {
    unsigned channels;
    std::vector<std::vector<float>> scaleFactors;
  };

  /*
   * Factory functions
   */

  /**
   * @brief Creates output configurations from GOConfig.
   */
  static std::vector<AudioOutputConfig> createAudioOutputConfigs(
    GOConfig &config, unsigned nAudioGroups);

  /**
   * @brief Creates a single stereo output for nAudioGroups groups.
   *
   * For each group i:
   *   scaleFactors[0][i*2] = 0.0f (L),
   *   scaleFactors[0][i*2+1] = MUTE_VOLUME (-121.0f) (R)
   *   scaleFactors[1][i*2] = MUTE_VOLUME (-121.0f) (L)
   *   scaleFactors[1][i*2+1] = 0.0f (R)
   */
  static std::vector<AudioOutputConfig> createDefaultOutputConfigs(
    unsigned nAudioGroups = 1);

private:
  /*
   * Private nested types
   */

  /**
   * @brief Per-output state: owns the output task and carries the callback
   * synchronization primitives shared between concurrent audio callbacks.
   */
  struct OutputState {
    std::unique_ptr<GOSoundOutputTask> mp_task;
    GOMutex mutex;
    GOCondition condition;

    /** true if this output has already been processed in the current period.
     * The next audio callback for this output will block at [W1] until
     * the period advances and this flag is reset to false. */
    bool isFinishedCurrPeriod;

    OutputState();
    // Needed to allow std::vector<OutputState>::resize(): GOMutex and
    // GOCondition are not moveable, so condition must be re-constructed
    // referencing the new object's mutex rather than the old one.
    OutputState(OutputState &&other) noexcept;
    ~OutputState();
  };

  static constexpr int DETACHED_RELEASE_TASK_ID = 0;

  /*
   * Constructor constants: objects that live for the entire instance lifetime
   */

  GOOrganModel &r_OrganModel;
  GOMemoryPool &r_MemoryPool;
  // mp_ReleaseTask references mp_AudioGroupTasks [B1]; created in constructor
  // body (after m_SamplerPlayer), added to m_scheduler in BuildEngine [B8]
  std::unique_ptr<GOSoundReleaseTask> mp_ReleaseTask;
  // mp_TouchTask references r_MemoryPool; created in constructor,
  // added to m_scheduler in BuildEngine [B8]
  std::unique_ptr<GOSoundTouchTask> mp_TouchTask;
  GOSoundRecorderTask m_RecorderTask;
  // m_SamplerPlayer is declared after mp_ReleaseTask so that mp_ReleaseTask
  // is already a valid (though null) unique_ptr when passed by reference to
  // the player constructor.
  GOSoundSamplerPlayer m_SamplerPlayer;

  /*
   * Configuration parameters
   */

  unsigned m_NAudioGroups;
  unsigned m_NAuxThreads;
  bool m_IsDownmix;
  unsigned m_NReleaseRepeats;
  int m_gain;
  float m_amplitude;
  GOSoundReverb::ReverbConfig m_ReverbConfig;

  /*
   * Start parameters (set from BuildEngine arguments)
   */

  unsigned m_NSamplesPerBuffer;

  /*
   * Lifecycle state
   */

  enum class LifecycleState { IDLE, BUILT, WORKING, USED, STREAMING };

  // Protects the IDLE↔BUILT transition (BuildEngine / DestroyEngine).
  // Any function that must guarantee the lifecycle state does not change
  // during its execution (e.g. GetMeterInfo) must acquire this mutex before
  // reading m_LifecycleState.
  GOMutex m_LifecycleMutex;

  std::atomic<LifecycleState> m_LifecycleState;

  /*
   * Tasks built in BuildEngine (in build order [B1]-[B11])
   */

  // [B1] mp_AudioGroupTasks: created per audio group (m_NAudioGroups entries)
  //   — referenced by: mp_ReleaseTask (constructor), m_OutputStates [B2]
  ptr_vector<GOSoundGroupTask> mp_AudioGroupTasks;
  // [B2] m_OutputStates: created from audioOutputConfigs (per-device
  // tasks + callback sync state)
  //   — uses mp_AudioGroupTasks [B1] via SetOutputs()
  //   — referenced by: m_MeterInfo [B3], m_RecorderTask [B5], reverb [B6]
  std::vector<OutputState> m_OutputStates;
  // [B3] m_MeterInfo: per-channel peak levels for the meter display
  //   — uses nTotalChannels accumulated over m_OutputStates [B2]
  //   — audio thread: NextPeriod() writes via atomic_fetch_max_relaxed()
  //   — GUI thread: GetMeterInfo() reads and resets under m_LifecycleMutex
  std::vector<std::atomic<float>> m_MeterInfo;
  // [B4] mp_DownmixTask: optional stereo downmix task (only when m_IsDownmix)
  //   — uses mp_AudioGroupTasks [B1] via SetOutputs()
  //   — referenced by: m_RecorderTask [B5], reverb setup [B6]
  std::unique_ptr<GOSoundOutputTask> mp_DownmixTask;
  // [B5] recorder: set up sample rate and outputs on m_RecorderTask
  //   — uses mp_DownmixTask [B4] or m_OutputStates [B2]
  // [B6] reverb: set up inline on mp_DownmixTask [B4] and m_OutputStates [B2]
  //   — uses m_ReverbConfig, sampleRate (BuildEngine parameter),
  //     m_NSamplesPerBuffer
  //
  // [B7] mp_TremulantTasks: one per tremulant in r_OrganModel
  //   — referenced by mp_WindchestTasks after [B9] Init()
  ptr_vector<GOSoundTremulantTask> mp_TremulantTasks;
  // [B8] mp_WindchestTasks: one special + one per windchest in r_OrganModel
  //   — initialized in [B9] with mp_TremulantTasks [B7]
  std::vector<std::unique_ptr<GOSoundWindchestTask>> mp_WindchestTasks;
  // [B9] Init(): connects mp_WindchestTasks [B8] to mp_TremulantTasks [B7]
  //
  // [B10] m_scheduler: all tasks added; SetRepeatCount(m_NReleaseRepeats)
  //   — uses all tasks above + mp_ReleaseTask + mp_TouchTask (constructor)
  GOScheduler m_scheduler;
  // [B11] mp_threads: worker threads created via BuildThreads(m_NAuxThreads)
  //   — uses m_scheduler [B10]
  std::vector<std::unique_ptr<GOSchedulerThread>> mp_threads;

  /*
   * Per-period counters reset by SetStreaming(true) at each streaming session
   */

  /** Number of audio callbacks that have entered the processing critical
   * section in the current period. Incremented atomically at the start of
   * the critical section in ProcessAudioCallback(). Reset to zero at the
   * end of each period and at the start of each streaming session. */
  std::atomic_uint m_NCallbacksEnteredCurrPeriod;

  /** Number of audio callbacks that have finished processing in the current
   * period. Incremented atomically after the output buffer is filled in
   * ProcessAudioCallback(). When it reaches the total number of outputs,
   * the period advances and both counters are reset to zero. Reset at the
   * start of each streaming session. */
  std::atomic_uint m_NCallbacksFinishedCurrPeriod;

  /*
   * Private helpers for functions called from GOSoundSystem
   */

  void NextPeriod();

public:
  /*
   * Constructors and destructors
   */

  GOSoundOrganEngine(GOOrganModel &organModel, GOMemoryPool &memoryPool);
  // Defined in the .cpp file because the destructor of std::unique_ptr requires
  // the complete type of the managed object, which is only forward-declared
  // here.
  ~GOSoundOrganEngine();

  /*
   * Sampler player accessor
   */

  /** Returns a reference to the sampler player (used to connect the organ
   * model via GOSoundSamplerPlayerProxy). */
  GOSoundSamplerPlayer &GetSamplerPlayer() { return m_SamplerPlayer; }

  /** @return GOSoundSamplerPlayer::UsedSamplerCount() — see there. */
  unsigned GetUsedSamplerCount() const {
    return m_SamplerPlayer.UsedSamplerCount();
  }

  /*
   * Configuration getters and setters
   */

  unsigned GetNAudioGroups() const { return m_NAudioGroups; }
  void SetNAudioGroups(unsigned nAudioGroups) { m_NAudioGroups = nAudioGroups; }

  unsigned GetNAuxThreads() const { return m_NAuxThreads; }
  void SetNAuxThreads(unsigned nAuxThreads) { m_NAuxThreads = nAuxThreads; }

  bool IsDownmix() const { return m_IsDownmix; }
  void SetDownmix(bool isDownmix) { m_IsDownmix = isDownmix; }

  unsigned GetNReleaseRepeats() const { return m_NReleaseRepeats; }
  void SetNReleaseRepeats(unsigned nReleaseRepeats) {
    m_NReleaseRepeats = nReleaseRepeats;
  }

  bool IsReleaseAlignmentEnabled() const {
    return m_SamplerPlayer.IsReleaseAlignmentEnabled();
  }
  void SetReleaseAlignmentEnabled(bool isEnabled) {
    m_SamplerPlayer.SetReleaseAlignmentEnabled(isEnabled);
  }

  const GOSoundReverb::ReverbConfig &GetReverbConfig() const {
    return m_ReverbConfig;
  }
  void SetReverbConfig(const GOSoundReverb::ReverbConfig &config) {
    m_ReverbConfig = config;
  }

  float GetAmplitude() const { return m_amplitude; }

  int GetGain() const { return m_gain; }
  void SetGain(int gain);

  unsigned GetHardPolyphony() const {
    return m_SamplerPlayer.GetHardPolyphony();
  }
  void SetHardPolyphony(unsigned polyphony) {
    m_SamplerPlayer.SetHardPolyphony(polyphony);
  }

  bool IsPolyphonyLimiting() const {
    return m_SamplerPlayer.IsPolyphonyLimiting();
  }
  void SetPolyphonyLimiting(bool isLimiting) {
    m_SamplerPlayer.SetPolyphonyLimiting(isLimiting);
  }

  bool IsScaledReleases() const { return m_SamplerPlayer.IsScaledReleases(); }
  void SetScaledReleases(bool isEnabled) {
    m_SamplerPlayer.SetScaledReleases(isEnabled);
  }

  bool IsRandomizeSpeaking() const {
    return m_SamplerPlayer.IsRandomizeSpeaking();
  }
  void SetRandomizeSpeaking(bool isEnabled) {
    m_SamplerPlayer.SetRandomizeSpeaking(isEnabled);
  }

  GOSoundResample::InterpolationType GetInterpolationType() const {
    return m_SamplerPlayer.GetInterpolationType();
  }
  void SetInterpolationType(unsigned type) {
    m_SamplerPlayer.SetInterpolationType(type);
  }

  unsigned GetNBytesPerSoundItem() const {
    return m_RecorderTask.GetBytesPerSample();
  }
  void SetNBytesPerSoundItem(unsigned nBytes) {
    m_RecorderTask.SetBytesPerSample(nBytes);
  }
  GOSoundRecorderTask &GetRecorderTask() { return m_RecorderTask; }

  /** Reads parameters from GOConfig and stores them via setters. */
  void SetFromConfig(GOConfig &config);

  /*
   * Start parameter getters (values come via BuildEngine)
   */

  unsigned GetSampleRate() const { return m_SamplerPlayer.GetSampleRate(); }
  unsigned GetNSamplesPerBuffer() const { return m_NSamplesPerBuffer; }

  /*
   * Other getters
   */

  uint64_t GetTime() const { return m_SamplerPlayer.GetTime(); }
  std::vector<float> GetMeterInfo();
  GOScheduler &GetScheduler() { return m_scheduler; }

  /*
   * Lifecycle state
   */

  /** true if the engine is in the initial state (before BuildEngine or after
   * DestroyEngine). */
  bool IsIdle() const {
    return m_LifecycleState.load() == LifecycleState::IDLE;
  }

  /** true if the engine is running (WORKING or USED). */
  bool IsWorking() const {
    return m_LifecycleState.load() >= LifecycleState::WORKING;
  }

  /** true if the engine is connected to the audio system (USED or STREAMING).
   */
  bool IsUsed() const {
    return m_LifecycleState.load() >= LifecycleState::USED;
  }

  /** true if audio callbacks are flowing (STREAMING). */
  bool IsStreaming() const {
    return m_LifecycleState.load() >= LifecycleState::STREAMING;
  }

  /** Switches between WORKING and USED; called from GOSoundSystem. */
  void SetUsed(bool isUsed);

  /** Switches between USED and STREAMING; called from GOSoundSystem.
   *  SetStreaming(false) broadcasts all output conditions to unblock any
   *  callbacks waiting at [W1]. */
  void SetStreaming(bool isActive);

  /*
   * Public lifecycle functions
   */

  /**
   * @brief Builds every sound task from the current configuration and
   * constructs the sampler pool for this session.
   *
   * Call after SetFromConfig() or manual setters. IDLE → BUILT.
   * @param audioOutputConfigs  Output configurations; must not be empty.
   * @param nSamplesPerBuffer   Buffer size in samples (from audio system).
   * @param sampleRate          Sample rate in Hz (from audio system).
   */
  void BuildEngine(
    const std::vector<AudioOutputConfig> &audioOutputConfigs,
    unsigned nSamplesPerBuffer,
    unsigned sampleRate);

  /**
   * @brief Destroys every task built by BuildEngine() and reclaims the
   * sampler pool.
   *
   * Requires the engine to be quiesced first (BUILT, i.e. after
   * StopEngine()) — this is not a pause, sounding notes do not survive it.
   * BUILT → IDLE.
   */
  void DestroyEngine();

  /**
   * @brief Starts a new round and resumes dispatching work to worker
   * threads.
   *
   * Safe to call after BuildEngine() (fresh start) or after StopEngine()
   * (resume: task content — active/releasing samplers — was left untouched
   * by StopEngine(), so sounding notes continue). BUILT → WORKING.
   */
  void StartEngine();

  /**
   * @brief Drains in-flight processing and halts work dispatch, without
   * touching any task's content.
   *
   * A subsequent StartEngine() resumes rather than restarts. WORKING →
   * BUILT.
   */
  void StopEngine();

  /*
   * Functions called from GOSoundSystem
   */

  /**
   * @brief Fills one output buffer and, when all outputs have been filled,
   * advances to the next period.
   *
   * Several callbacks may be called cuncurrently, but only one callback per
   * output may finish in one period. All other callbacks will wait for thr next
   * periods.
   *
   * @param outputIndex  Zero-based index of the audio output device.
   * @param outBuffer    Buffer to fill with audio data for this device.
   * @return true if all outputs have been processed and a new period has been
   * started (NextPeriod was invoked and worker threads were woken up).
   */
  bool ProcessAudioCallback(
    unsigned outputIndex, GOSoundBufferMutable &outBuffer);
};

#endif /* GOSOUNDORGANENGINE_H */
