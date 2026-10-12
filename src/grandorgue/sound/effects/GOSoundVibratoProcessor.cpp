/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundVibratoProcessor.h"

#include <algorithm>

/**
 * Converts milliseconds to frames, rounding up; 0 ms gives 0 frames. In
 * integers, so the result is exact; 64 bits keep the product from overflowing.
 */
static unsigned ms_to_frames(unsigned ms, unsigned sampleRate) {
  return (unsigned)(((uint64_t)ms * sampleRate + 999) / 1000);
}

GOSoundVibratoProcessor::GOSoundVibratoProcessor(
  const GOSoundResample &resample,
  GOSoundResample::InterpolationType interpolationType,
  unsigned maxLagMs)
  : r_resample(resample),
    m_InterpolationType(interpolationType),
    m_MaxLagMs(maxLagMs) {}

void GOSoundVibratoProcessor::EnsureSetup(
  unsigned nChannels,
  unsigned nFrames,
  unsigned sampleRate,
  unsigned nMaxLagFrames,
  unsigned nActivationFrames,
  unsigned nDeactivationFrames) {
  /* normalisePitchRateCurve() needs the target mean to be within the allowed
   * range, which a one-frame period cannot guarantee */
  assert(nFrames >= 2);

  m_NChannels = nChannels;
  m_SampleRate = sampleRate;
  m_NFramesPerPeriod = nFrames;
  m_MinAllowedIncrementPerPeriod
    = (uint64_t)nFrames * GOSoundVibratoPitchRateCurve::MIN_ALLOWED_RATE_UNITS;
  m_MaxAllowedIncrementPerPeriod
    = (uint64_t)nFrames * GOSoundVibratoPitchRateCurve::MAX_ALLOWED_RATE_UNITS;
  m_NDelayBufferRingFrames = nMaxLagFrames + nFrames
    + GOSoundVibratoProcessorState::N_LOOKAHEAD_FRAMES;
  m_PitchRateChunk = {nullptr, 0, 0};
  m_ActivationRamp = GOSoundLinearFaderCore::CrossfadeRamp(
    GOSoundLinearFaderCore::TO_OTHER, nActivationFrames);
  m_DeactivationRamp = GOSoundLinearFaderCore::CrossfadeRamp(
    GOSoundLinearFaderCore::FROM_OTHER, nDeactivationFrames);
  m_IsSetupCalled = true;
}

void GOSoundVibratoProcessor::EnsureSetup(
  unsigned nChannels, unsigned nFrames, unsigned sampleRate) {
  EnsureSetup(
    nChannels,
    nFrames,
    sampleRate,
    ms_to_frames(m_MaxLagMs, sampleRate),
    ms_to_frames(m_ActivationCrossfadeMs, sampleRate),
    ms_to_frames(m_DeactivationCrossfadeMs, sampleRate));
}

void GOSoundVibratoProcessor::SetActivationCrossfadeMs(unsigned ms) {
  assert(ms <= MAX_CROSSFADE_MS);
  m_ActivationCrossfadeMs = ms;
  if (m_IsSetupCalled)
    m_ActivationRamp = GOSoundLinearFaderCore::CrossfadeRamp(
      GOSoundLinearFaderCore::TO_OTHER, ms_to_frames(ms, m_SampleRate));
}

void GOSoundVibratoProcessor::SetDeactivationCrossfadeMs(unsigned ms) {
  assert(ms <= MAX_CROSSFADE_MS);
  m_DeactivationCrossfadeMs = ms;
  if (m_IsSetupCalled)
    m_DeactivationRamp = GOSoundLinearFaderCore::CrossfadeRamp(
      GOSoundLinearFaderCore::FROM_OTHER, ms_to_frames(ms, m_SampleRate));
}

void GOSoundVibratoProcessor::SetPitchRateChunk(
  const GOSoundVibratoPitchRateCurve::ChunkDescription &chunk) {
  assert(!chunk.HasRates() || chunk.nFrames == m_NFramesPerPeriod);
  m_PitchRateChunk = chunk;
}

std::unique_ptr<GOSoundVibratoProcessorState> GOSoundVibratoProcessor::
  CreateTypedState() const {
  assert(
    m_IsSetupCalled && "EnsureSetup() must be called before CreateState()");

  return std::make_unique<GOSoundVibratoProcessorState>(
    m_NChannels, m_NDelayBufferRingFrames);
}

const uint16_t *GOSoundVibratoProcessor::GetResamplerPositionIncrementsForUse(
  unsigned readPosFraction,
  unsigned nFramesAvailableToRead,
  unsigned nFramesAvailableToWrite,
  uint16_t *pTmpIncrements) const {
  const GOSoundVibratoPitchRateCurve::ChunkDescription &chunk
    = m_PitchRateChunk;
  const unsigned nFrames = m_NFramesPerPeriod;

  /* Both rescale targets are clamped to the mean rates that
   * normalisePitchRateCurve() accepts. The min target is the smallest sum that
   * reaches minNFramesToRead frames; the max one is the largest that stays
   * within maxNFramesToRead. Neither bound is in terms of the mean rate: the
   * min target can be far slower (with few frames to read), and the max one
   * faster (a chunk may hold cells at MAX_RATE_UNITS). */
  assert(nFramesAvailableToRead >= nFrames);
  assert(
    nFramesAvailableToRead + nFramesAvailableToWrite
      + GOSoundVibratoProcessorState::N_LOOKAHEAD_FRAMES
    == m_NDelayBufferRingFrames);
  assert(chunk.HasRates());
  assert(chunk.nFrames == nFrames);

  const uint16_t *pResult = chunk.pRateUnits;
  /* Both bounds are inclusive. At least nFrames must stay free for the next
   * period's write, so at least this many frames must be read. */
  const unsigned minNFramesToRead
    = nFramesAvailableToWrite < nFrames ? nFrames - nFramesAvailableToWrite : 0;
  // It is impossible to read more than is available
  const unsigned maxNFramesToRead = nFramesAvailableToRead;

  assert(minNFramesToRead <= maxNFramesToRead);

  /* Check where the read position would be after the chunk: first coarsely,
   * by the whole frames it advances; only on the frame max ahead exactly, by
   * the sum of the increments. If a bound is violated, take the sum to
   * rescale the chunk to. */
  const unsigned nFramesToReadWanted
    = GOSoundResample::getIndexIncrementByUnits(
      readPosFraction, chunk.nTotalUnits);
  // The sum to rescale the chunk to; 0 if no bound is violated
  uint64_t nUnitsToBeRead = 0;

  if (nFramesToReadWanted >= maxNFramesToRead) {
    /* The whole frames are not enough here: the final position on the frame
     * max ahead is allowed only with a zero fraction. The resampler reads a
     * frame before advancing, so the last read is one increment before the
     * final position. With a non-zero fraction the last read may still be on
     * the frame max ahead, whose taps are not written yet; with a zero
     * fraction it is strictly before it, since every increment is positive. */
    const uint64_t maxNUnitsToRead = GOSoundResample::computeMaxNUnitsToReach(
      readPosFraction, maxNFramesToRead);

    // Every increment is positive: the curve and the rescale keep them at least
    // MIN_RATE_UNITS
    static_assert(GOSoundVibratoPitchRateCurve::MIN_RATE_UNITS > 0);

    /* The max target is a mean rate only for a chunk of cells within the
     * allowed range; a chunk may hold cells at MAX_RATE_UNITS, so clamp it. */
    if (chunk.nTotalUnits > maxNUnitsToRead)
      nUnitsToBeRead
        = std::min(maxNUnitsToRead, m_MaxAllowedIncrementPerPeriod);
  } else if (nFramesToReadWanted < minNFramesToRead)
    /* The min target is sized for minNFramesToRead frames, not for nFrames
     * increments: a curve may hold MIN_RATE_UNITS or MIN_ALLOWED_RATE_UNITS -
     * 1, and then its mean lands below MIN_ALLOWED_RATE_UNITS. */
    nUnitsToBeRead = std::max(
      GOSoundResample::computeMinNUnitsToReach(
        readPosFraction, minNFramesToRead),
      m_MinAllowedIncrementPerPeriod);

  if (nUnitsToBeRead) {
    /* A violated bound is never 0 units: maxNFramesToRead >= nFrames > 0, and
     * minNFramesToRead > 0 here. Rescale a copy of the chunk so that its sum
     * lands on the violated bound. */
    std::copy(pResult, pResult + nFrames, pTmpIncrements);
    GOSoundVibratoPitchRateCurve::normalisePitchRateCurve(
      pTmpIncrements, nFrames, chunk.nTotalUnits, nUnitsToBeRead);
    pResult = pTmpIncrements;
  }
  return pResult;
}

void GOSoundVibratoProcessor::ReadFromDelayBuffer(
  GOSoundVibratoProcessorState &state,
  const uint16_t *pPosIncrements,
  GOSoundBufferPlanarMutable &outBuffer) const {
  using Ring = GOSoundResample::RingPlanarFrameVector<float, float>;

  const unsigned nFrames = outBuffer.GetNFrames();
  const unsigned nChannels = outBuffer.GetNChannels();

  Ring ring(
    state.m_DelayBufferRingWithTail.GetData(),
    nChannels,
    m_NDelayBufferRingFrames,
    GOSoundVibratoProcessorState::N_DELAY_BUFFER_TAIL_FRAMES);
  auto readWith = [&]<class ResamplerT>(ResamplerT resampler) {
    if (pPosIncrements)
      resampler.template ResampleBlockVariableRatePlanar<Ring>(
        state.m_DelayBufferReadResamplingPos,
        ring,
        pPosIncrements,
        nFrames,
        nChannels,
        outBuffer.GetData(),
        nFrames);
    else {
      // The frozen lag: the read position must advance as fast as the write one
      assert(state.m_DelayBufferReadResamplingPos.IsSameRate());
      resampler.template ResampleBlockConstantRatePlanar<Ring>(
        state.m_DelayBufferReadResamplingPos,
        ring,
        nFrames,
        nChannels,
        outBuffer.GetData(),
        nFrames);
    }
  };

  if (m_InterpolationType == GOSoundResample::GO_POLYPHASE_INTERPOLATION)
    readWith(GOSoundResample::PolyphaseResampler(r_resample));
  else
    readWith(GOSoundResample::LinearResampler(r_resample));
}

void GOSoundVibratoProcessor::CrossfadeWithDelayed(
  bool isToActivate,
  const GOSoundBufferPlanar &delayedBuffer,
  GOSoundVibratoProcessorState &state,
  GOSoundBufferPlanarMutable &inOutBuffer) const {
  using State = GOSoundVibratoProcessorState;

  const State::ActivatingPhase oldPhase = state.m_ActivatingPhase;
  // The phase during the crossfade
  const State::ActivatingPhase newPhase
    = isToActivate ? State::ACTIVATING : State::DEACTIVATING;
  // The steady phase after the crossfade
  const State::ActivatingPhase targetPhase
    = isToActivate ? State::ACTIVE : State::INACTIVE;
  const GOSoundLinearFaderCore::CrossfadeRamp &ramp
    = GetCrossfadeRamp(isToActivate);
  const unsigned nRampFrames = ramp.GetNRampFrames();

  assert(oldPhase != targetPhase);
  if (isSteadyPhase(oldPhase))
    // Starting a crossfade from a non-crossfading phase
    state.m_CrossfadePos = 0;
  else if (
    newPhase != oldPhase || nRampFrames != state.m_LastCrossfadeNFrames) {
    /* A crossfade was running in the previous period, but the direction or the
     * length of its ramp has changed (the length: by a setter, very rare). The
     * ramp it was counted in is rebuilt from the old direction (the old phase)
     * and the stored old length; the new position is the one with the same gain
     * in the new ramp (rounded to a frame, so the weights of the signals do
     * not jump) */
    const GOSoundLinearFaderCore::CrossfadeRamp oldRamp(
      oldPhase == State::ACTIVATING ? GOSoundLinearFaderCore::TO_OTHER
                                    : GOSoundLinearFaderCore::FROM_OTHER,
      state.m_LastCrossfadeNFrames);

    // Recalculating the position to the new ramp
    state.m_CrossfadePos = nRampFrames
      ? GOSoundLinearFaderCore::mapFrameOffset(
        oldRamp, state.m_CrossfadePos, ramp)
      : 0; // no frames: the crossfade is disabled, the switch is instant
  }
  state.m_ActivatingPhase = newPhase;
  state.m_LastCrossfadeNFrames = nRampFrames;
  GOSoundLinearFaderCore::crossfadePlanar(
    ramp, state.m_CrossfadePos, delayedBuffer, inOutBuffer);
  // May go past the ramp end
  state.m_CrossfadePos += inOutBuffer.GetNFrames();
  if (ramp.IsOverAt(state.m_CrossfadePos)) {
    state.m_ActivatingPhase = targetPhase;
    state.m_CrossfadePos = 0;
    state.m_LastCrossfadeNFrames = 0;
  }
  // The invariant for the next call: either a steady phase, or a crossfade
  // that is still inside the ramp it was counted in
  assert(
    isSteadyPhase(state.m_ActivatingPhase)
    || state.m_CrossfadePos < state.m_LastCrossfadeNFrames);
}

void GOSoundVibratoProcessor::Process(
  GOSoundVibratoProcessorState &state,
  GOSoundBufferPlanarMutable &buffer) const {
  using State = GOSoundVibratoProcessorState;

  const bool isToVibrate = m_PitchRateChunk.HasRates();

  /* Without rates and with no delayed signal heard (INACTIVE) - either the
   * organ has no pitch tremulant, or its tremulant is a purely amplitude one,
   * or it is switched off and faded out - there is nothing to do: the state is
   * already reset */
  if (isToVibrate || state.m_ActivatingPhase != State::INACTIVE) {
    const unsigned nFrames = m_NFramesPerPeriod;
    const unsigned nChannels = m_NChannels;

    // The chain guarantees the format of the last EnsureSetup()
    assert(buffer.GetNFrames() == nFrames);
    assert(buffer.GetNChannels() == nChannels);

    // Asserts there that the free room is enough. The buffer stays undelayed
    state.WriteToDelayBuffer(buffer);

    uint16_t tmpPosIncrements[nFrames]; // stack
    /* Without rates the lag is frozen: nullptr makes ReadFromDelayBuffer()
     * advance the read position as fast as the write one */
    const uint16_t *pPosIncrements = isToVibrate
      ? GetResamplerPositionIncrementsForUse(
        state.m_DelayBufferReadResamplingPos.GetFraction(),
        state.GetNFramesAvailableToRead(),
        state.GetNFramesAvailableToWrite(),
        tmpPosIncrements)
      : nullptr;

    if (isToVibrate && state.m_ActivatingPhase == State::ACTIVE)
      ReadFromDelayBuffer(state, pPosIncrements, buffer);
    else {
      /* A crossfade starts or continues: the delayed signal is read aside, so
       * that buffer keeps the undelayed one */
      GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(delayedBuffer, nChannels, nFrames);

      ReadFromDelayBuffer(state, pPosIncrements, delayedBuffer);
      CrossfadeWithDelayed(isToVibrate, delayedBuffer, state, buffer);
    }

    assert(state.GetNFramesAvailableToWrite() >= nFrames);
    if (state.m_ActivatingPhase == State::INACTIVE) {
      // Reachable only by a finished deactivation: a period with rates never
      // ends INACTIVE
      assert(!isToVibrate);
      // The delayed signal has faded out: the state must equal a fresh one
      state.Reset();
    }
  }
}
