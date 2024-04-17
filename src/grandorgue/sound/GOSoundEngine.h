/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDENGINE_H_
#define GOSOUNDENGINE_H_

#include <atomic>
#include <vector>

#include "scheduler/GOSoundScheduler.h"

#include "GOSoundResample.h"
#include "GOSoundSampler.h"
#include "GOSoundSamplerPool.h"

class GOWindchest;
class GOSoundProvider;
class GOSoundRecorder;
class GOSoundGroupWorkItem;
class GOSoundOutputWorkItem;
class GOSoundReleaseWorkItem;
class GOSoundTouchWorkItem;
class GOSoundTremulantWorkItem;
class GOSoundWindchestWorkItem;
class GOSoundWorkItem;
class GOOrganController;
class GOConfig;

typedef struct {
  unsigned channels;
  std::vector<std::vector<float>> scale_factors;
} GOAudioOutputConfiguration;

class GOSoundEngine {
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
  unsigned m_WorkerSlots;
  std::vector<double> m_MeterInfo;
  ptr_vector<GOSoundTremulantWorkItem> m_TremulantTasks;
  ptr_vector<GOSoundWindchestWorkItem> m_WindchestTasks;
  ptr_vector<GOSoundGroupWorkItem> m_AudioGroupTasks;
  ptr_vector<GOSoundOutputWorkItem> m_AudioOutputTasks;
  GOSoundRecorder *m_AudioRecorder;
  GOSoundReleaseWorkItem *m_ReleaseProcessor;
  std::unique_ptr<GOSoundTouchWorkItem> m_TouchTask;
  GOSoundScheduler m_Scheduler;

  struct resampler_coefs_s m_ResamplerCoefs;

  std::atomic_bool m_HasBeenSetup;

  unsigned SamplesDiffToMs(uint64_t fromSamples, uint64_t toSamples);

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
  float GetRandomFactor();
  unsigned GetBufferSizeFor(unsigned outputIndex, unsigned n_frames);

public:
  GOSoundEngine();
  ~GOSoundEngine();
  void Reset();
  void Setup(GOOrganController *organController, unsigned release_count = 1);
  void ClearSetup();
  void SetAudioOutput(std::vector<GOAudioOutputConfiguration> audio_outputs);
  void SetupReverb(GOConfig &settings);
  void SetVolume(int volume);
  void SetSampleRate(unsigned sample_rate);
  void SetSamplesPerBuffer(unsigned sample_per_buffer);
  void SetInterpolationType(unsigned type);
  unsigned GetSampleRate();
  void SetAudioGroupCount(unsigned groups);
  unsigned GetAudioGroupCount();
  void SetHardPolyphony(unsigned polyphony);
  void SetPolyphonyLimiting(bool limiting);
  unsigned GetHardPolyphony() const;
  int GetVolume() const;
  void SetScaledReleases(bool enable);
  void SetRandomizeSpeaking(bool enable);
  const std::vector<double> &GetMeterInfo();
  void SetAudioRecorder(GOSoundRecorder *recorder, bool downmix);

  inline GOSoundSampler *StartPipeSample(
    const GOSoundProvider *pipeProvider,
    unsigned windchestN,
    unsigned audioGroup,
    unsigned velocity,
    unsigned delay,
    uint64_t prevEventTime,
    bool isRelease = false,
    uint64_t *pStartTimeSamples = nullptr) {
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
    uint64_t prevEventTime) {
    return CreateTaskSample(
      tremProvider, -tremulantN, 0, 0x7f, 0, prevEventTime, false, nullptr);
  }

  uint64_t StopSample(const GOSoundProvider *pipe, GOSoundSampler *handle);
  void SwitchSample(const GOSoundProvider *pipe, GOSoundSampler *handle);
  void UpdateVelocity(
    const GOSoundProvider *pipe, GOSoundSampler *handle, unsigned velocity);

  void GetEmptyAudioOutput(
    unsigned outputIndex, unsigned nFrames, float *pOutputBuffer);
  void GetAudioOutput(
    float *output_buffer, unsigned n_frames, unsigned audio_output, bool last);
  void NextPeriod();
  GOSoundScheduler &GetScheduler();

  bool ProcessSampler(
    float *buffer, GOSoundSampler *sampler, unsigned n_frames, float volume);
  void ProcessRelease(GOSoundSampler *sampler);
  void PassSampler(GOSoundSampler *sampler);
  void ReturnSampler(GOSoundSampler *sampler);
  float GetGain();
  uint64_t GetTime() const { return m_CurrentTime; }
};

#endif /* GOSOUNDENGINE_H_ */
