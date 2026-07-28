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

#include "threading/GOMutex.h"

#include "playing/GOSoundResample.h"
#include "playing/GOSoundSampler.h"
#include "playing/GOSoundSamplerPool.h"
#include "reverb/GOSoundReverb.h"
#include "scheduler/GOSoundScheduler.h"

#include "GOSoundOrganInterface.h"

class GOConfig;
class GOMemoryPool;
class GOOrganModel;
class GOSoundBufferMutable;
class GOSoundGroupTask;
class GOSoundOutputTask;
class GOSoundProvider;
class GOSoundRecorder;
class GOSoundReleaseTask;
class GOSoundTask;
class GOSoundThread;
class GOSoundTouchTask;
class GOSoundTremulantTask;
class GOSoundWindchestTask;
class GOWindchest;

/**
 * @brief Sound engine for one loaded organ.
 *
 * Lifecycle (steps 3-4 are repeatable for restart with new parameters):
 *
 *   1. Constructor: GOSoundOrganEngine(organModel, memoryPool)
 *   2. Configuration: SetFromConfig(config) or manual setters
 *   3. BuildAndStart(audioOutputConfigs, nSamplesPerBuffer, sampleRate,
 * recorder) — builds tasks and starts the engine
 *      ... GetAudioOutput() is called from the audio thread ...
 *   4. StopAndDestroy() — stops the engine and destroys tasks
 */
class GOSoundOrganEngine : public GOSoundOrganInterface {
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
  static constexpr int DETACHED_RELEASE_TASK_ID = 0;

  /*
   * Constructor constants: objects that live for the entire instance lifetime
   */

  GOOrganModel &r_OrganModel;
  GOMemoryPool &r_MemoryPool;
  // mp_ReleaseTask references mp_AudioGroupTasks [B1]; created in constructor,
  // added to m_Scheduler in BuildEngine [B8]
  std::unique_ptr<GOSoundReleaseTask> mp_ReleaseTask;
  // mp_TouchTask references r_MemoryPool; created in constructor,
  // added to m_Scheduler in BuildEngine [B8]
  std::unique_ptr<GOSoundTouchTask> mp_TouchTask;
  GOSoundResample m_resample;

  /*
   * Configuration parameters
   */

  unsigned m_NAudioGroups;
  unsigned m_NAuxThreads;
  bool m_IsDownmix;
  unsigned m_NReleaseRepeats;
  bool m_IsPolyphonyLimiting;
  unsigned m_PolyphonySoftLimit;
  bool m_IsScaledReleases;
  bool m_IsReleaseAlignmentEnabled;
  bool m_IsRandomizeSpeaking;
  // TODO: rename to m_gain (stores gain in dB; in GrandOrgue "gain" means dB)
  int m_volume;
  // TODO: rename to m_amplitude (stores the linear amplitude coefficient
  // derived
  //       from m_volume; in GrandOrgue "amplitude" means a linear coefficient)
  float m_gain;
  GOSoundResample::InterpolationType m_InterpolationType;
  GOSoundReverb::ReverbConfig m_ReverbConfig;

  /*
   * Start parameters (set from BuildAndStart arguments)
   */

  unsigned m_NSamplesPerBuffer;
  unsigned m_SampleRate;

  /*
   * Lifecycle state
   */

  enum class LifecycleState { IDLE, BUILT, WORKING, USED };

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
  //   — referenced by: mp_ReleaseTask (constructor), mp_AudioOutputTasks [B2]
  ptr_vector<GOSoundGroupTask> mp_AudioGroupTasks;
  // [B2] mp_AudioOutputTasks: created from audioOutputConfigs (per-device
  // tasks)
  //   — uses mp_AudioGroupTasks [B1] via SetOutputs()
  //   — referenced by: m_MeterInfo [B3], p_AudioRecorder [B5], reverb [B6]
  std::vector<std::unique_ptr<GOSoundOutputTask>> mp_AudioOutputTasks;
  // [B3] m_MeterInfo: per-channel peak levels for the meter display
  //   — uses nTotalChannels accumulated over mp_AudioOutputTasks [B2]
  //   — audio thread: NextPeriod() writes via atomic_fetch_max_relaxed()
  //   — GUI thread: GetMeterInfo() reads and resets under m_LifecycleMutex
  std::vector<std::atomic<float>> m_MeterInfo;
  // [B4] mp_DownmixTask: optional stereo downmix task (only when m_IsDownmix)
  //   — uses mp_AudioGroupTasks [B1] via SetOutputs()
  //   — referenced by: p_AudioRecorder [B5], reverb setup [B6]
  std::unique_ptr<GOSoundOutputTask> mp_DownmixTask;
  // [B5] p_AudioRecorder: non-owning pointer to the recorder passed in
  //   — uses mp_DownmixTask [B4] or mp_AudioOutputTasks [B2]
  GOSoundRecorder *p_AudioRecorder;
  // [B6] reverb: set up inline on mp_DownmixTask [B4] and mp_AudioOutputTasks
  // [B2]
  //   — uses m_ReverbConfig, m_SampleRate, m_NSamplesPerBuffer
  //
  // [B7] mp_TremulantTasks: one per tremulant in r_OrganModel
  //   — referenced by mp_WindchestTasks after [B9] Init()
  ptr_vector<GOSoundTremulantTask> mp_TremulantTasks;
  // [B8] mp_WindchestTasks: one special + one per windchest in r_OrganModel
  //   — initialized in [B9] with mp_TremulantTasks [B7]
  std::vector<std::unique_ptr<GOSoundWindchestTask>> mp_WindchestTasks;
  // [B9] Init(): connects mp_WindchestTasks [B8] to mp_TremulantTasks [B7]
  //
  // [B10] m_Scheduler: all tasks added; SetRepeatCount(m_NReleaseRepeats)
  //   — uses all tasks above + mp_ReleaseTask + mp_TouchTask (constructor)
  GOSoundScheduler m_Scheduler;
  // [B11] mp_threads: worker threads created via BuildThreads(m_NAuxThreads)
  //   — uses m_Scheduler [B10]
  std::vector<std::unique_ptr<GOSoundThread>> mp_threads;

  /*
   * Counters
   */

  uint64_t m_CurrentTime;
  GOSoundSamplerPool m_SamplerPool;
  std::atomic_uint m_UsedPolyphony;

  /*
   * Private lifecycle functions (callee first)
   */

  void BuildEngine(
    const std::vector<AudioOutputConfig> &audioOutputConfigs,
    unsigned nSamplesPerBuffer,
    unsigned sampleRate,
    GOSoundRecorder &recorder);
  void DestroyEngine();

  void ResetCounters();

  void StartEngine();
  void StopEngine();

  /*
   * Other private functions
   */

  unsigned MsToSamples(unsigned ms) const { return m_SampleRate * ms / 1000; }

  unsigned SamplesDiffToMs(uint64_t fromSamples, uint64_t toSamples) const;

  inline static unsigned isWindchestTask(int taskId) { return taskId >= 0; }

  inline static unsigned windchestTaskToIndex(int taskId) {
    return (unsigned)taskId;
  }

  inline static unsigned tremulantTaskToIndex(int taskId) {
    return -taskId - 1;
  }

  void StartSampler(GOSoundSampler *sampler);

  GOSoundSampler *CreateTaskSample(
    const GOSoundProvider *soundProvider,
    int samplerTaskId,
    unsigned audioGroup,
    unsigned velocity,
    unsigned delay,
    uint64_t prevEventTime,
    bool isRelease,
    uint64_t *pStartTimeSamples);

  void CreateReleaseSampler(GOSoundSampler *sampler);

  /**
   * Creates a new sampler with decay of current loop.
   * Switches this sampler to the new attack.
   * Used when a wave tremulant is switched on or off.
   * @param pSampler current playing sampler for switching to a new attack
   */
  void SwitchToAnotherAttack(GOSoundSampler *pSampler);

  float GetRandomFactor() const;

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

  bool IsReleaseAlignmentEnabled() const { return m_IsReleaseAlignmentEnabled; }
  void SetReleaseAlignmentEnabled(bool isEnabled) {
    m_IsReleaseAlignmentEnabled = isEnabled;
  }

  const GOSoundReverb::ReverbConfig &GetReverbConfig() const {
    return m_ReverbConfig;
  }
  void SetReverbConfig(const GOSoundReverb::ReverbConfig &config) {
    m_ReverbConfig = config;
  }

  // TODO: rename to GetAmplitude() (returns linear amplitude coefficient)
  float GetGain() const { return m_gain; }

  // TODO: rename to GetGain() (returns gain in dB)
  int GetVolume() const { return m_volume; }
  // TODO: rename to SetGain(int gain)
  void SetVolume(int volume);

  unsigned GetHardPolyphony() const { return m_SamplerPool.GetUsageLimit(); }
  void SetHardPolyphony(unsigned polyphony);

  bool IsPolyphonyLimiting() const { return m_IsPolyphonyLimiting; }
  void SetPolyphonyLimiting(bool isLimiting) {
    m_IsPolyphonyLimiting = isLimiting;
  }

  bool IsScaledReleases() const { return m_IsScaledReleases; }
  void SetScaledReleases(bool isEnabled) { m_IsScaledReleases = isEnabled; }

  bool IsRandomizeSpeaking() const { return m_IsRandomizeSpeaking; }
  void SetRandomizeSpeaking(bool isEnabled) {
    m_IsRandomizeSpeaking = isEnabled;
  }

  GOSoundResample::InterpolationType GetInterpolationType() const {
    return m_InterpolationType;
  }
  void SetInterpolationType(unsigned type) {
    m_InterpolationType = (GOSoundResample::InterpolationType)type;
  }

  /** Reads parameters from GOConfig and stores them via setters. */
  void SetFromConfig(GOConfig &config);

  /*
   * Start parameter getters (values come via BuildAndStart)
   */

  unsigned GetSampleRate() const override { return m_SampleRate; }
  unsigned GetNSamplesPerBuffer() const { return m_NSamplesPerBuffer; }

  /*
   * Other getters
   */

  uint64_t GetTime() const { return m_CurrentTime; }
  std::vector<float> GetMeterInfo();
  GOSoundScheduler &GetScheduler() { return m_Scheduler; }

  /*
   * Lifecycle state
   */

  /** true if the engine is in the initial state (before BuildAndStart or after
   * StopAndDestroy). */
  bool IsIdle() const {
    return m_LifecycleState.load() == LifecycleState::IDLE;
  }

  /** true if the engine is running (WORKING or USED). */
  bool IsWorking() const {
    return m_LifecycleState.load() >= LifecycleState::WORKING;
  }

  /** true if the engine is connected to the audio system. */
  bool IsUsed() const {
    return m_LifecycleState.load() >= LifecycleState::USED;
  }

  /** Switches between WORKING and USED; called from GOSoundSystem. */
  void SetUsed(bool isUsed);

  /*
   * Public lifecycle functions
   */

  /**
   * @brief Creates tasks and starts the engine.
   *
   * Call after SetFromConfig() or manual setters.
   * After return the engine is ready to receive GetAudioOutput() calls.
   * @param audioOutputConfigs  Output configurations; must not be empty.
   * @param nSamplesPerBuffer   Buffer size in samples (from audio system).
   * @param sampleRate          Sample rate in Hz (from audio system).
   * @param recorder            Recorder (non-owning).
   */
  void BuildAndStart(
    const std::vector<AudioOutputConfig> &audioOutputConfigs,
    unsigned nSamplesPerBuffer,
    unsigned sampleRate,
    GOSoundRecorder &recorder);

  /**
   * @brief Stops the engine and destroys tasks.
   *
   * Call after the audio system has disconnected from the engine.
   */
  void StopAndDestroy();

  /*
   * Organ interface (from GOSoundOrganInterface)
   */

  GOSoundSampler *StartPipeSample(
    const GOSoundProvider *pipeProvider,
    unsigned windchestN,
    unsigned audioGroup,
    unsigned velocity,
    unsigned delay,
    uint64_t prevEventTime,
    bool isRelease = false,
    uint64_t *pStartTimeSamples = nullptr) override {
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

  inline GOSoundSampler *StartTremulantSample(
    const GOSoundProvider *tremProvider,
    unsigned tremulantN,
    uint64_t prevEventTime) override {
    return CreateTaskSample(
      tremProvider, -tremulantN, 0, 0x7f, 0, prevEventTime, false, nullptr);
  }

  uint64_t StopSample(
    const GOSoundProvider *pipe, GOSoundSampler *handle) override;
  void SwitchSample(
    const GOSoundProvider *pipe, GOSoundSampler *handle) override;
  void UpdateVelocity(
    const GOSoundProvider *pipe,
    GOSoundSampler *handle,
    unsigned velocity) override;

  /*
   * Functions called from GOSoundSystem
   */

  void GetAudioOutput(
    unsigned outputIndex, bool isLast, GOSoundBufferMutable &outBuffer);
  void NextPeriod();

  /** Wake up all worker threads. Called from the audio callback. */
  void WakeupThreads();

  bool ProcessSampler(
    float *buffer, GOSoundSampler *sampler, unsigned n_frames, float volume);
  void ProcessRelease(GOSoundSampler *sampler);
  void PassSampler(GOSoundSampler *sampler);
  void ReturnSampler(GOSoundSampler *sampler);
};

#endif /* GOSOUNDORGANENGINE_H */
