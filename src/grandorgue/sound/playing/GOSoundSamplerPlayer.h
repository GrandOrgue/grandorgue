/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDSAMPLERPLAYER_H
#define GOSOUNDSAMPLERPLAYER_H

#include <atomic>
#include <cstdint>
#include <memory>
#include <vector>

#include "GOSoundResample.h"
#include "GOSoundSamplerPool.h"
#include "sound/GOSoundOrganInterface.h"

#include "ptrvector.h"

class GOSoundGroupTask;
class GOSoundReleaseTask;
class GOSoundTremulantTask;
class GOSoundWindchestTask;

/**
 * @brief Manages samplers and implements the organ sound interface.
 *
 * Handles sampler lifecycle (start, stop, release, switch), sampler pool
 * management, and the GOSoundOrganInterface methods called by the organ model.
 * Designed to be owned by GOSoundOrganEngine.
 */
class GOSoundSamplerPlayer : public GOSoundOrganInterface {
private:
  /*
   * Constructor constants
   */

  ptr_vector<GOSoundGroupTask> &r_AudioGroupTasks;
  std::vector<std::unique_ptr<GOSoundWindchestTask>> &r_WindchestTasks;
  ptr_vector<GOSoundTremulantTask> &r_TremulantTasks;
  /** nullptr when player is constructed; populated in the body of
   * GOSoundOrganEngine constructor. Used only during playback (after Build). */
  std::unique_ptr<GOSoundReleaseTask> &rp_ReleaseTask;
  GOSoundSamplerPool m_SamplerPool;
  GOSoundResample m_resample;

  /*
   * Configuration parameters
   */

  bool m_IsScaledReleases;
  bool m_IsReleaseAlignmentEnabled;
  bool m_IsRandomizeSpeaking;
  bool m_IsPolyphonyLimiting;
  unsigned m_PolyphonySoftLimit;
  GOSoundResample::InterpolationType m_InterpolationType;

  /*
   * Start parameters (set in Build())
   */

  unsigned m_SampleRate;

  /*
   * State
   */

  /** Current engine time in samples. Starts at 1 after Reset() so that zero
   * can be used as a sentinel meaning "not set" in sampler fields (e.g.
   * stop, new_attack). */
  uint64_t m_CurrentTime;
  std::atomic_uint m_UsedPolyphony;

  /*
   * Private helpers
   */

  unsigned MsToSamples(unsigned ms) const { return m_SampleRate * ms / 1000; }

  unsigned SamplesDiffToMs(uint64_t fromSamples, uint64_t toSamples) const;

  static constexpr int DETACHED_RELEASE_TASK_ID = 0;

  static bool isWindchestTask(int taskId) { return taskId >= 0; }

  static unsigned windchestTaskToIndex(int taskId) { return (unsigned)taskId; }

  static unsigned tremulantTaskToIndex(int taskId) { return -taskId - 1; }

  float GetRandomFactor() const;

  void StartSampler(GOSoundSampler *sampler);

  /**
   * @brief Creates and starts a new sampler.
   *
   * @param pSoundProvider     Audio data source.
   * @param samplerTaskId      Windchest task index (>=0) or negated tremulant
   *                           index (<0).
   * @param audioGroup         Audio group index.
   * @param velocity           MIDI velocity (0–127).
   * @param delay              Start delay in milliseconds.
   * @param prevEventTime      Time of the previous event in samples.
   * @param isRelease          true to start with the release section.
   * @param pStartTimeSamples  If non-null, receives the actual start time
   *                           in samples.
   * @return the new sampler, or nullptr if the pool is exhausted.
   */
  GOSoundSampler *CreateTaskSample(
    const GOSoundProvider *pSoundProvider,
    int samplerTaskId,
    unsigned audioGroup,
    unsigned velocity,
    unsigned delay,
    uint64_t prevEventTime,
    bool isRelease,
    uint64_t *pStartTimeSamples);

  /**
   * @brief Creates a release tail sampler for the given pipe sampler.
   * @param handle  The currently playing pipe sampler being released.
   */
  void CreateReleaseSampler(GOSoundSampler *handle);

  /**
   * @brief Creates a new sampler with decay of current loop and switches
   * this sampler to the new attack. Used when a wave tremulant is switched
   * on or off.
   * @param pSampler  Current playing sampler for switching to a new attack.
   */
  void SwitchToAnotherAttack(GOSoundSampler *pSampler);

public:
  /*
   * Constructor
   */

  /**
   * @brief Constructs the player.
   *
   * Task containers are passed by reference; they may be empty at construction
   * time and are populated later by GOSoundOrganEngine::BuildEngine().
   * pReleaseTask is nullptr at construction; it is set in
   * GOSoundOrganEngine's constructor body before any playback begins.
   */
  GOSoundSamplerPlayer(
    ptr_vector<GOSoundGroupTask> &audioGroupTasks,
    std::vector<std::unique_ptr<GOSoundWindchestTask>> &windchestTasks,
    ptr_vector<GOSoundTremulantTask> &tremulantTasks,
    std::unique_ptr<GOSoundReleaseTask> &pReleaseTask);

  /*
   * Configuration getters and setters
   */

  unsigned GetHardPolyphony() const { return m_SamplerPool.GetUsageLimit(); }
  void SetHardPolyphony(unsigned polyphony);

  bool IsPolyphonyLimiting() const { return m_IsPolyphonyLimiting; }
  void SetPolyphonyLimiting(bool isLimiting) {
    m_IsPolyphonyLimiting = isLimiting;
  }

  bool IsScaledReleases() const { return m_IsScaledReleases; }
  void SetScaledReleases(bool isEnabled) { m_IsScaledReleases = isEnabled; }

  bool IsReleaseAlignmentEnabled() const { return m_IsReleaseAlignmentEnabled; }
  void SetReleaseAlignmentEnabled(bool isEnabled) {
    m_IsReleaseAlignmentEnabled = isEnabled;
  }

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

  /*
   * Start parameter getters
   */

  unsigned GetSampleRate() const override { return m_SampleRate; }

  /*
   * Other getters
   */

  /** Returns the current engine time in samples. Starts at 1 after Reset()
   * so that zero can be used as a sentinel meaning "not set". */
  uint64_t GetTime() const { return m_CurrentTime; }

  /** Returns the peak used polyphony since the last call and resets the
   * counter to zero. */
  unsigned GetAndResetUsedPolyphony() { return m_UsedPolyphony.exchange(0); }

  /*
   * Lifecycle
   */

  /**
   * @brief Stores the sample rate and asserts that tasks are ready.
   *
   * Must be called after GOSoundOrganEngine builds and populates the task
   * containers and sets rp_ReleaseTask.
   * @param sampleRate  Sample rate in Hz.
   */
  void Build(unsigned sampleRate);

  /** @brief Symmetric to Build: resets m_SampleRate. */
  void Destroy();

  /** @brief Resets m_CurrentTime to 1 (0 is sentinel "not set"), returns
   * all samplers to the pool, and clears m_UsedPolyphony. */
  void Reset();

  /**
   * @brief Advances the current time by nSamplesPerBuffer samples and
   * updates the used polyphony counter.
   */
  void AdvanceTime(unsigned nSamplesPerBuffer);

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
    uint64_t *pStartTimeSamples = nullptr) override;

  GOSoundSampler *StartTremulantSample(
    const GOSoundProvider *tremProvider,
    unsigned tremulantN,
    uint64_t prevEventTime) override;

  uint64_t StopSample(
    const GOSoundProvider *pipe, GOSoundSampler *handle) override;
  void SwitchSample(
    const GOSoundProvider *pipe, GOSoundSampler *handle) override;
  void UpdateVelocity(
    const GOSoundProvider *pipe,
    GOSoundSampler *handle,
    unsigned velocity) override;

  /*
   * Functions called from tasks
   */

  /**
   * @brief Processes one period of audio for a single sampler.
   *
   * @param output_buffer  Interleaved stereo buffer to mix into.
   * @param sampler        The sampler to process.
   * @param n_frames       Number of frames.
   * @param volume         Windchest/tremulant volume factor.
   * @return true if the sampler should be kept alive for the next period.
   */
  bool ProcessSampler(
    float *output_buffer,
    GOSoundSampler *sampler,
    unsigned n_frames,
    float volume);

  /**
   * @brief Processes a release or attack-switch event for a sampler.
   *
   * Called from GOSoundReleaseTask. Starts a release tail (if stop is set)
   * or switches to another attack (if new_attack is set), then re-queues
   * the sampler.
   * @param sampler  The sampler to process.
   */
  void ProcessRelease(GOSoundSampler *sampler);

  /**
   * @brief Queues a sampler into the appropriate group or tremulant task.
   * @param sampler  The sampler to pass to its task.
   */
  void PassSampler(GOSoundSampler *sampler);

  /**
   * @brief Returns a sampler to the pool.
   * @param sampler  The sampler to return.
   */
  void ReturnSampler(GOSoundSampler *sampler);
};

#endif /* GOSOUNDSAMPLERPLAYER_H */
