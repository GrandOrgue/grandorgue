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
#include "scheduler/GOSoundScheduler.h"

#include "GOSoundOrganInterface.h"

class GOConfig;
class GOMemoryPool;
class GOOrganModel;
class GOSoundBufferMutable;
class GOSoundProvider;
class GOSoundRecorder;
class GOSoundGroupTask;
class GOSoundOutputTask;
class GOSoundReleaseTask;
class GOSoundThread;
class GOSoundTouchTask;
class GOSoundTremulantTask;
class GOSoundWindchestTask;
class GOSoundTask;
class GOWindchest;

typedef struct {
  unsigned channels;
  std::vector<std::vector<float>> scale_factors;
} GOAudioOutputConfiguration;

/**
 * This class represents a sound engine of one loaded organ. It reflects some
 * GrandOrgue-wide objects (AudioGroup) and some objects of it's model
 * (WindChests, Tremulants)
 */

class GOSoundOrganEngine : public GOSoundOrganInterface {
private:
  static constexpr int DETACHED_RELEASE_TASK_ID = 0;

  unsigned m_PolyphonySoftLimit;
  bool m_PolyphonyLimiting;
  bool m_ScaledReleases;
  bool m_ReleaseAlignmentEnabled;
  bool m_RandomizeSpeaking;
  int m_Volume;
  unsigned m_SamplesPerBuffer;
  float m_Gain;
  unsigned m_SampleRate;

  // time in samples
  uint64_t m_CurrentTime;
  GOSoundSamplerPool m_SamplerPool;
  unsigned m_AudioGroupCount;
  std::atomic_uint m_UsedPolyphony;

  // protects mp_MeterInfo/p_MeterInfo against concurrent replacement
  // (SetAudioOutput vs GetMeterInfo)
  GOMutex m_MeterMutex;

  // Per-channel peak levels for the meter display (one element per real output
  // channel; polyphony is tracked separately in m_UsedPolyphony).
  //
  // Ownership and access pattern:
  //   Audio thread   — p_MeterInfo.load(acquire) each period; writes peaks via
  //                    atomic_fetch_max_relaxed(); no mutex held
  //   GUI thread     — GetMeterInfo() under m_MeterMutex; reads *p_MeterInfo,
  //                    resets elements via exchange(0); prepends polyphony
  //                    ratio from m_UsedPolyphony.exchange(0)
  //   Control thread — SetAudioOutput() under m_MeterMutex; creates a new
  //                    vector when channel count changes, stores pointer via
  //                    p_MeterInfo.store(release)
  //
  // mp_MeterInfo owns the vector; p_MeterInfo is an atomic pointer to the
  // same object (always == mp_MeterInfo.get()). SetAudioOutput() always
  // allocates a new vector, so size and data pointer are always consistent
  // within a single vector object published atomically via p_MeterInfo.
  // std::shared_ptr with atomic access would provide automatic lifetime safety
  // but incurs atomic reference-count increments/decrements on every
  // NextPeriod() call (~100/s), adding unnecessary overhead in the audio
  // thread. float is used instead of double because std::atomic<float> is
  // lock-free on all supported platforms (x86, ARM), while std::atomic<double>
  // is not.
  std::unique_ptr<std::vector<std::atomic<float>>> mp_MeterInfo;
  // always == mp_MeterInfo.get()
  std::atomic<std::vector<std::atomic<float>> *> p_MeterInfo;

  ptr_vector<GOSoundTremulantTask> m_TremulantTasks;
  ptr_vector<GOSoundWindchestTask> m_WindchestTasks;
  ptr_vector<GOSoundGroupTask> m_AudioGroupTasks;
  ptr_vector<GOSoundOutputTask> m_AudioOutputTasks;
  GOSoundRecorder *m_AudioRecorder;
  GOSoundReleaseTask *m_ReleaseProcessor;
  std::unique_ptr<GOSoundTouchTask> m_TouchTask;
  GOSoundScheduler m_Scheduler;

  std::vector<std::unique_ptr<GOSoundThread>> mp_threads;
  GOMutex m_ThreadLock;

  GOSoundResample m_resample;
  GOSoundResample::InterpolationType m_interpolation;

  std::atomic_bool m_HasBeenSetup;

  void StartThreads(unsigned nConcurrency);
  void StopThreads();

  unsigned MsToSamples(unsigned ms) const { return m_SampleRate * ms / 1000; }

  unsigned SamplesDiffToMs(uint64_t fromSamples, uint64_t toSamples) const;

  /* samplerTaskId:
     -1 .. -n Tremulants
     0 (DETACHED_RELEASE_TASK_ID) detached release
     1 .. n Windchests
  */
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
   * Switch this sampler to the new attack.
   * It is used when a wave tremulant is switched on or off.
   * @param pSampler current playing sampler for switching to a new attack
   */
  void SwitchToAnotherAttack(GOSoundSampler *pSampler);
  float GetRandomFactor() const;
  unsigned GetBufferSizeFor(unsigned outputIndex, unsigned n_frames) const;

public:
  GOSoundOrganEngine();
  ~GOSoundOrganEngine();

  // Group 1: Independent setters (no call-order dependencies)
  unsigned GetSampleRate() const override { return m_SampleRate; }
  void SetSampleRate(unsigned sample_rate) { m_SampleRate = sample_rate; }
  void SetSamplesPerBuffer(unsigned sample_per_buffer) {
    m_SamplesPerBuffer = sample_per_buffer;
  }
  int GetVolume() const { return m_Volume; }
  void SetVolume(int volume);
  unsigned GetHardPolyphony() const { return m_SamplerPool.GetUsageLimit(); }
  void SetHardPolyphony(unsigned polyphony);
  void SetPolyphonyLimiting(bool limiting) { m_PolyphonyLimiting = limiting; }
  void SetScaledReleases(bool enable) { m_ScaledReleases = enable; }
  void SetRandomizeSpeaking(bool enable) { m_RandomizeSpeaking = enable; }
  void SetInterpolationType(unsigned type) {
    m_interpolation = (GOSoundResample::InterpolationType)type;
  }

  // Group 2: Dependent setters (must be called in this order)
  // Must be called after SetSamplesPerBuffer because uses m_SamplesPerBuffer
  unsigned GetAudioGroupCount() const { return m_AudioGroupCount; }
  void SetAudioGroupCount(unsigned groups);
  // Must be called after SetAudioGroupCount because uses m_AudioGroupCount and
  // m_SamplesPerBuffer
  void SetAudioOutput(std::vector<GOAudioOutputConfiguration> audio_outputs);
  // Must be called after SetAudioOutput because iterates m_AudioOutputTasks
  void SetupReverb(GOConfig &settings);
  // Must be called after SetAudioOutput because uses m_AudioOutputTasks
  void SetAudioRecorder(GOSoundRecorder *recorder, bool downmix);

  // Group 3: Other getters
  float GetGain() const { return m_Gain; }
  uint64_t GetTime() const { return m_CurrentTime; }
  std::vector<float> GetMeterInfo();
  GOSoundScheduler &GetScheduler() { return m_Scheduler; }

  void Reset();
  void Setup(
    GOOrganModel &organModel,
    GOMemoryPool &memoryPool,
    unsigned releaseCount = 1);
  void ClearSetup();

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

  /** Configure the engine for the given organ and start worker threads */
  void BuildAndStart(
    GOConfig &config,
    unsigned sampleRate,
    unsigned samplesPerBuffer,
    GOOrganModel &organModel,
    GOMemoryPool &memoryPool,
    unsigned releaseConcurrency,
    GOSoundRecorder &audioRecorder);

  /** Stop worker threads and tear down the organ setup */
  void StopAndDestroy();

  /** Wake up all worker threads. Called from the audio callback. */
  void WakeupThreads();

  void GetAudioOutput(
    unsigned outputIndex, bool isLast, GOSoundBufferMutable &outBuffer);
  void NextPeriod();

  bool ProcessSampler(
    float *buffer, GOSoundSampler *sampler, unsigned n_frames, float volume);
  void ProcessRelease(GOSoundSampler *sampler);
  void PassSampler(GOSoundSampler *sampler);
  void ReturnSampler(GOSoundSampler *sampler);
};

#endif /* GOSOUNDORGANENGINE_H */
