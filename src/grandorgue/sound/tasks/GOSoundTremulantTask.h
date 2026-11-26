/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDTREMULANTTASK_H
#define GOSOUNDTREMULANTTASK_H

#include <cstdint>
#include <memory>

#include "sound/buffer/GOSoundTypedBufferLoopBase.h"
#include "sound/effects/GOSoundVibratoPitchRateCurve.h"

#include "GOSoundTaskBase.h"

class GOSchedulerThread;
class GOSoundTremulantWaveForm;
class GOTremulant;

/**
 * Renders a tremulant: every audio period it moves a cursor along the loop of
 * its wave form and publishes the amplitude multiplier for the windchests.
 *
 * The depth g (0 .. 1) of the modulation ramps up over the start time when the
 * tremulant is engaged and down over the stop time when it is disengaged, from
 * any position of the loop. The cursor speeds up and slows down together with
 * g. From its tremulant the task reads only IsEngaged() (on the audio thread;
 * the flag is atomic) and, on the control thread, the parameters through
 * BuildWaveForm().
 *
 * It also publishes the pitch rates of the round (GetPitchRateChunk()) for the
 * vibrato processors of the windchests; both are read from one cursor, so the
 * amplitude and the pitch modulation stay in phase.
 */
class GOSoundTremulantTask : public GOSoundTaskBase {
  friend class GOTestSoundOrganEngine;   // the engine tests read o_WaveForm
  friend class GOTestSoundTremulantTask; // direct access for unit tests
  friend class GOTestSoundTremulantTaskPitch;

private:
  /** The model object; it outlives the task */
  GOTremulant &r_tremulant;

  /** The frames per audio period */
  const unsigned m_NFramesPerBuffer;

  /** The sample rate of the sound engine, in Hz */
  const unsigned m_SampleRate;

  /** The loop; nullptr for a Wave tremulant, where the task does nothing */
  std::unique_ptr<GOSoundTremulantWaveForm> o_WaveForm;

  /** The cursor in the loop, in frames: the phase of the tremulant */
  GOSoundTypedBufferLoopBase::LoopPosition m_position;

  /**
   * Whether the tremulant was engaged in the previous round; it chooses the
   * depth ramp of the wave form: waveForm.GetRamp(m_WasTremulantEngaged)
   */
  bool m_WasTremulantEngaged;

  /**
   * The position in that ramp after the last round, in frames; the ramp is
   * over when it reaches the ramp length, and then it is not moved any more
   */
  unsigned m_RampFrameI;

  /** The amplitude multiplier of the last frame of the last round */
  float m_LastAmplitude;

  /**
   * The pitch rates of the last round: a pointer into the curve of the wave
   * form or into o_RampRates; {nullptr, 0, 0} if there is no pitch modulation
   */
  GOSoundVibratoPitchRateCurve::ChunkDescription m_PitchChunk;

  /**
   * The rates of a round while the depth is changing, m_NFramesPerBuffer
   * items; nullptr for a Wave tremulant. It is round state, not part of the
   * wave form, and not on the stack because the windchests read it after
   * DoRun(). It is allocated always, also while the current wave form has no
   * pitch modulation: SwapWaveForm() may bring a wave form with the pitch at
   * any time, and it runs with the engine stopped, where an allocation is
   * forbidden (it would lengthen the pause), so the buffer must already exist
   */
  std::unique_ptr<uint16_t[]> o_RampRates;

  bool DoRun(GOSchedulerThread *pThread) override;

public:
  /**
   * @param tremulant the model object; must outlive the task
   * @param nFramesPerBuffer the frames per audio period
   * @param sampleRate the sample rate of the sound engine, in Hz
   */
  GOSoundTremulantTask(
    GOTremulant &tremulant, unsigned nFramesPerBuffer, unsigned sampleRate);

  // Defined in the .cpp: GOSoundTremulantWaveForm is incomplete here
  ~GOSoundTremulantTask();

  /**
   * Builds a wave form from the current parameters of the tremulant. Call on
   * the control thread; the sound may be playing, the task is not touched.
   *
   * @return the wave form, nullptr for a Wave tremulant
   */
  std::unique_ptr<GOSoundTremulantWaveForm> BuildWaveForm() const;

  /**
   * Installs a wave form built by BuildWaveForm() and hands the previous one
   * back. Must be called with the engine stopped (between StopEngine() and
   * StartEngine(), or in BuildEngine() before it starts): DoRun() must not be
   * running. Neither allocates nor frees.
   *
   * Why a swap and not a plain set: a plain set would free the old wave form
   * here, while the sound is suspended, and lengthen the pause. The swap gives
   * it back to the caller, which frees it after the resume.
   *
   * The tremulant continues from the same phase and the same depth: the cursor
   * keeps its fraction of the loop if the loop length changed, a running ramp
   * keeps its depth although its length may have changed. The pitch chunk is
   * reset because it pointed into the old curve.
   *
   * @param ioWaveForm in: the new wave form (nullptr for a Wave tremulant);
   *   out: the old one
   */
  void SwapWaveForm(std::unique_ptr<GOSoundTremulantWaveForm> &ioWaveForm);

  /**
   * Runs the task if it has not run in this round yet; idempotent. The
   * getters below only read the result of the round and never run the task,
   * so whoever needs them calls this first (GOSoundWindchestTask::DoRun()).
   */
  void EnsureDone();

  /**
   * @return the amplitude multiplier of the last frame of the current round,
   *   1.0 = none. Valid from DoRun() to the next DoRun(); does not run the
   *   task, call EnsureDone() first (asserts IsDone()).
   */
  float GetAmplitude() const;

  /**
   * @return the pitch rates for the vibrato in the current round, or
   *   {nullptr, 0, 0} if the pitch is not modulated now. Valid from DoRun() to
   *   the next DoRun(); does not run the task (asserts IsDone()), so call
   *   EnsureDone() first or call it after the scheduler has run the task.
   */
  const GOSoundVibratoPitchRateCurve::ChunkDescription &GetPitchRateChunk()
    const;
};

#endif
