/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundTremulantTask.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <utility>

#include "model/GOTremulant.h"
#include "sound/dsp-kernels/GOSoundLinearFaderCore.h"
#include "sound/dsp-kernels/GOSoundResample.h"

#include "GOSoundTremulantWaveForm.h"

using UnitRamp = GOSoundLinearFaderCore::UnitRamp;
using ChunkDescription = GOSoundVibratoPitchRateCurve::ChunkDescription;

/**
 * The advance of a cursor over the first frameI frames of a depth ramp, in
 * whole frames.
 */
static int64_t ramp_advance_at(const UnitRamp &ramp, unsigned frameI) {
  return std::llround(ramp.GetGainSumBefore(frameI));
}

/**
 * Fills the rates of a round while the depth is changing:
 * UNIT + g * (rate - UNIT) per frame, where the rate is read from the curve at
 * the cursor position of that frame.
 *
 * @param curve the pitch rates of the wave form
 * @param roundStartPos the position of the cursor at the start of the round
 * @param ramp the depth ramp
 * @param roundStartRampFrameI the position in the ramp at the start of the
 *   round
 * @param advanceBeforeRound ramp_advance_at(ramp, roundStartRampFrameI)
 * @param nFrames the frames per audio period
 * @param outRates nFrames items to fill
 * @return the rates with their sum
 */
static ChunkDescription fill_ramp_rates(
  const GOSoundVibratoPitchRateCurve &curve,
  const GOSoundTypedBufferLoopBase::LoopPosition &roundStartPos,
  const UnitRamp &ramp,
  unsigned roundStartRampFrameI,
  int64_t advanceBeforeRound,
  unsigned nFrames,
  uint16_t *outRates) {
  constexpr int UNIT = GOSoundResample::UPSAMPLE_FACTOR;
  /* The cursor moves at most one frame per frame, so every read is inside the
   * chunk of the curve that starts at roundStartPos */
  const uint16_t *pLoopRates = curve.GetChunk(roundStartPos);
  unsigned nTotalUnits = 0;

  for (unsigned frameI = 0; frameI < nFrames; ++frameI) {
    const unsigned rampFrameI = roundStartRampFrameI + frameI;
    const int64_t offset
      = ramp_advance_at(ramp, rampFrameI) - advanceBeforeRound;

    assert(offset >= 0 && offset <= (int64_t)frameI);

    const unsigned rate = (unsigned)std::lround(
      UNIT + ramp.GetGainAt(rampFrameI) * ((int)pLoopRates[offset] - UNIT));

    outRates[frameI] = rate;
    nTotalUnits += rate;
  }
  return {outRates, nFrames, nTotalUnits};
}

GOSoundTremulantTask::GOSoundTremulantTask(
  GOTremulant &tremulant, unsigned nFramesPerBuffer, unsigned sampleRate)
  : GOSoundTaskBase(PRIORITY_TREMULANT, false),
    r_tremulant(tremulant),
    m_NFramesPerBuffer(nFramesPerBuffer),
    m_SampleRate(sampleRate),
    m_WasTremulantEngaged(false),
    m_RampFrameI(0),
    m_LastAmplitude(1.0f),
    m_PitchChunk{nullptr, 0, 0},
    /* Always for a Synth tremulant, see the member; a Wave tremulant never gets
     * a wave form (BuildWaveForm() gives nullptr), so never needs the buffer */
    o_RampRates(
      tremulant.GetTremulantType() != GOWavTrem
        ? std::make_unique_for_overwrite<uint16_t[]>(nFramesPerBuffer)
        : nullptr) {}

GOSoundTremulantTask::~GOSoundTremulantTask() = default;

std::unique_ptr<GOSoundTremulantWaveForm> GOSoundTremulantTask::BuildWaveForm()
  const {
  std::unique_ptr<GOSoundTremulantWaveForm> oWaveForm;

  if (r_tremulant.GetTremulantType() != GOWavTrem) {
    const GOTremulant::Description desc = r_tremulant.Render(m_SampleRate);

    oWaveForm
      = std::make_unique<GOSoundTremulantWaveForm>(desc, m_NFramesPerBuffer);
  }
  return oWaveForm;
}

void GOSoundTremulantTask::SwapWaveForm(
  std::unique_ptr<GOSoundTremulantWaveForm> &ioWaveForm) {
  std::swap(o_WaveForm, ioWaveForm);
  // It pointed into the curve of the old wave form
  m_PitchChunk = {nullptr, 0, 0};
  if (o_WaveForm) {
    const GOSoundTremulantWaveForm &newWaveForm = *o_WaveForm;
    const unsigned nLoopFrames = newWaveForm.GetNLoopFrames();

    /* Keep the fraction of the loop. A position that was never valid (there
     * was no wave form before) starts at 0 */
    if (m_position.GetNLoopFrames() != nLoopFrames)
      m_position
        = GOSoundTypedBufferLoopBase::LoopPosition(m_position, nLoopFrames);
    /* Keep the depth: the ramp of the same direction may have another length
     * now. With no old form the task has been idle: the falling ramp is over */
    m_RampFrameI = ioWaveForm ? GOSoundLinearFaderCore::mapFrameOffsetOrJump(
                     ioWaveForm->GetRamp(m_WasTremulantEngaged),
                     m_RampFrameI,
                     newWaveForm.GetRamp(m_WasTremulantEngaged))
                              : newWaveForm.GetRamp(false).GetNRampFrames();
    /* The new falling ramp is already over (an instant stop or a shorter stop
     * time): the tremulant is stopped now, so the next engagement must start
     * from the loop start, as DoRun() does when a falling ramp ends */
    if (
      !m_WasTremulantEngaged
      && newWaveForm.GetRamp(false).IsOverAt(m_RampFrameI))
      m_position.Reset();
  } else {
    // A Wave tremulant: nothing to play
    m_position = GOSoundTypedBufferLoopBase::LoopPosition();
    m_RampFrameI = 0;
  }
}

void GOSoundTremulantTask::EnsureDone() {
  if (!IsDone())
    Run();
}

float GOSoundTremulantTask::GetAmplitude() const {
  assert(IsDone());
  return m_LastAmplitude;
}

const ChunkDescription &GOSoundTremulantTask::GetPitchRateChunk() const {
  assert(IsDone());
  return m_PitchChunk;
}

bool GOSoundTremulantTask::DoRun(GOSchedulerThread *pThread) {
  // The amplitude of a round is the amplitude of its last frame
  float lastAmplitude = 1.0f;

  // No pitch modulation unless a branch below sets it
  m_PitchChunk = {nullptr, 0, 0};
  if (o_WaveForm) {
    const GOSoundTremulantWaveForm &waveForm = *o_WaveForm;
    const bool isEngaged = r_tremulant.IsEngaged();
    const UnitRamp &ramp = waveForm.GetRamp(isEngaged);

    /* The engagement changed: go on with the ramp of the new direction from the
     * same depth */
    if (isEngaged != m_WasTremulantEngaged) {
      m_RampFrameI = GOSoundLinearFaderCore::mapFrameOffsetOrJump(
        waveForm.GetRamp(m_WasTremulantEngaged), m_RampFrameI, ramp);
      m_WasTremulantEngaged = isEngaged;
      // An instant stop: the falling ramp has no frames and is over at once
      if (!isEngaged && ramp.IsOverAt(m_RampFrameI))
        m_position.Reset();
    }

    // The depth is still changing: the ramp has not reached its end
    const bool isInRamp = !ramp.IsOverAt(m_RampFrameI);

    if (isEngaged || isInRamp) {
      // How far the cursor moves in this round, in frames of the loop
      unsigned nToLastFrame;
      unsigned nToNextRound;
      float rampFactor;
      /* False if the falling ramp reaches its end in this round: the tremulant
       * stops and the cursor goes back to the loop start */
      bool isToContinue = true;

      if (isInRamp) {
        /* The cursor moves by the sum of the ramp gain over the frames passed.
         * The frames of the round have the positions in the ramp
         * [roundStartRampFrameI, roundEndRampFrameI), the last one is
         * lastRampFrameI */
        const unsigned roundStartRampFrameI = m_RampFrameI;
        const unsigned roundEndRampFrameI
          = roundStartRampFrameI + m_NFramesPerBuffer;
        const unsigned lastRampFrameI = roundEndRampFrameI - 1;
        const int64_t advanceBeforeRound
          = ramp_advance_at(ramp, roundStartRampFrameI);
        const int64_t advanceBeforeLastFrame
          = ramp_advance_at(ramp, lastRampFrameI);
        const int64_t advanceAfterRound
          = ramp_advance_at(ramp, roundEndRampFrameI);

        // The ramp gain is never negative, so the cursor never goes back
        assert(
          advanceBeforeRound <= advanceBeforeLastFrame
          && advanceBeforeLastFrame <= advanceAfterRound);

        nToLastFrame = (unsigned)(advanceBeforeLastFrame - advanceBeforeRound);
        nToNextRound = (unsigned)(advanceAfterRound - advanceBeforeRound);
        rampFactor = ramp.GetGainAt(lastRampFrameI);
        // The pitch of the same frames; it reads m_position before it moves
        if (waveForm.HasPitch())
          m_PitchChunk = fill_ramp_rates(
            waveForm.GetPitchRateCurve(),
            m_position,
            ramp,
            roundStartRampFrameI,
            advanceBeforeRound,
            m_NFramesPerBuffer,
            o_RampRates.get());
        // Once the ramp is over, it is not moved nor read any more
        m_RampFrameI = std::min(roundEndRampFrameI, ramp.GetNRampFrames());
        isToContinue = isEngaged || !ramp.IsOverAt(m_RampFrameI);
      } else {
        // The full engagement: the cursor moves one frame per frame
        nToLastFrame = m_NFramesPerBuffer - 1;
        nToNextRound = m_NFramesPerBuffer;
        rampFactor = 1.0f;
        if (waveForm.HasPitch())
          m_PitchChunk
            = waveForm.GetPitchRateCurve().GetChunkDescription(m_position);
      }

      GOSoundTypedBufferLoopBase::LoopPosition lastPos = m_position;

      lastPos.Advance(nToLastFrame);
      lastAmplitude = waveForm.GetAmplitudeAt(lastPos, rampFactor);
      if (isToContinue)
        m_position.Advance(nToNextRound);
      else
        // The next engagement starts the loop from its beginning
        m_position.Reset();
    }
    // else: idle, nothing to do
  }
  m_LastAmplitude = lastAmplitude;
  return true;
}
