/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundVibratoProcessor.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>
#include <vector>

#include "sound/buffer/GOSoundBufferPlanarManaged.h"
#include "sound/buffer/GOSoundBufferPlanarMutable.h"
#include "sound/dsp-kernels/GOSoundLinearFaderCore.h"
#include "sound/dsp-kernels/GOSoundResample.h"
#include "sound/effects/GOSoundVibratoPitchRateCurve.h"
#include "sound/effects/GOSoundVibratoProcessor.h"
#include "sound/effects/GOSoundVibratoProcessorState.h"
#include "sound/processing/GOSoundProcessor.h"

#include "../GOSoundTestSignals.h"
#include "GOTestScope.h"

const std::string GOTestSoundVibratoProcessor::TEST_NAME
  = "GOTestSoundVibratoProcessor";

using Curve = GOSoundVibratoPitchRateCurve;
using State = GOSoundVibratoProcessorState;

static constexpr unsigned TEST_SAMPLE_RATE = 48000;
static constexpr unsigned UNIT_RATE = GOSoundResample::UPSAMPLE_FACTOR;
static constexpr unsigned L = State::N_LOOKAHEAD_FRAMES;

namespace {
/**
 * Plays the part of the owner of the loop cursor (the tremulant): walks a
 * curve with its own LoopPosition, gives the processor the chunk at the cursor
 * (or the chunk with no rates) before every period and advances the cursor
 * after it.
 */
class ChunkFeeder {
private:
  GOSoundVibratoProcessor &r_processor;
  const Curve *p_curve = nullptr;
  Curve::LoopPosition m_pos;

public:
  /**
   * @param processor the processor to feed
   * @param pCurve the curve to walk, or nullptr for the chunks with no rates
   */
  ChunkFeeder(GOSoundVibratoProcessor &processor, const Curve *pCurve = nullptr)
    : r_processor(processor) {
    SetCurve(pCurve);
  }

  /**
   * Another curve starts from the start of its loop, as a new cursor does; the
   * same curve is left as it is.
   */
  void SetCurve(const Curve *pCurve) {
    if (pCurve != p_curve || (pCurve && !m_pos.IsValid())) {
      p_curve = pCurve;
      m_pos = pCurve ? Curve::LoopPosition(pCurve->GetNLoopFrames())
                     : Curve::LoopPosition();
    }
  }

  /** Returns the cursor to the start of the loop */
  void Rewind() { m_pos.Reset(); }

  /** Sets the chunk of the next period */
  void BeginPeriod() {
    r_processor.SetPitchRateChunk(
      p_curve ? p_curve->GetChunkDescription(m_pos)
              : Curve::ChunkDescription{nullptr, 0, 0});
  }

  /** Moves the cursor past the period */
  void EndPeriod(unsigned nFrames) {
    if (p_curve)
      m_pos.Advance(nFrames);
  }

  GOSoundVibratoProcessor &GetProcessor() { return r_processor; }
};
} // namespace

void GOTestSoundVibratoProcessor::TestStateConstructor() {
  static_assert(State::N_LOOKAHEAD_FRAMES == GOSoundResample::MAX_POINTS - 1);
  static_assert(State::N_DELAY_BUFFER_TAIL_FRAMES == State::N_LOOKAHEAD_FRAMES);

  const unsigned ring = 30;
  State state(2, ring);

  GOAssert(
    state.m_DelayBufferRingWithTail.GetNChannels() == 2
      && state.m_DelayBufferRingWithTail.GetNFrames()
        == ring + State::N_DELAY_BUFFER_TAIL_FRAMES,
    "the buffer must have the ring and the tail for every channel");
  GOAssert(
    state.m_DelayBufferWritePos == L
      && state.m_DelayBufferReadResamplingPos.GetIndex() == 0
      && state.m_DelayBufferReadResamplingPos.GetFraction() == 0,
    "a new state starts with W = L and R = 0");
  GOAssert(state.GetNFramesAvailableToRead() == 0, "nothing to read yet");
  GOAssert(
    state.GetNFramesAvailableToWrite() == ring - L, "the room is ring - L");
}

void GOTestSoundVibratoProcessor::TestStateAvailableFrames() {
  const unsigned ring = 20;
  State state(1, ring);

  struct Row {
    unsigned writePos;
    unsigned readIndex;
    unsigned expectedToWrite;
    unsigned expectedToRead;
  };
  const Row rows[] = {
    {19, 12, 13, 0}, // the start: the maximum room
    {4, 12, 8, 5},   // after a write of 5 with a wrap
    {4, 17, 13, 0},  // after a read of 5
    {9, 9, 0, 13},   // the full ring
  };

  for (const Row &row : rows) {
    state.m_DelayBufferWritePos = row.writePos;
    state.m_DelayBufferReadResamplingPos.Init(1.0f, row.readIndex);
    GOAssert(
      state.GetNFramesAvailableToWrite() == row.expectedToWrite,
      "wrong GetNFramesAvailableToWrite()");
    GOAssert(
      state.GetNFramesAvailableToRead() == row.expectedToRead,
      "wrong GetNFramesAvailableToRead()");
    GOAssert(
      state.GetNFramesAvailableToRead() + L + state.GetNFramesAvailableToWrite()
        == ring,
      "AvailToRead + L + AvailToWrite must be the ring");
  }
}

void GOTestSoundVibratoProcessor::TestStateWriteToDelayBuffer() {
  const unsigned ring = 20;
  const unsigned nChannels = 2;
  State state(nChannels, ring);
  std::mt19937 rng(7);
  // The model of the ring and which of the first L slots were written
  std::vector<std::vector<float>> model(
    nChannels, std::vector<float>(ring, 0.0f));
  std::vector<bool> isWritten(L, false);
  float nextValue = 1.0f;

  // Slots [0, L) are zero from the reset, so they count as written
  std::fill(isWritten.begin(), isWritten.end(), true);
  for (unsigned iterI = 0; iterI < 500; ++iterI) {
    const unsigned nFrames = 1 + rng() % 13;
    const unsigned writePos = state.m_DelayBufferWritePos;

    // Make the room for the write by placing the read index behind it
    state.m_DelayBufferReadResamplingPos.Init(
      1.0f, (writePos + (ring - L)) % ring);

    GOSoundBufferPlanarManaged src(nChannels, nFrames);

    for (unsigned channelI = 0; channelI < nChannels; ++channelI)
      for (unsigned frameI = 0; frameI < nFrames; ++frameI)
        src.GetChannelBuffer(channelI).GetData()[frameI] = nextValue++;
    state.WriteToDelayBuffer(src);
    for (unsigned frameI = 0; frameI < nFrames; ++frameI) {
      const unsigned slot = (writePos + frameI) % ring;

      for (unsigned channelI = 0; channelI < nChannels; ++channelI)
        model[channelI][slot]
          = src.GetChannelBuffer(channelI).GetData()[frameI];
      if (slot < L)
        isWritten[slot] = true;
    }
    GOAssert(
      state.m_DelayBufferWritePos == (writePos + nFrames) % ring,
      "the write position must advance by nFrames modulo the ring");
    for (unsigned channelI = 0; channelI < nChannels; ++channelI) {
      const float *pData
        = state.m_DelayBufferRingWithTail.GetChannelBuffer(channelI).GetData();

      for (unsigned slotI = 0; slotI < ring; ++slotI)
        GOAssert(
          pData[slotI] == model[channelI][slotI],
          "the ring content must match the model");
      for (unsigned tailI = 0; tailI < L; ++tailI)
        if (isWritten[tailI])
          GOAssert(
            pData[ring + tailI] == pData[tailI],
            "the tail must mirror the start of the ring");
    }
  }
}

void GOTestSoundVibratoProcessor::TestStateReset() {
  const unsigned ring = 30;
  const unsigned nChannels = 2;
  State state(nChannels, ring);
  const float garbage = 12345.0f;

  // Fill the whole buffer, including the tail, with garbage
  for (unsigned channelI = 0; channelI < nChannels; ++channelI) {
    float *pData
      = state.m_DelayBufferRingWithTail.GetChannelBuffer(channelI).GetData();

    std::fill(pData, pData + ring + State::N_DELAY_BUFFER_TAIL_FRAMES, garbage);
  }
  state.m_DelayBufferWritePos = 5;
  state.m_DelayBufferReadResamplingPos.Init(1.0f, 11);
  state.m_DelayBufferReadResamplingPos.Inc(100);
  state.Reset();
  GOAssert(
    state.m_DelayBufferWritePos == L
      && state.m_DelayBufferReadResamplingPos.GetIndex() == 0
      && state.m_DelayBufferReadResamplingPos.GetFraction() == 0,
    "the reset puts W = L and R = 0");
  GOAssert(state.GetNFramesAvailableToRead() == 0, "AvailToRead is 0");
  for (unsigned channelI = 0; channelI < nChannels; ++channelI) {
    const float *pData
      = state.m_DelayBufferRingWithTail.GetChannelBuffer(channelI).GetData();

    for (unsigned frameI = 0; frameI < L; ++frameI)
      GOAssert(pData[frameI] == 0.0f, "the first L frames must be zeroed");
    GOAssert(pData[L] == garbage, "the rest of the ring must not be cleared");
    GOAssert(
      pData[ring + 1] == garbage, "the tail must not be cleared by a reset");
  }

  // A reset of a reset state is harmless
  state.Reset();
  GOAssert(
    state.m_DelayBufferWritePos == L
      && state.m_DelayBufferReadResamplingPos.GetIndex() == 0
      && state.GetNFramesAvailableToRead() == 0,
    "a repeated Reset() must be harmless");
}

void GOTestSoundVibratoProcessor::TestEnsureSetup() {
  GOSoundResample resample;
  GOSoundVibratoProcessor processor(resample);

  // The maximum lag is 100 ms
  for (const unsigned sampleRate : {44100u, 48000u, 96000u}) {
    processor.EnsureSetup(2, 64, sampleRate);
    GOAssert(
      processor.m_NDelayBufferRingFrames == sampleRate / 10 + 64 + L,
      "the ring must cover the maximum lag, a period and the look-ahead");
  }
  processor.EnsureSetup(1, 20, TEST_SAMPLE_RATE, 0, 0, 0);
  GOAssert(
    processor.m_NDelayBufferRingFrames == 20 + L,
    "with no extra lag the ring is a period plus the look-ahead");

  // The maximum lag is a parameter of the constructor
  for (const unsigned maxLagMs : {0u, 50u, 200u}) {
    GOSoundVibratoProcessor lagProcessor(
      resample, GOSoundResample::GO_LINEAR_INTERPOLATION, maxLagMs);

    lagProcessor.EnsureSetup(1, 64, 48000);
    GOAssert(
      lagProcessor.m_NDelayBufferRingFrames == maxLagMs * 48 + 64 + L,
      "the ring must cover the maximum lag given to the constructor");
  }

  // A repeated call resets the chunk to "no rates"
  const std::vector<float> deviations(20, 0.0f);
  Curve curve;

  curve.Build(deviations.data(), 20, 20);
  processor.SetPitchRateChunk(
    curve.GetChunkDescription(Curve::LoopPosition(20)));
  GOAssert(processor.m_PitchRateChunk.HasRates(), "the chunk must be set");
  processor.EnsureSetup(1, 20, TEST_SAMPLE_RATE, 0, 0, 0);
  GOAssert(
    !processor.m_PitchRateChunk.HasRates()
      && processor.m_PitchRateChunk.nFrames == 0
      && processor.m_PitchRateChunk.nTotalUnits == 0,
    "EnsureSetup() must reset the chunk to {nullptr, 0, 0}");
}

void GOTestSoundVibratoProcessor::TestStatePhase() {
  State state(1, 30);

  GOAssert(
    state.m_ActivatingPhase == State::INACTIVE && state.m_CrossfadePos == 0,
    "a new state is INACTIVE with the crossfade position 0");

  // A reset ends a crossfade
  state.m_ActivatingPhase = State::DEACTIVATING;
  state.m_CrossfadePos = 5;
  state.m_LastCrossfadeNFrames = 7;
  state.Reset();
  GOAssert(
    state.m_ActivatingPhase == State::INACTIVE && state.m_CrossfadePos == 0
      && state.m_LastCrossfadeNFrames == 0,
    "Reset() must end a crossfade");

  // and makes an ACTIVE state INACTIVE
  state.m_ActivatingPhase = State::ACTIVE;
  state.Reset();
  GOAssert(
    state.m_ActivatingPhase == State::INACTIVE && state.m_CrossfadePos == 0,
    "Reset() must make the state INACTIVE");

  GOAssert(
    GOSoundVibratoProcessor::isSteadyPhase(State::INACTIVE)
      && GOSoundVibratoProcessor::isSteadyPhase(State::ACTIVE)
      && !GOSoundVibratoProcessor::isSteadyPhase(State::ACTIVATING)
      && !GOSoundVibratoProcessor::isSteadyPhase(State::DEACTIVATING),
    "isSteadyPhase() must be true only for INACTIVE and ACTIVE");
}

/**
 * @return whether the ramp has exactly nFrames frames
 */
static bool has_ramp_length(
  const GOSoundLinearFaderCore::CrossfadeRamp &ramp, unsigned nFrames) {
  return ramp.IsOverAt(nFrames)
    && (nFrames == 0 || !ramp.IsOverAt(nFrames - 1));
}

void GOTestSoundVibratoProcessor::TestEnsureSetupCrossfadeRamps() {
  using Core = GOSoundLinearFaderCore;

  GOSoundResample resample;
  GOSoundVibratoProcessor processor(resample);

  // The defaults: 3 ms and 10 ms, in whole frames (a fraction is rounded up:
  // 3 ms at 44100 Hz is 132.3 frames)
  struct Row {
    unsigned sampleRate;
    unsigned nActivationFrames;
    unsigned nDeactivationFrames;
  };
  const Row rows[] = {{44100, 133, 441}, {48000, 144, 480}, {96000, 288, 960}};

  for (const Row &row : rows) {
    processor.EnsureSetup(2, 64, row.sampleRate);
    GOAssert(
      has_ramp_length(processor.m_ActivationRamp, row.nActivationFrames),
      "the activation ramp must have the default length");
    GOAssert(
      has_ramp_length(processor.m_DeactivationRamp, row.nDeactivationFrames),
      "the deactivation ramp must have the default length");
  }
  processor.EnsureSetup(2, 64, 48000);

  // The directions: the weight of the undelayed signal falls on activation
  // and grows on deactivation
  GOAssert(
    processor.m_ActivationRamp.GetGainAt(0) == Core::GAIN_ONE
      && processor.m_ActivationRamp.GetGainAt(144) == Core::GAIN_ZERO,
    "the activation ramp must fall from 1 to 0");
  GOAssert(
    processor.m_DeactivationRamp.GetGainAt(0) == Core::GAIN_ZERO
      && processor.m_DeactivationRamp.GetGainAt(480) == Core::GAIN_ONE,
    "the deactivation ramp must grow from 0 to 1");

  // Explicit lengths in milliseconds, set before EnsureSetup()
  processor.SetActivationCrossfadeMs(20);
  processor.SetDeactivationCrossfadeMs(100);
  processor.EnsureSetup(2, 64, 48000);
  GOAssert(
    has_ramp_length(processor.m_ActivationRamp, 960)
      && has_ramp_length(processor.m_DeactivationRamp, 4800),
    "explicit lengths must be converted from milliseconds");

  // Set after EnsureSetup(): the ramps are rebuilt at once; the maximum
  // length at 96 kHz
  processor.EnsureSetup(2, 64, 96000);
  processor.SetActivationCrossfadeMs(GOSoundVibratoProcessor::MAX_CROSSFADE_MS);
  processor.SetDeactivationCrossfadeMs(
    GOSoundVibratoProcessor::MAX_CROSSFADE_MS);
  GOAssert(
    has_ramp_length(processor.m_ActivationRamp, 10 * 96000)
      && has_ramp_length(processor.m_DeactivationRamp, 10 * 96000),
    "the maximum length must be converted exactly");
  processor.SetActivationCrossfadeMs(5);
  GOAssert(
    has_ramp_length(processor.m_ActivationRamp, 480)
      && has_ramp_length(processor.m_DeactivationRamp, 10 * 96000),
    "a setter must rebuild only its own ramp");

  // A fraction of a frame is rounded up: 1 ms at 1500 Hz is 1.5 frames
  processor.EnsureSetup(2, 64, 1500);
  processor.SetActivationCrossfadeMs(1);
  processor.SetDeactivationCrossfadeMs(3);
  GOAssert(
    has_ramp_length(processor.m_ActivationRamp, 2)
      && has_ramp_length(processor.m_DeactivationRamp, 5),
    "a fraction of a frame is rounded up");

  // Explicit lengths in frames
  processor.EnsureSetup(2, 64, 48000, 0, 7, 9);
  GOAssert(
    has_ramp_length(processor.m_ActivationRamp, 7)
      && has_ramp_length(processor.m_DeactivationRamp, 9),
    "explicit lengths in frames must be taken as they are");
  GOAssert(
    processor.m_NDelayBufferRingFrames == 64 + L,
    "the maximum lag in frames must be taken as it is");

  // Disabled: constant-gain ramps with the end gains
  processor.SetActivationCrossfadeMs(0);
  processor.SetDeactivationCrossfadeMs(0);
  processor.EnsureSetup(2, 64, 48000);
  GOAssert(
    has_ramp_length(processor.m_ActivationRamp, 0)
      && processor.m_ActivationRamp.GetGainAt(0) == Core::GAIN_ZERO
      && processor.m_ActivationRamp.GetGainAt(1000) == Core::GAIN_ZERO,
    "a disabled activation is the constant 0: only the delayed signal");
  GOAssert(
    has_ramp_length(processor.m_DeactivationRamp, 0)
      && processor.m_DeactivationRamp.GetGainAt(0) == Core::GAIN_ONE
      && processor.m_DeactivationRamp.GetGainAt(1000) == Core::GAIN_ONE,
    "a disabled deactivation is the constant 1: only the undelayed signal");
}

void GOTestSoundVibratoProcessor::TestSetPitchRateChunk() {
  GOSoundResample resample;
  GOSoundVibratoProcessor processor(resample);
  const std::vector<float> deviations(20, 0.0f);
  Curve curve;
  Curve::LoopPosition pos(20);

  processor.EnsureSetup(1, 20, TEST_SAMPLE_RATE, 0, 0, 0);
  curve.Build(deviations.data(), 20, 20);

  const Curve::ChunkDescription chunk = curve.GetChunkDescription(pos);

  processor.SetPitchRateChunk(chunk);
  GOAssert(
    processor.m_PitchRateChunk.pRateUnits == chunk.pRateUnits
      && processor.m_PitchRateChunk.nFrames == 20
      && processor.m_PitchRateChunk.nTotalUnits == chunk.nTotalUnits,
    "the three fields of the chunk must be stored");

  // The next position gives another chunk
  pos.Advance(5);

  const Curve::ChunkDescription nextChunk = curve.GetChunkDescription(pos);

  processor.SetPitchRateChunk(nextChunk);
  GOAssert(
    processor.m_PitchRateChunk.pRateUnits == nextChunk.pRateUnits,
    "a new chunk must replace the old one");

  processor.SetPitchRateChunk({nullptr, 0, 0});
  GOAssert(
    !processor.m_PitchRateChunk.HasRates(),
    "the chunk with no rates must bypass");
}

void GOTestSoundVibratoProcessor::TestCreateTypedState() {
  GOSoundResample resample;
  GOSoundVibratoProcessor processor(resample);

  processor.EnsureSetup(2, 64, TEST_SAMPLE_RATE);

  const std::unique_ptr<State> oState = processor.CreateTypedState();

  GOAssert(
    oState->m_NDelayBufferRingFrames == processor.m_NDelayBufferRingFrames
      && oState->m_DelayBufferRingWithTail.GetNChannels() == 2
      && oState->m_DelayBufferRingWithTail.GetNFrames()
        == processor.m_NDelayBufferRingFrames
          + State::N_DELAY_BUFFER_TAIL_FRAMES,
    "the state must be sized for the last EnsureSetup()");
  GOAssert(oState->m_DelayBufferWritePos == L, "the state must be primed");
}

/**
 * Makes nFrames increments whose sum is exactly totalUnits (spread evenly).
 */
static std::vector<uint16_t> make_increments(
  unsigned nFrames, uint64_t totalUnits) {
  std::vector<uint16_t> incs(nFrames, (uint16_t)(totalUnits / nFrames));
  const unsigned nExtra = (unsigned)(totalUnits % nFrames);

  for (unsigned incI = 0; incI < nExtra; ++incI)
    ++incs[incI];
  return incs;
}

static uint64_t sum_of(const uint16_t *pData, unsigned n) {
  return std::accumulate(pData, pData + n, (uint64_t)0);
}

void GOTestSoundVibratoProcessor::TestIncrementsZeroWidthWindow() {
  const unsigned nFrames = 20;
  GOSoundResample resample;
  GOSoundVibratoProcessor processor(resample);

  // No extra lag: the window is of zero width: min == max == nFrames
  processor.EnsureSetup(1, nFrames, TEST_SAMPLE_RATE, 0, 0, 0);

  const unsigned availRead = nFrames;
  const unsigned availWrite = 0;

  for (const unsigned fraction : {0u, UNIT_RATE - 1}) {
    const uint64_t bound
      = GOSoundResample::computeMaxNUnitsToReach(fraction, nFrames);

    /* Exactly on the bound, one above and one below. One above is rescaled:
     * a non-zero fraction on the frame max ahead is not allowed */
    for (const int64_t delta : {-1, 0, 1}) {
      const bool isRescaled = delta != 0;
      const std::vector<uint16_t> chunkIncs
        = make_increments(nFrames, bound + delta);
      uint16_t tmp[nFrames];

      processor.SetPitchRateChunk(
        {chunkIncs.data(), nFrames, (unsigned)(bound + delta)});

      const uint16_t *pResult = processor.GetResamplerPositionIncrementsForUse(
        fraction, availRead, availWrite, tmp);

      GOAssert(
        sum_of(chunkIncs.data(), nFrames) == bound + delta,
        "the chunk must stay unmodified");
      if (isRescaled) {
        GOAssert(pResult == tmp, "a sum off the bound must be rescaled");
        GOAssert(
          sum_of(tmp, nFrames) == bound,
          "the rescaled sum must land exactly on the bound");
        for (unsigned frameI = 0; frameI < nFrames; ++frameI)
          GOAssert(
            tmp[frameI] >= Curve::MIN_RATE_UNITS
              && tmp[frameI] <= Curve::MAX_RATE_UNITS,
            "every rescaled rate must be in the range");
      } else
        GOAssert(
          pResult == chunkIncs.data(), "a sum on the bound is used as is");
    }
  }

  // 20 rates of 9216 (a whole frame and two over the bound) become exactly
  // 8192 each (the sum 163840)
  const std::vector<uint16_t> chunkIncs(nFrames, UNIT_RATE + UNIT_RATE / 8);
  uint16_t tmp[nFrames];

  processor.SetPitchRateChunk(
    {chunkIncs.data(), nFrames, (unsigned)sum_of(chunkIncs.data(), nFrames)});
  processor.GetResamplerPositionIncrementsForUse(0, availRead, availWrite, tmp);
  for (unsigned frameI = 0; frameI < nFrames; ++frameI)
    GOAssert(
      tmp[frameI] == UNIT_RATE, "every output rate must be exactly 8192");
}

void GOTestSoundVibratoProcessor::TestIncrementsWiderWindow() {
  const unsigned nFrames = 20;
  GOSoundResample resample;
  GOSoundVibratoProcessor processor(resample);

  // 10 extra frames of lag: ring = 20 + 10 + L
  processor.EnsureSetup(1, nFrames, TEST_SAMPLE_RATE, 10, 0, 0);

  // avail read + avail write == ring - L == 30
  const unsigned availRead = 25;
  const unsigned availWrite = 5;
  const unsigned minToRead = nFrames - availWrite; // 15
  const unsigned maxToRead = availRead;            // 25

  for (const unsigned fraction : {0u, 1u, UNIT_RATE - 1}) {
    const uint64_t minUnits
      = GOSoundResample::computeMinNUnitsToReach(fraction, minToRead);
    const uint64_t maxUnits
      = GOSoundResample::computeMaxNUnitsToReach(fraction, maxToRead);

    struct Case {
      uint64_t units;
      bool isRescaled;
      uint64_t expectedUnits; // when rescaled
    };
    const Case cases[] = {
      {minUnits, false, 0}, // exactly the lower bound: valid
      {minUnits - 1, true, minUnits},
      {maxUnits - 1, false, 0}, // just inside
      {maxUnits, false, 0},     // on the bound with a zero fraction: valid
      // on the frame max ahead with a non-zero fraction: rescaled
      {maxUnits + 1, true, maxUnits},
    };

    for (const Case &testCase : cases) {
      const std::vector<uint16_t> chunkIncs
        = make_increments(nFrames, testCase.units);
      uint16_t tmp[nFrames];

      processor.SetPitchRateChunk(
        {chunkIncs.data(), nFrames, (unsigned)testCase.units});

      const uint16_t *pResult = processor.GetResamplerPositionIncrementsForUse(
        fraction, availRead, availWrite, tmp);

      if (testCase.isRescaled) {
        GOAssert(pResult == tmp, "an out-of-window chunk must be rescaled");
        GOAssert(
          sum_of(tmp, nFrames) == testCase.expectedUnits,
          "the rescaled sum must land exactly on the violated bound");
      } else
        GOAssert(pResult == chunkIncs.data(), "a valid chunk is used as is");
    }
  }

  // min == 0: a tiny sum is never rescaled
  processor.EnsureSetup(1, nFrames, TEST_SAMPLE_RATE, 20, 0, 0);

  const std::vector<uint16_t> slow(nFrames, Curve::MIN_RATE_UNITS);
  uint16_t tmp[nFrames];

  processor.SetPitchRateChunk(
    {slow.data(), nFrames, (unsigned)sum_of(slow.data(), nFrames)});
  GOAssert(
    processor.GetResamplerPositionIncrementsForUse(0, 20, 20, tmp)
      == slow.data(),
    "with min == 0 a slow chunk must be used as is");
}

void GOTestSoundVibratoProcessor::TestIncrementsLowerMeanBound() {
  const unsigned nFrames = 64;
  GOSoundResample resample;
  GOSoundVibratoProcessor processor(resample);

  // ring = 64 + 1024 + L: avail read + avail write == 1088
  processor.EnsureSetup(1, nFrames, TEST_SAMPLE_RATE, 1024, 0, 0);

  const unsigned availWrite = 51;
  const unsigned availRead = nFrames + 1024 - availWrite; // 1037
  const unsigned minToRead = nFrames - availWrite;        // 13
  const unsigned fraction = 1599;
  /* A normalised curve may hold cells at MIN_ALLOWED_RATE_UNITS - 1: the
   * remainder step lowers a pressed extreme by 1. The min target of such a
   * chunk has the mean 1639, below MIN_ALLOWED_RATE_UNITS */
  const uint64_t chunkUnits
    = (uint64_t)nFrames * (Curve::MIN_ALLOWED_RATE_UNITS - 1); // 104896
  const std::vector<uint16_t> chunkIncs = make_increments(nFrames, chunkUnits);
  const uint64_t expectedUnits
    = (uint64_t)nFrames * Curve::MIN_ALLOWED_RATE_UNITS; // 104960
  uint16_t tmp[nFrames];

  GOAssert(
    GOSoundResample::getIndexIncrementByUnits(fraction, chunkUnits) < minToRead,
    "the chunk must violate the lower bound");

  processor.SetPitchRateChunk(
    {chunkIncs.data(), nFrames, (unsigned)chunkUnits});

  const uint16_t *pResult = processor.GetResamplerPositionIncrementsForUse(
    fraction, availRead, availWrite, tmp);

  GOAssert(pResult == tmp, "the chunk must be rescaled");
  GOAssert(
    sum_of(tmp, nFrames) == expectedUnits,
    "the rescaled sum must be raised to the slowest allowed mean");
  GOAssert(
    GOSoundResample::getIndexIncrementByUnits(fraction, expectedUnits)
      == minToRead,
    "the rescaled sum must still read the required number of frames");
  for (unsigned frameI = 0; frameI < nFrames; ++frameI)
    GOAssert(
      tmp[frameI] >= Curve::MIN_RATE_UNITS
        && tmp[frameI] <= Curve::MAX_RATE_UNITS,
      "every rescaled rate must be in the range");
}

void GOTestSoundVibratoProcessor::TestIncrementsUpperMeanBound() {
  const unsigned nFrames = 64;
  GOSoundResample resample;
  GOSoundVibratoProcessor processor(resample);

  // ring = 64 + 1024 + L: avail read + avail write == 1088
  processor.EnsureSetup(1, nFrames, TEST_SAMPLE_RATE, 1024, 0, 0);

  /* The max target's mean is (availRead * 8192 - fraction) / nFrames, so it
   * exceeds the band only at availRead == 5 * nFrames, and only a chunk of the
   * maximal cells is above that target then */
  const unsigned availRead = 5 * nFrames;                 // 320
  const unsigned availWrite = nFrames + 1024 - availRead; // 768
  const unsigned fraction = 1;
  const uint64_t chunkUnits = (uint64_t)nFrames * Curve::MAX_RATE_UNITS;
  const std::vector<uint16_t> chunkIncs = make_increments(nFrames, chunkUnits);
  const uint64_t expectedUnits
    = (uint64_t)nFrames * Curve::MAX_ALLOWED_RATE_UNITS; // 2621312
  uint16_t tmp[nFrames];

  GOAssert(
    GOSoundResample::getIndexIncrementByUnits(fraction, chunkUnits)
      >= availRead,
    "the chunk must violate the upper bound");

  processor.SetPitchRateChunk(
    {chunkIncs.data(), nFrames, (unsigned)chunkUnits});

  const uint16_t *pResult = processor.GetResamplerPositionIncrementsForUse(
    fraction, availRead, availWrite, tmp);

  GOAssert(pResult == tmp, "the chunk must be rescaled");
  GOAssert(
    sum_of(tmp, nFrames) == expectedUnits,
    "the rescaled sum must be capped to the fastest allowed mean");
  GOAssert(
    GOSoundResample::getIndexIncrementByUnits(fraction, expectedUnits)
      < availRead,
    "the rescaled sum must stay within the available frames");
  for (unsigned frameI = 0; frameI < nFrames; ++frameI)
    GOAssert(
      tmp[frameI] >= Curve::MIN_RATE_UNITS
        && tmp[frameI] <= Curve::MAX_RATE_UNITS,
      "every rescaled rate must be in the range");
}

void GOTestSoundVibratoProcessor::TestIncrementsRandom() {
  const unsigned nFrames = 20;
  GOSoundResample resample;
  GOSoundVibratoProcessor processor(resample);
  std::mt19937 rng(99);

  for (const unsigned nMaxLag : {0u, 3u, 10u, 30u}) {
    processor.EnsureSetup(1, nFrames, TEST_SAMPLE_RATE, nMaxLag, 0, 0);

    const unsigned ringMinusL = nFrames + nMaxLag;

    for (unsigned iterI = 0; iterI < 2000; ++iterI) {
      const unsigned availRead = nFrames + rng() % (nMaxLag + 1);
      const unsigned availWrite = ringMinusL - availRead;
      const unsigned fraction = rng() % UNIT_RATE;
      std::vector<uint16_t> chunkIncs(nFrames);

      for (uint16_t &inc : chunkIncs)
        inc
          = (uint16_t)(Curve::MIN_RATE_UNITS + rng() % (Curve::MAX_ALLOWED_RATE_UNITS - Curve::MIN_RATE_UNITS));

      const Curve::ChunkDescription chunk = {
        chunkIncs.data(), nFrames, (unsigned)sum_of(chunkIncs.data(), nFrames)};
      uint16_t tmp[nFrames];

      processor.SetPitchRateChunk(chunk);

      const uint16_t *pResult = processor.GetResamplerPositionIncrementsForUse(
        fraction, availRead, availWrite, tmp);
      const uint64_t resultUnits = sum_of(pResult, nFrames);
      const unsigned minToRead
        = availWrite < nFrames ? nFrames - availWrite : 0;
      const uint64_t minUnits
        = GOSoundResample::computeMinNUnitsToReach(fraction, minToRead);
      const uint64_t maxUnits
        = GOSoundResample::computeMaxNUnitsToReach(fraction, availRead);
      // The lower target is raised if its mean would be below MIN_ALLOWED
      const uint64_t expectedMinUnits
        = std::max(minUnits, (uint64_t)nFrames * Curve::MIN_ALLOWED_RATE_UNITS);

      GOAssert(
        resultUnits >= minUnits && resultUnits <= maxUnits,
        "the result must always lie inside the window of valid sums");
      // What the upper bound is for: every tap of the last read is written
      GOAssert(
        GOSoundResample::getIndexIncrementByUnits(
          fraction, resultUnits - pResult[nFrames - 1])
          < availRead,
        "the last read must be before the frame max ahead");
      if (pResult == chunkIncs.data())
        GOAssert(
          resultUnits == chunk.nTotalUnits, "an unchanged chunk keeps its sum");
      else
        GOAssert(
          resultUnits == expectedMinUnits || resultUnits == maxUnits,
          "a rescaled chunk must land exactly on a bound");
      for (unsigned frameI = 0; frameI < nFrames; ++frameI)
        GOAssert(
          pResult[frameI] >= Curve::MIN_RATE_UNITS
            && pResult[frameI] <= Curve::MAX_RATE_UNITS,
          "every increment must be in the range");
    }
  }
}

void GOTestSoundVibratoProcessor::TestReadFromDelayBuffer() {
  const unsigned nFrames = 20;
  GOSoundResample resample;

  for (const GOSoundResample::InterpolationType type :
       {GOSoundResample::GO_LINEAR_INTERPOLATION,
        GOSoundResample::GO_POLYPHASE_INTERPOLATION}) {
    GOSoundVibratoProcessor processor(resample, type);

    processor.EnsureSetup(1, nFrames, TEST_SAMPLE_RATE, 0, 0, 0);

    const unsigned ring = processor.m_NDelayBufferRingFrames;
    std::unique_ptr<State> oState = processor.CreateTypedState();
    float *pData
      = oState->m_DelayBufferRingWithTail.GetChannelBuffer(0).GetData();

    // A ramp in the ring and the mirrored tail
    for (unsigned slotI = 0; slotI < ring; ++slotI)
      pData[slotI] = (float)(slotI + 1);
    for (unsigned tailI = 0; tailI < L; ++tailI)
      pData[ring + tailI] = pData[tailI];

    // The read position near the end of the ring: the window crosses the seam
    const unsigned startIndex = ring - 5;

    oState->m_DelayBufferReadResamplingPos.Init(1.0f, startIndex);

    const std::vector<uint16_t> incs(nFrames, UNIT_RATE);
    GOSoundBufferPlanarManaged out(1, nFrames);

    processor.ReadFromDelayBuffer(*oState, incs.data(), out);

    GOAssert(
      oState->m_DelayBufferReadResamplingPos.GetIndex()
        == (startIndex + nFrames) % ring,
      "the read index must advance by the summed increments modulo the ring");
    GOAssert(
      oState->m_DelayBufferReadResamplingPos.GetFraction() == 0,
      "the fraction must stay 0");

    const float *pOut = out.GetChannelBuffer(0).GetData();

    if (type == GOSoundResample::GO_LINEAR_INTERPOLATION)
      for (unsigned frameI = 0; frameI < nFrames; ++frameI)
        GOAssert(
          pOut[frameI] == (float)((startIndex + frameI) % ring + 1),
          "linear: the output must be the ring read from the start index");
    else
      for (unsigned frameI = 0; frameI < nFrames; ++frameI)
        GOAssert(std::isfinite(pOut[frameI]), "polyphase: finite output");
  }
}

void GOTestSoundVibratoProcessor::TestReadFromDelayBufferFrozen() {
  const unsigned nFrames = 20;
  const unsigned startFraction = 1234;
  GOSoundResample resample;

  for (const GOSoundResample::InterpolationType type :
       {GOSoundResample::GO_LINEAR_INTERPOLATION,
        GOSoundResample::GO_POLYPHASE_INTERPOLATION})
    for (const unsigned nChannels : {1u, 2u}) {
      GOSoundVibratoProcessor processor(resample, type);

      processor.EnsureSetup(nChannels, nFrames, TEST_SAMPLE_RATE, 0, 0, 0);

      const unsigned ring = processor.m_NDelayBufferRingFrames;
      const unsigned startIndex = ring - 5;
      std::unique_ptr<State> oFrozen = processor.CreateTypedState();
      std::unique_ptr<State> oUnit = processor.CreateTypedState();

      // The same content with the mirrored tail and the same position with a
      // fraction
      for (State *pState : {oFrozen.get(), oUnit.get()}) {
        for (unsigned channelI = 0; channelI < nChannels; ++channelI) {
          float *pData
            = pState->m_DelayBufferRingWithTail.GetChannelBuffer(channelI)
                .GetData();

          for (unsigned slotI = 0; slotI < ring; ++slotI)
            pData[slotI] = std::sin(0.37f * slotI + channelI);
          for (unsigned tailI = 0; tailI < L; ++tailI)
            pData[ring + tailI] = pData[tailI];
        }
        pState->m_DelayBufferReadResamplingPos.Init(1.0f, startIndex);
        pState->m_DelayBufferReadResamplingPos.Inc(startFraction);
      }

      const std::vector<uint16_t> unitIncs(nFrames, UNIT_RATE);
      GOSoundBufferPlanarManaged frozenOut(nChannels, nFrames);
      GOSoundBufferPlanarManaged unitOut(nChannels, nFrames);

      processor.ReadFromDelayBuffer(*oFrozen, nullptr, frozenOut);
      processor.ReadFromDelayBuffer(*oUnit, unitIncs.data(), unitOut);
      for (unsigned itemI = 0, n = frozenOut.GetNItems(); itemI < n; ++itemI)
        GOAssert(
          frozenOut.GetData()[itemI] == unitOut.GetData()[itemI],
          "a read without increments must equal the read with the unit ones");

      for (const State *pState : {oFrozen.get(), oUnit.get()}) {
        GOAssert(
          pState->m_DelayBufferReadResamplingPos.GetIndex()
              == (startIndex + nFrames) % ring
            && pState->m_DelayBufferReadResamplingPos.GetFraction()
              == startFraction,
          "the position must advance by exactly nFrames and keep its fraction");
        GOAssert(
          pState->m_DelayBufferReadResamplingPos.IsSameRate(),
          "the rate of the position must stay 1.0");
      }

      // The explicit increments of the curve never change the rate
      const std::vector<uint16_t> fastIncs(nFrames, UNIT_RATE + UNIT_RATE / 2);

      processor.ReadFromDelayBuffer(*oFrozen, fastIncs.data(), frozenOut);
      GOAssert(
        oFrozen->m_DelayBufferReadResamplingPos.IsSameRate(),
        "a read with the increments must not change the rate of the position");
    }
}

static constexpr unsigned CROSSFADE_N_CHANNELS = 2;

/**
 * @return whether two floats are equal up to the rounding of a crossfade
 */
static bool is_near(float a, float b) { return std::fabs(a - b) < 1e-5f; }

void GOTestSoundVibratoProcessor::TestCrossfadeWithDelayed() {
  /* A processor with a state. A period passed to run() has the delayed signal
   * of ones and the undelayed signal of zeros, so the result is the weight of
   * the delayed signal, i.e. 1 - the gain of the ramp */
  struct Rig {
    GOSoundResample resample;
    GOSoundVibratoProcessor processor;
    std::unique_ptr<State> oState;
    const unsigned nFrames;
    // Whether all the runs gave equal channels
    bool areChannelsEqual = true;

    Rig(
      unsigned nPeriodFrames,
      unsigned nActivationFrames,
      unsigned nDeactivationFrames)
      : processor(resample, GOSoundResample::GO_LINEAR_INTERPOLATION),
        nFrames(nPeriodFrames) {
      processor.EnsureSetup(
        CROSSFADE_N_CHANNELS,
        nPeriodFrames,
        TEST_SAMPLE_RATE,
        0,
        nActivationFrames,
        nDeactivationFrames);
      oState = processor.CreateTypedState();
    }

    // Returns the result of the first channel
    std::vector<float> run(bool isToActivate) {
      GOSoundBufferPlanarManaged delayed(CROSSFADE_N_CHANNELS, nFrames);
      GOSoundBufferPlanarManaged inOut(CROSSFADE_N_CHANNELS, nFrames);

      std::fill(
        delayed.GetData(), delayed.GetData() + delayed.GetNItems(), 1.0f);
      std::fill(inOut.GetData(), inOut.GetData() + inOut.GetNItems(), 0.0f);
      processor.CrossfadeWithDelayed(isToActivate, delayed, *oState, inOut);

      const float *pFirst = inOut.GetChannelBuffer(0).GetData();
      const float *pSecond = inOut.GetChannelBuffer(1).GetData();

      areChannelsEqual
        = areChannelsEqual && std::equal(pFirst, pFirst + nFrames, pSecond);
      return std::vector<float>(pFirst, pFirst + nFrames);
    }
  };
  auto isState
    = [this](const Rig &rig, State::ActivatingPhase phase, unsigned pos) {
        GOAssert(
          rig.areChannelsEqual, "all the channels must be crossfaded equally");
        return rig.oState->m_ActivatingPhase == phase
          && rig.oState->m_CrossfadePos == pos;
      };

  // An activation from INACTIVE over several periods; the ramp ends inside the
  // third one: the rest of it is the delayed signal exactly
  {
    Rig rig(16, 40, 24);

    for (unsigned periodI = 0; periodI < 2; ++periodI) {
      const std::vector<float> result = rig.run(true);

      for (unsigned frameI = 0; frameI < 16; ++frameI)
        GOAssert(
          is_near(result[frameI], (16 * periodI + frameI) / 40.0f),
          "the weight of the delayed signal must grow linearly");
      GOAssert(
        isState(rig, State::ACTIVATING, 16 * (periodI + 1)),
        "a running activation must advance the position by the period");
    }

    const std::vector<float> result = rig.run(true);

    for (unsigned frameI = 0; frameI < 16; ++frameI)
      GOAssert(
        frameI < 8 ? is_near(result[frameI], (32 + frameI) / 40.0f)
                   : result[frameI] == 1.0f,
        "after the ramp end the output must be the delayed signal exactly");
    GOAssert(
      isState(rig, State::ACTIVE, 0),
      "the end of the ramp inside a period must make the phase ACTIVE");
  }

  // A deactivation from ACTIVE; the rest after the ramp is the undelayed one
  {
    Rig rig(16, 40, 24);

    rig.oState->m_ActivatingPhase = State::ACTIVE;

    const std::vector<float> first = rig.run(false);

    for (unsigned frameI = 0; frameI < 16; ++frameI)
      GOAssert(
        is_near(first[frameI], 1.0f - frameI / 24.0f),
        "the weight of the delayed signal must fall linearly");
    GOAssert(
      isState(rig, State::DEACTIVATING, 16),
      "a deactivation must start from the position 0");

    const std::vector<float> second = rig.run(false);

    for (unsigned frameI = 0; frameI < 16; ++frameI)
      GOAssert(
        frameI < 8 ? is_near(second[frameI], 1.0f - (16 + frameI) / 24.0f)
                   : second[frameI] == 0.0f,
        "after the ramp end the output must be the undelayed signal exactly");
    GOAssert(
      isState(rig, State::INACTIVE, 0),
      "the end of the ramp inside a period must make the phase INACTIVE");
  }

  // The ramp ends exactly on the period boundary; a period longer than a ramp
  {
    Rig rig(16, 32, 24);

    rig.run(true);

    const std::vector<float> result = rig.run(true);

    for (unsigned frameI = 0; frameI < 16; ++frameI)
      GOAssert(
        is_near(result[frameI], (16 + frameI) / 32.0f),
        "the last period of the ramp must be crossfaded");
    GOAssert(
      isState(rig, State::ACTIVE, 0),
      "the ramp ending on the boundary must make the phase ACTIVE");

    Rig shortRig(16, 5, 5);
    const std::vector<float> shortResult = shortRig.run(true);

    for (unsigned frameI = 0; frameI < 16; ++frameI)
      GOAssert(
        frameI < 5 ? is_near(shortResult[frameI], frameI / 5.0f)
                   : shortResult[frameI] == 1.0f,
        "a period longer than the ramp has the ramp, then the delayed signal");
    GOAssert(
      isState(shortRig, State::ACTIVE, 0),
      "a period longer than the ramp must make the phase ACTIVE");
  }

  // A direction change in the middle of a crossfade: the position in the new
  // ramp gives the same weights, although the ramps differ in length
  {
    Rig rig(10, 40, 80);

    rig.run(true);

    const std::vector<float> activated = rig.run(true);

    GOAssert(
      isState(rig, State::ACTIVATING, 20),
      "two periods of the activation make the position 20");

    const std::vector<float> deactivated = rig.run(false);

    // The weight of the undelayed signal was 0.5; it grows from 0.5 on
    for (unsigned frameI = 0; frameI < 10; ++frameI)
      GOAssert(
        is_near(deactivated[frameI], 1.0f - (40 + frameI) / 80.0f),
        "the deactivation must continue from the same weight");
    GOAssert(
      std::fabs(deactivated[0] - activated[9]) <= 0.026f,
      "the weights must not jump at the change of the direction");
    GOAssert(
      isState(rig, State::DEACTIVATING, 50),
      "the position must be mapped into the deactivation ramp");

    const std::vector<float> reactivated = rig.run(true);

    for (unsigned frameI = 0; frameI < 10; ++frameI)
      GOAssert(
        is_near(reactivated[frameI], (15 + frameI) / 40.0f),
        "the activation must continue from the same weight");
    GOAssert(
      std::fabs(reactivated[0] - deactivated[9]) <= 0.026f,
      "the weights must not jump at the change back");
    GOAssert(
      isState(rig, State::ACTIVATING, 25),
      "the position must be mapped into the activation ramp");
  }

  // A direction change that maps onto the end of the new ramp: it ends at once
  {
    Rig rig(2, 100, 10);

    rig.run(true);

    const std::vector<float> result = rig.run(false);

    for (unsigned frameI = 0; frameI < 2; ++frameI)
      GOAssert(
        result[frameI] == 0.0f,
        "a deactivation mapped to the ramp end is the undelayed signal");
    GOAssert(
      isState(rig, State::INACTIVE, 0),
      "a deactivation mapped to the ramp end must make the phase INACTIVE");
  }

  // Disabled crossfades are instant switches
  {
    Rig rig(16, 0, 24);

    for (const float value : rig.run(true))
      GOAssert(value == 1.0f, "a disabled activation is the delayed signal");
    GOAssert(
      isState(rig, State::ACTIVE, 0),
      "a disabled activation must make the phase ACTIVE at once");

    for (const float value : rig.run(false))
      GOAssert(
        value <= 1.0f && value >= 1.0f - 16 / 24.0f - 1e-5f,
        "an enabled deactivation after a disabled activation is a crossfade");
    GOAssert(
      isState(rig, State::DEACTIVATING, 16),
      "the deactivation must start from the position 0");

    // A direction change into a disabled crossfade is instant too
    for (const float value : rig.run(true))
      GOAssert(value == 1.0f, "a disabled activation is the delayed signal");
    GOAssert(
      isState(rig, State::ACTIVE, 0),
      "a direction change into a disabled activation must make it ACTIVE");
  }
  {
    Rig rig(16, 40, 0);

    rig.oState->m_ActivatingPhase = State::ACTIVE;
    for (const float value : rig.run(false))
      GOAssert(value == 0.0f, "a disabled deactivation is the undelayed one");
    GOAssert(
      isState(rig, State::INACTIVE, 0),
      "a disabled deactivation must make the phase INACTIVE at once");

    rig.run(true);
    GOAssert(
      isState(rig, State::ACTIVATING, 16),
      "an enabled activation after a disabled deactivation is a crossfade");
    for (const float value : rig.run(false))
      GOAssert(
        value == 0.0f,
        "a direction change into a disabled deactivation is the undelayed one");
    GOAssert(
      isState(rig, State::INACTIVE, 0),
      "a direction change into a disabled deactivation must make it INACTIVE");
  }
  {
    Rig rig(16, 0, 0);

    for (const float value : rig.run(true))
      GOAssert(value == 1.0f, "both disabled: the activation is instant");
    GOAssert(isState(rig, State::ACTIVE, 0), "both disabled: ACTIVE");
    for (const float value : rig.run(false))
      GOAssert(value == 0.0f, "both disabled: the deactivation is instant");
    GOAssert(isState(rig, State::INACTIVE, 0), "both disabled: INACTIVE");
  }

  // The stored ramp length follows the crossfade and is 0 in the steady phases
  {
    Rig rig(16, 40, 24);

    GOAssert(rig.oState->m_LastCrossfadeNFrames == 0, "a new state: 0");
    rig.run(true);
    GOAssert(
      rig.oState->m_LastCrossfadeNFrames == 40,
      "a running activation stores the length of its ramp");
    rig.run(true);
    rig.run(true);
    GOAssert(
      isState(rig, State::ACTIVE, 0) && rig.oState->m_LastCrossfadeNFrames == 0,
      "the length is reset when the crossfade is over");
  }

  /* A setter changes the length of the ramp of a running crossfade (the
   * organ is suspended in between): the position is recalculated so that the
   * weights stay the same. 1 ms is 48 frames at TEST_SAMPLE_RATE */
  {
    // Longer: 40 -> 96 frames at the position 20 (the weight 0.5): 48
    Rig rig(10, 40, 24);

    rig.run(true);

    const std::vector<float> before = rig.run(true);

    rig.processor.SetActivationCrossfadeMs(2);

    const std::vector<float> after = rig.run(true);

    for (unsigned frameI = 0; frameI < 10; ++frameI)
      GOAssert(
        is_near(after[frameI], (48 + frameI) / 96.0f),
        "the weights must continue from the same weight on the longer ramp");
    GOAssert(
      std::fabs(after[0] - before[9]) <= 0.026f,
      "the weights must not jump when the ramp gets longer");
    GOAssert(
      isState(rig, State::ACTIVATING, 58)
        && rig.oState->m_LastCrossfadeNFrames == 96,
      "the position and the length must be those of the new ramp");
  }
  {
    // Shorter: 96 -> 48 frames at the position 40 (0.4167): 20, and the ramp
    // ends inside this period
    Rig rig(40, 96, 24);

    rig.run(true);
    rig.processor.SetActivationCrossfadeMs(1);

    const std::vector<float> result = rig.run(true);

    for (unsigned frameI = 0; frameI < 40; ++frameI)
      GOAssert(
        frameI < 28 ? is_near(result[frameI], (20 + frameI) / 48.0f)
                    : result[frameI] == 1.0f,
        "a shorter ramp must end inside the period, then the delayed signal");
    GOAssert(
      isState(rig, State::ACTIVE, 0) && rig.oState->m_LastCrossfadeNFrames == 0,
      "a shortened crossfade must end");
  }
  {
    // Disabled: an instant switch
    Rig rig(10, 100, 24);

    rig.run(true);
    rig.run(true);
    rig.processor.SetActivationCrossfadeMs(0);
    for (const float value : rig.run(true))
      GOAssert(value == 1.0f, "a crossfade disabled in the middle is instant");
    GOAssert(
      isState(rig, State::ACTIVE, 0) && rig.oState->m_LastCrossfadeNFrames == 0,
      "a crossfade disabled in the middle must end");
  }
  {
    /* A direction change after the length of the ramp being left was changed:
     * the old ramp is the one the position was counted in (40 frames), not the
     * new one of the processor (96 frames) */
    Rig rig(10, 40, 80);

    rig.run(true);
    rig.run(true);
    rig.processor.SetActivationCrossfadeMs(2);

    const std::vector<float> result = rig.run(false);

    for (unsigned frameI = 0; frameI < 10; ++frameI)
      GOAssert(
        is_near(result[frameI], 1.0f - (40 + frameI) / 80.0f),
        "the weight 0.5 at the old position 20 of 40 must map to 40 of 80");
    GOAssert(
      isState(rig, State::DEACTIVATING, 50)
        && rig.oState->m_LastCrossfadeNFrames == 80,
      "the direction change must use the stored length");
  }
}

/**
 * Processes nPeriods periods of a mono signal and returns the output. The
 * feeder gives the processor the chunk of every period.
 */
static std::vector<float> run_periods(
  ChunkFeeder &feeder,
  GOSoundProcessorState &state,
  unsigned nFrames,
  const std::vector<float> &input) {
  GOSoundProcessor &processor = feeder.GetProcessor();
  const unsigned nPeriods = (unsigned)input.size() / nFrames;
  std::vector<float> output(input.size());
  GOSoundBufferPlanarManaged buffer(1, nFrames);

  for (unsigned periodI = 0; periodI < nPeriods; ++periodI) {
    float *pData = buffer.GetChannelBuffer(0).GetData();

    std::copy(
      input.begin() + periodI * nFrames,
      input.begin() + (periodI + 1) * nFrames,
      pData);
    feeder.BeginPeriod();
    processor.Process(state, buffer);
    feeder.EndPeriod(nFrames);
    std::copy(pData, pData + nFrames, output.begin() + periodI * nFrames);
  }
  return output;
}

static std::vector<float> make_ramp(unsigned n) {
  std::vector<float> ramp(n);

  for (unsigned i = 0; i < n; ++i)
    ramp[i] = (float)(i + 1);
  return ramp;
}

void GOTestSoundVibratoProcessor::TestProcessBypass() {
  GOSoundResample resample;
  GOSoundVibratoProcessor processor(resample);

  processor.EnsureSetup(2, 64, TEST_SAMPLE_RATE);

  GOSoundProcessor &untypedProcessor = processor;
  std::unique_ptr<State> oState = processor.CreateTypedState();
  GOSoundBufferPlanarManaged buffer(2, 64);

  for (unsigned itemI = 0, n = buffer.GetNItems(); itemI < n; ++itemI)
    buffer.GetData()[itemI] = (float)(itemI + 1);

  const std::vector<float> before(
    buffer.GetData(), buffer.GetData() + buffer.GetNItems());

  untypedProcessor.Process(*oState, buffer);
  for (unsigned itemI = 0, n = buffer.GetNItems(); itemI < n; ++itemI)
    GOAssert(
      buffer.GetData()[itemI] == before[itemI],
      "a bypass period must leave the buffer untouched");
  GOAssert(
    oState->m_DelayBufferWritePos == L,
    "a bypass period must keep the state primed");
}

void GOTestSoundVibratoProcessor::TestProcessUnityCurveDelay() {
  const unsigned nFrames = 64;
  const unsigned nPeriods = 30;
  GOSoundResample resample;
  GOSoundVibratoProcessor processor(
    resample, GOSoundResample::GO_LINEAR_INTERPOLATION);

  processor.SetActivationCrossfadeMs(0);
  processor.SetDeactivationCrossfadeMs(0);
  processor.EnsureSetup(1, nFrames, TEST_SAMPLE_RATE);

  const std::vector<float> deviations(100, 0.0f);
  Curve curve;

  curve.Build(deviations.data(), 100, nFrames);

  ChunkFeeder feeder(processor, &curve);
  std::unique_ptr<State> oState = processor.CreateTypedState();
  const std::vector<float> input = make_ramp(nPeriods * nFrames);
  const std::vector<float> output
    = run_periods(feeder, *oState, nFrames, input);

  for (unsigned n = 0; n < output.size(); ++n)
    GOAssert(
      output[n] == (n >= L ? input[n - L] : 0.0f),
      "a zero-deviation curve must delay the signal by exactly the "
      "look-ahead");
}

void GOTestSoundVibratoProcessor::TestProcessRescaledToExactDelay() {
  GOSoundResample resample;

  for (const unsigned nFrames : {20u, 128u, 2048u})
    for (const float centsValue : {1200.0f, 12.0f, -12.0f}) {
      GOSoundVibratoProcessor processor(
        resample, GOSoundResample::GO_LINEAR_INTERPOLATION);

      /* No extra lag: a window of zero width, every period is rescaled (above
       * for positive cents, below for negative ones) */
      processor.EnsureSetup(1, nFrames, TEST_SAMPLE_RATE, 0, 0, 0);

      const std::vector<float> deviations(
        50, GOSoundTestSignals::centsToDeviation(centsValue));
      Curve curve;

      // Not normalised: Build() would normalise a constant curve to 1.0
      curve.BuildInternal(deviations.data(), 50, nFrames, false);

      ChunkFeeder feeder(processor, &curve);
      std::unique_ptr<State> oState = processor.CreateTypedState();
      const std::vector<float> input = make_ramp(40 * nFrames);
      const std::vector<float> output
        = run_periods(feeder, *oState, nFrames, input);

      for (unsigned n = 0; n < output.size(); ++n)
        GOAssert(
          output[n] == (n >= L ? input[n - L] : 0.0f),
          "a constant curve in a zero-width window must give an exact delay "
          "of the look-ahead, with no frame dropped or repeated");
    }
}

void GOTestSoundVibratoProcessor::TestProcessOctaveDownSettles() {
  const unsigned nFrames = 20;
  const unsigned nMaxLag = 50;
  const unsigned nPeriods = 100;
  GOSoundResample resample;
  GOSoundVibratoProcessor processor(
    resample, GOSoundResample::GO_LINEAR_INTERPOLATION);

  processor.EnsureSetup(1, nFrames, TEST_SAMPLE_RATE, nMaxLag, 0, 0);

  const std::vector<float> deviations(
    50, GOSoundTestSignals::centsToDeviation(-1200.0f));
  Curve curve;

  curve.BuildInternal(deviations.data(), 50, nFrames, false);

  ChunkFeeder feeder(processor, &curve);
  std::unique_ptr<State> oState = processor.CreateTypedState();
  const std::vector<float> input = make_ramp(nPeriods * nFrames);
  const std::vector<float> output
    = run_periods(feeder, *oState, nFrames, input);

  for (unsigned n = 1; n < output.size(); ++n) {
    GOAssert(output[n] >= output[n - 1], "the output must never go backwards");
    GOAssert(output[n] <= input[n], "the output must not read the future");
    if (output[n] > 0.0f) {
      const float lag = input[n] - output[n];

      GOAssert(
        lag >= (float)L - 1.0f && lag <= (float)(nMaxLag + L + nFrames),
        "the lag must stay between the look-ahead and the maximum");
    }
  }
  // Settled: the last periods are read at the unit rate
  for (unsigned n = (nPeriods - 5) * nFrames; n + 1 < output.size(); ++n)
    GOAssert(
      std::fabs(output[n + 1] - output[n] - 1.0f) < 1e-3f,
      "after the lag has grown to its maximum the slope must be 1");
}

void GOTestSoundVibratoProcessor::TestProcessNormalisedCurve() {
  const unsigned nFrames = 64;
  const unsigned loopLen = 9600; // one cycle of 5 Hz at 48 kHz
  const unsigned nPeriods = 450; // three cycles
  GOSoundResample resample;
  std::vector<float> deviations(loopLen);

  for (unsigned frameI = 0; frameI < loopLen; ++frameI)
    deviations[frameI] = GOSoundTestSignals::centsToDeviation(
      100.0f * std::sin(2.0f * (float)M_PI * (float)frameI / loopLen));

  Curve curve;

  curve.Build(deviations.data(), loopLen, nFrames);
  for (const GOSoundResample::InterpolationType type :
       {GOSoundResample::GO_LINEAR_INTERPOLATION,
        GOSoundResample::GO_POLYPHASE_INTERPOLATION}) {
    GOSoundVibratoProcessor processor(resample, type);

    processor.SetActivationCrossfadeMs(0);
    processor.SetDeactivationCrossfadeMs(0);
    processor.EnsureSetup(1, nFrames, TEST_SAMPLE_RATE);

    ChunkFeeder feeder(processor, &curve);
    GOSoundProcessor &untypedProcessor = processor;
    std::unique_ptr<State> oState = processor.CreateTypedState();
    const std::vector<float> input = make_ramp(nPeriods * nFrames);
    std::vector<float> output(input.size());
    GOSoundBufferPlanarManaged buffer(1, nFrames);
    const float maxLag
      = (float)(processor.m_NDelayBufferRingFrames + nFrames + 16);

    for (unsigned periodI = 0; periodI < nPeriods; ++periodI) {
      float *pData = buffer.GetChannelBuffer(0).GetData();

      std::copy(
        input.begin() + periodI * nFrames,
        input.begin() + (periodI + 1) * nFrames,
        pData);
      feeder.BeginPeriod();
      untypedProcessor.Process(*oState, buffer);
      feeder.EndPeriod(nFrames);
      std::copy(pData, pData + nFrames, output.begin() + periodI * nFrames);

      /* The geometry invariant, independent of the kernel: the read position
       * after the period is at or before W - L (a non-zero fraction needs at
       * least one whole frame to spare), so that every tap of every used
       * position was a written frame */
      GOAssert(
        oState->GetNFramesAvailableToRead()
          >= (oState->m_DelayBufferReadResamplingPos.GetFraction() ? 1u : 0u),
        "the read position must never get ahead of the written frames");
    }
    // The ramp checks are exact only for the linear interpolation: the
    // polyphase kernel deviates from the ramp where the position has a
    // fraction
    if (type == GOSoundResample::GO_LINEAR_INTERPOLATION)
      for (unsigned n = L + 16; n < output.size(); ++n) {
        /* A period writes all its frames before reading, so the output may be
         * ahead of input[n] by the rest of the period, but not beyond it */
        const unsigned periodLastFrame = (n / nFrames + 1) * nFrames - 1;

        GOAssert(
          output[n] >= output[n - 1],
          "no stale or unwritten ring slot may be read: the ramp must stay "
          "monotonic");
        GOAssert(
          output[n] <= input[periodLastFrame] && input[n] - output[n] <= maxLag,
          "the lag must stay bounded over several cycles");
      }
  }
}

void GOTestSoundVibratoProcessor::TestProcessBypassAfterActiveAndRestart() {
  const unsigned nFrames = 64;
  GOSoundResample resample;
  GOSoundVibratoProcessor processor(
    resample, GOSoundResample::GO_LINEAR_INTERPOLATION);

  processor.SetActivationCrossfadeMs(0);
  processor.SetDeactivationCrossfadeMs(0);
  processor.EnsureSetup(1, nFrames, TEST_SAMPLE_RATE);

  std::vector<float> deviations(100);

  for (unsigned frameI = 0; frameI < 100; ++frameI)
    deviations[frameI] = GOSoundTestSignals::centsToDeviation(
      200.0f * std::sin(0.3f * (float)frameI));

  Curve curve;

  curve.Build(deviations.data(), 100, nFrames);

  ChunkFeeder feeder(processor, &curve);
  GOSoundProcessor &untypedProcessor = processor;
  const std::vector<float> input = make_ramp(20 * nFrames);
  std::unique_ptr<State> oFresh = processor.CreateTypedState();
  const std::vector<float> reference
    = run_periods(feeder, *oFresh, nFrames, input);

  // Active periods, then a bypass period
  std::unique_ptr<State> oState = processor.CreateTypedState();

  feeder.Rewind();
  run_periods(feeder, *oState, nFrames, make_ramp(3 * nFrames));
  GOAssert(oState->m_ActivatingPhase == State::ACTIVE, "the vibrato is active");
  feeder.SetCurve(nullptr);

  GOSoundBufferPlanarManaged buffer(1, nFrames);

  for (unsigned i = 0; i < nFrames; ++i)
    buffer.GetChannelBuffer(0).GetData()[i] = (float)(i + 1);
  feeder.BeginPeriod();
  untypedProcessor.Process(*oState, buffer);
  for (unsigned i = 0; i < nFrames; ++i)
    GOAssert(
      buffer.GetChannelBuffer(0).GetData()[i] == (float)(i + 1),
      "a bypass period must leave the buffer untouched");
  GOAssert(
    oState->m_DelayBufferWritePos == L
      && oState->m_DelayBufferReadResamplingPos.GetIndex() == 0,
    "a bypass period must re-prime the delay positions");

  // Active again: it must start as a fresh state does
  feeder.SetCurve(&curve);
  GOAssert(
    run_periods(feeder, *oState, nFrames, input) == reference,
    "an activation after a bypass must start from the minimum lag and the "
    "start of the curve");

  // A restart of the chain: the state is reset and the cursor rewound
  oState->Reset();
  feeder.Rewind();
  GOAssert(
    run_periods(feeder, *oState, nFrames, input) == reference,
    "after a reset the curve must restart at its beginning");
}

void GOTestSoundVibratoProcessor::TestProcessCrossfade() {
  const unsigned nFrames = 64;
  const unsigned nActivationFrames = 2 * nFrames;
  const unsigned nDeactivationFrames = 3 * nFrames;
  const unsigned nPeriods = 9;
  GOSoundResample resample;
  GOSoundVibratoProcessor processor(
    resample, GOSoundResample::GO_LINEAR_INTERPOLATION);

  processor.EnsureSetup(
    1, nFrames, TEST_SAMPLE_RATE, 0, nActivationFrames, nDeactivationFrames);

  // A zero-deviation curve: the delayed signal is the input delayed by L
  // frames
  const std::vector<float> deviations(100, 0.0f);
  Curve curve;

  curve.Build(deviations.data(), 100, nFrames);

  ChunkFeeder feeder(processor, &curve);
  GOSoundProcessor &untypedProcessor = processor;
  std::unique_ptr<State> oState = processor.CreateTypedState();
  const std::vector<float> input = make_ramp(nPeriods * nFrames);
  std::vector<float> output;
  std::vector<State::ActivatingPhase> phases;
  GOSoundBufferPlanarManaged buffer(1, nFrames);

  for (unsigned periodI = 0; periodI < nPeriods; ++periodI) {
    float *pData = buffer.GetChannelBuffer(0).GetData();

    // The periods 0-1 activate, 2-3 are active, 4-6 deactivate, 7-8 bypass
    if (periodI == 4)
      feeder.SetCurve(nullptr);
    std::copy(
      input.begin() + periodI * nFrames,
      input.begin() + (periodI + 1) * nFrames,
      pData);
    feeder.BeginPeriod();
    untypedProcessor.Process(*oState, buffer);
    feeder.EndPeriod(nFrames);
    output.insert(output.end(), pData, pData + nFrames);
    phases.push_back(oState->m_ActivatingPhase);
  }

  const std::vector<State::ActivatingPhase> expectedPhases = {
    State::ACTIVATING,
    State::ACTIVE,
    State::ACTIVE,
    State::ACTIVE,
    State::DEACTIVATING,
    State::DEACTIVATING,
    State::INACTIVE,
    State::INACTIVE,
    State::INACTIVE};

  GOAssert(
    phases == expectedPhases,
    "the phases must go INACTIVE -> ACTIVATING -> ACTIVE -> DEACTIVATING -> "
    "INACTIVE");

  for (unsigned n = 0; n < output.size(); ++n) {
    // The weight of the undelayed signal
    const float undelayedWeight = n < nActivationFrames
      ? 1.0f - (float)n / nActivationFrames
      : n < 4 * nFrames ? 0.0f
      : n < 4 * nFrames + nDeactivationFrames
      ? (float)(n - 4 * nFrames) / nDeactivationFrames
      : 1.0f;
    const float delayed = n >= L ? input[n - L] : 0.0f;

    GOAssert(
      std::fabs(
        output[n]
        - (undelayedWeight * input[n] + (1.0f - undelayedWeight) * delayed))
        < 1e-2f,
      "the output must be the crossfade of the undelayed and delayed signals");
  }
  for (unsigned n = 4 * nFrames + nDeactivationFrames; n < output.size(); ++n)
    GOAssert(
      output[n] == input[n], "after the deactivation the input is not changed");

  // The delayed signal has faded out: the state equals a fresh one
  GOAssert(
    oState->m_DelayBufferWritePos == L
      && oState->m_DelayBufferReadResamplingPos.GetIndex() == 0
      && oState->m_DelayBufferReadResamplingPos.GetFraction() == 0
      && oState->m_CrossfadePos == 0,
    "after the deactivation the state must be reset");
}

void GOTestSoundVibratoProcessor::TestProcessCrossfadeReactivation() {
  const unsigned nFrames = 64;
  const unsigned nMaxLag = 128;
  const unsigned nActivationFrames = 4 * nFrames;
  const unsigned nDeactivationFrames = 3 * nFrames;
  const unsigned nPeriods = 24;
  // The periods without the curve: the one that starts the deactivation to be
  // interrupted, and the final ones
  const unsigned offPeriodI = 4;
  const unsigned reactivationPeriodI = 5;
  const unsigned finalOffPeriodI = 13;
  GOSoundResample resample;
  GOSoundVibratoProcessor processor(
    resample, GOSoundResample::GO_LINEAR_INTERPOLATION);

  processor.EnsureSetup(
    1,
    nFrames,
    TEST_SAMPLE_RATE,
    nMaxLag,
    nActivationFrames,
    nDeactivationFrames);

  std::vector<float> deviations(100);

  for (unsigned frameI = 0; frameI < 100; ++frameI)
    deviations[frameI] = GOSoundTestSignals::centsToDeviation(
      200.0f * std::sin(0.3f * (float)frameI));

  Curve curve;

  curve.Build(deviations.data(), 100, nFrames);

  // The chunk at the start of the loop, as a new cursor gives it
  Curve::LoopPosition startCursor(curve.GetNLoopFrames());
  const Curve::ChunkDescription startChunk
    = curve.GetChunkDescription(startCursor);
  const uint64_t startChunkUnits = startChunk.nTotalUnits;

  // A cursor that was not rewound would give another chunk here
  startCursor.Advance(nFrames);
  GOAssert(
    curve.GetChunkDescription(startCursor).nTotalUnits != startChunkUnits,
    "precondition: the second chunk of the loop must differ from the first");

  ChunkFeeder feeder(processor, &curve);
  GOSoundProcessor &untypedProcessor = processor;
  std::unique_ptr<State> oState = processor.CreateTypedState();
  GOSoundBufferPlanarManaged buffer(1, nFrames);
  const unsigned ring = processor.m_NDelayBufferRingFrames;
  std::vector<float> output;
  std::vector<State::ActivatingPhase> phases;
  // The reads of the periods 4 and 5: the position before and after
  unsigned readIndexBefore = 0;
  unsigned readFractionBefore = 0;
  uint64_t expectedReadUnits = 0;
  unsigned readIndexDeltaFrozen = 0;
  bool isFractionChangedWhileFrozen = false;
  int64_t readUnitsDeltaReactivated = 0;
  unsigned crossfadePosAfterReactivation = 0;

  for (unsigned periodI = 0; periodI < nPeriods; ++periodI) {
    float *pData = buffer.GetChannelBuffer(0).GetData();
    const bool isCurve = periodI < offPeriodI
      || (periodI >= reactivationPeriodI && periodI < finalOffPeriodI);

    feeder.SetCurve(isCurve ? &curve : nullptr);
    for (unsigned frameI = 0; frameI < nFrames; ++frameI)
      pData[frameI] = std::sin(
        2.0f * (float)M_PI * 440.0f * (periodI * nFrames + frameI)
        / TEST_SAMPLE_RATE);
    readIndexBefore = oState->m_DelayBufferReadResamplingPos.GetIndex();
    readFractionBefore = oState->m_DelayBufferReadResamplingPos.GetFraction();

    if (periodI == offPeriodI)
      /* The lag must be above the minimum, or the test is meaningless: the
       * minimum lag leaves nothing to read before the write of a period */
      GOAssert(
        oState->GetNFramesAvailableToRead() > 0,
        "precondition: the lag must have grown before the curve is removed");
    if (periodI == reactivationPeriodI) {
      /* What the chunk at the start of the loop must become after the write of
       * this period, which makes nFrames more to read and less to write. It is
       * computed by the same method that Process() uses, so the limiting by
       * the window of valid lags is not verified independently here (see
       * TestIncrements*); this checks that the chunk is the one at the start of
       * the loop and that its sum is what is read */
      uint16_t tmpIncs[nFrames];

      processor.SetPitchRateChunk(startChunk);

      const uint16_t *pIncs = processor.GetResamplerPositionIncrementsForUse(
        readFractionBefore,
        oState->GetNFramesAvailableToRead() + nFrames,
        oState->GetNFramesAvailableToWrite() - nFrames,
        tmpIncs);

      expectedReadUnits = sum_of(pIncs, nFrames);
    }
    feeder.BeginPeriod();
    untypedProcessor.Process(*oState, buffer);
    feeder.EndPeriod(nFrames);
    output.insert(output.end(), pData, pData + nFrames);
    phases.push_back(oState->m_ActivatingPhase);

    const unsigned readIndexAfter
      = oState->m_DelayBufferReadResamplingPos.GetIndex();
    const unsigned readFractionAfter
      = oState->m_DelayBufferReadResamplingPos.GetFraction();

    if (periodI == offPeriodI) {
      readIndexDeltaFrozen = (readIndexAfter + ring - readIndexBefore) % ring;
      isFractionChangedWhileFrozen = readFractionAfter != readFractionBefore;
    } else if (periodI == reactivationPeriodI) {
      readUnitsDeltaReactivated
        = (int64_t)((readIndexAfter + ring - readIndexBefore) % ring)
          * UNIT_RATE
        + (int64_t)readFractionAfter - (int64_t)readFractionBefore;
      crossfadePosAfterReactivation = oState->m_CrossfadePos;
    }
  }

  // The periods 0-2 activate, 3 is active, 4 deactivates, 5-6 reactivate
  const std::vector<State::ActivatingPhase> expectedPhasesBegin = {
    State::ACTIVATING,
    State::ACTIVATING,
    State::ACTIVATING,
    State::ACTIVE,
    State::DEACTIVATING,
    State::ACTIVATING,
    State::ACTIVE};

  GOAssert(
    std::equal(
      expectedPhasesBegin.begin(), expectedPhasesBegin.end(), phases.begin()),
    "the phases must go to DEACTIVATING, turn around to ACTIVATING and settle "
    "ACTIVE");
  GOAssert(
    phases[finalOffPeriodI - 1] == State::ACTIVE
      && phases[finalOffPeriodI] == State::DEACTIVATING
      && phases.back() == State::INACTIVE,
    "after the reactivation the vibrato must be able to deactivate fully");

  /* The crossfade turned around at the same weight: the position in the
   * activation ramp that has the gain of the deactivation ramp after the
   * period 4, plus the period 5 */
  const unsigned mappedPos = GOSoundLinearFaderCore::mapFrameOffset(
    processor.m_DeactivationRamp, nFrames, processor.m_ActivationRamp);

  GOAssert(
    crossfadePosAfterReactivation == mappedPos + nFrames,
    "the position must be mapped into the activation ramp");

  // Without the curve the lag is frozen: the read position advances as the
  // write one, keeping its fraction
  GOAssert(
    readIndexDeltaFrozen == nFrames && !isFractionChangedWhileFrozen,
    "the lag must stay frozen while the curve is absent");

  /* The feeder's curve was removed meanwhile, so the new cursor starts from
   * the start of the loop, not from where the old one had stopped */
  GOAssert(
    readUnitsDeltaReactivated == (int64_t)expectedReadUnits,
    "the first period after the reactivation must use the start of the loop "
    "limited by the window of valid lags");

  // No jump anywhere, in particular at the turns
  float maxStep = 0.0f;

  for (unsigned n = 1; n < output.size(); ++n)
    maxStep = std::max(maxStep, std::fabs(output[n] - output[n - 1]));
  // The step of the input itself is 2 pi 440 / 48000 = 0.058
  GOAssert(maxStep < 0.15f, "the output must have no jump over the turns");

  // The delayed signal has faded out: the state equals a fresh one
  GOAssert(
    oState->m_DelayBufferWritePos == L
      && oState->m_DelayBufferReadResamplingPos.GetIndex() == 0
      && oState->m_DelayBufferReadResamplingPos.GetFraction() == 0
      && oState->m_CrossfadePos == 0,
    "after the final deactivation the state must be reset");
}

void GOTestSoundVibratoProcessor::TestProcessCrossfadeContinuity() {
  const unsigned nFrames = 64;
  const unsigned nPeriodsWithCurve = 20;
  const unsigned nPeriods = 100;
  GOSoundResample resample;
  std::vector<float> deviations(100);

  for (unsigned frameI = 0; frameI < 100; ++frameI)
    deviations[frameI] = GOSoundTestSignals::centsToDeviation(
      200.0f * std::sin(0.3f * (float)frameI));

  Curve curve;

  curve.Build(deviations.data(), 100, nFrames);

  const std::vector<float> input
    = GOSoundTestSignals::makeSine(nPeriods * nFrames, 440.0, TEST_SAMPLE_RATE);

  // The largest step between two neighbour output samples for a pair of the
  // crossfade lengths
  auto getMaxStep = [&](unsigned activationMs, unsigned deactivationMs) {
    GOSoundVibratoProcessor processor(
      resample, GOSoundResample::GO_LINEAR_INTERPOLATION);

    processor.SetActivationCrossfadeMs(activationMs);
    processor.SetDeactivationCrossfadeMs(deactivationMs);
    processor.EnsureSetup(1, nFrames, TEST_SAMPLE_RATE);

    ChunkFeeder feeder(processor, &curve);
    GOSoundProcessor &untypedProcessor = processor;
    std::unique_ptr<State> oState = processor.CreateTypedState();
    GOSoundBufferPlanarManaged buffer(1, nFrames);
    std::vector<float> output;

    for (unsigned periodI = 0; periodI < nPeriods; ++periodI) {
      float *pData = buffer.GetChannelBuffer(0).GetData();

      if (periodI == nPeriodsWithCurve)
        feeder.SetCurve(nullptr);
      std::copy(
        input.begin() + periodI * nFrames,
        input.begin() + (periodI + 1) * nFrames,
        pData);
      feeder.BeginPeriod();
      untypedProcessor.Process(*oState, buffer);
      feeder.EndPeriod(nFrames);
      output.insert(output.end(), pData, pData + nFrames);
    }
    GOAssert(
      oState->m_ActivatingPhase == State::INACTIVE,
      "the crossfades must be over at the end");

    float maxStep = 0.0f;

    for (unsigned n = 1; n < output.size(); ++n)
      maxStep = std::max(maxStep, std::fabs(output[n] - output[n - 1]));
    return maxStep;
  };
  // The step of the input itself is 2 pi 440 / 48000 = 0.058
  const float maxStepWithCrossfades = getMaxStep(
    GOSoundVibratoProcessor::DEFAULT_ACTIVATION_CROSSFADE_MS,
    GOSoundVibratoProcessor::DEFAULT_DEACTIVATION_CROSSFADE_MS);
  const float maxStepWithoutCrossfades = getMaxStep(0, 0);

  GOAssert(
    maxStepWithCrossfades < 0.15f,
    "the crossfades must remove the jumps at the switch-on and switch-off");
  GOAssert(
    maxStepWithoutCrossfades > 0.15f,
    "without the crossfades the switch-on or switch-off must jump");
}

void GOTestSoundVibratoProcessor::TestProcessPitchDeviation() {
  struct DeviationCase {
    float depthCents;
    unsigned nLoopFrames;
    float carrierHz;
    // The accuracy of a zero-crossing measurement of one period
    float toleranceCents;
  };

  const DeviationCase cases[] = {
    {50.0f, 9600, 440.0f, 2.0f},
    {100.0f, 6000, 220.0f, 3.0f},
    {100.0f, 9600, 880.0f, 3.0f},
  };
  const GOSoundResample::InterpolationType interpolationTypes[] = {
    GOSoundResample::GO_LINEAR_INTERPOLATION,
    GOSoundResample::GO_POLYPHASE_INTERPOLATION};
  const unsigned nFrames = 64;
  const unsigned nLoops = 3;
  GOSoundResample resample;

  for (const GOSoundResample::InterpolationType interpolationType :
       interpolationTypes)
    for (const DeviationCase &deviationCase : cases) {
      const unsigned nLoopFrames = deviationCase.nLoopFrames;
      std::vector<float> deviations(nLoopFrames);
      // The rates the curve is expected to hold, in frames per frame
      std::vector<double> rates(nLoopFrames);
      double sumRates = 0.0;

      /* The curve starts downwards, so the lag of the delay line only grows
       * from its initial minimum and never has to be limited */
      for (unsigned frameI = 0; frameI < nLoopFrames; ++frameI) {
        const double frameCents = -deviationCase.depthCents
          * std::sin(2.0 * M_PI * frameI / nLoopFrames);

        deviations[frameI]
          = GOSoundTestSignals::centsToDeviation((float)frameCents);
        rates[frameI] = std::exp2(frameCents / 1200.0);
        sumRates += rates[frameI];
      }
      // The curve is normalised: the mean rate of a loop is exactly 1
      for (double &rate : rates)
        rate *= nLoopFrames / sumRates;

      Curve curve;

      curve.Build(deviations.data(), nLoopFrames, nFrames);

      GOSoundVibratoProcessor processor(resample, interpolationType);

      processor.EnsureSetup(1, nFrames, TEST_SAMPLE_RATE);

      ChunkFeeder feeder(processor, &curve);
      std::unique_ptr<State> oState = processor.CreateTypedState();
      const unsigned nPeriods = (nLoops * nLoopFrames + nFrames - 1) / nFrames;
      const std::vector<float> input = GOSoundTestSignals::makeSine(
        nPeriods * nFrames, deviationCase.carrierHz, TEST_SAMPLE_RATE);
      const std::vector<float> output
        = run_periods(feeder, *oState, nFrames, input);

      GOAssert(
        oState->m_ActivatingPhase == State::ACTIVE,
        "the vibrato must be active during the whole measurement");

      /* The first loop is skipped: the activation crossfade and the build-up of
       * the lag are over after it. The output frame n is read with the rate of
       * the curve frame n % nLoopFrames. */
      std::vector<double> frameRates(output.size());
      unsigned nMeasuredPeriods = 0;

      for (unsigned frameI = 0, n = output.size(); frameI < n; ++frameI)
        frameRates[frameI] = rates[frameI % nLoopFrames];

      const double maxErrorCents = GOSoundTestSignals::maxPitchErrorCents(
        output,
        nLoopFrames,
        deviationCase.carrierHz,
        TEST_SAMPLE_RATE,
        frameRates,
        nMeasuredPeriods);

      GOAssert(
        nMeasuredPeriods > 20, "there must be enough periods to measure");
      GOAssert(
        maxErrorCents <= deviationCase.toleranceCents,
        "the measured pitch must follow the cents of the curve");
    }
}

void GOTestSoundVibratoProcessor::TestProcessGarbageNotLeaked() {
  const unsigned nFrames = 20;
  GOSoundResample resample;
  GOSoundVibratoProcessor processor(
    resample, GOSoundResample::GO_LINEAR_INTERPOLATION);

  // A tiny ring: many laps in a few periods
  processor.EnsureSetup(1, nFrames, TEST_SAMPLE_RATE, 0, 0, 0);

  const std::vector<float> deviations(50, 0.0f);
  Curve curve;

  curve.Build(deviations.data(), 50, nFrames);

  ChunkFeeder feeder(processor, &curve);
  std::unique_ptr<State> oState = processor.CreateTypedState();
  float *pData
    = oState->m_DelayBufferRingWithTail.GetChannelBuffer(0).GetData();

  /* Garbage everywhere, including the tail, then a full reset. The reset does
   * not clear the tail on purpose: it is read only after the writes have
   * wrapped the ring and mirrored fresh data into it, so the stale tail must
   * never reach the output. A tiny ring makes many laps in a few periods. */
  std::fill(
    pData,
    pData + processor.m_NDelayBufferRingFrames
      + State::N_DELAY_BUFFER_TAIL_FRAMES,
    1.0e6f);
  oState->Reset();

  std::vector<float> input(40 * nFrames);

  for (unsigned i = 0; i < input.size(); ++i)
    input[i] = (float)(i % 100 + 1);

  const std::vector<float> output
    = run_periods(feeder, *oState, nFrames, input);

  for (unsigned n = 0; n < output.size(); ++n)
    GOAssert(
      std::fabs(output[n]) <= 100.0f,
      "no garbage of the delay buffer may reach the output");
}

void GOTestSoundVibratoProcessor::run() {
  GO_RUN_TEST(TestStateConstructor())
  GO_RUN_TEST(TestStateAvailableFrames())
  GO_RUN_TEST(TestStateWriteToDelayBuffer())
  GO_RUN_TEST(TestStateReset())
  GO_RUN_TEST(TestStatePhase())
  GO_RUN_TEST(TestEnsureSetup())
  GO_RUN_TEST(TestEnsureSetupCrossfadeRamps())
  GO_RUN_TEST(TestSetPitchRateChunk())
  GO_RUN_TEST(TestCreateTypedState())
  GO_RUN_TEST(TestIncrementsZeroWidthWindow())
  GO_RUN_TEST(TestIncrementsWiderWindow())
  GO_RUN_TEST(TestIncrementsLowerMeanBound())
  GO_RUN_TEST(TestIncrementsUpperMeanBound())
  GO_RUN_TEST(TestIncrementsRandom())
  GO_RUN_TEST(TestReadFromDelayBuffer())
  GO_RUN_TEST(TestReadFromDelayBufferFrozen())
  GO_RUN_TEST(TestCrossfadeWithDelayed())
  GO_RUN_TEST(TestProcessBypass())
  GO_RUN_TEST(TestProcessUnityCurveDelay())
  GO_RUN_TEST(TestProcessRescaledToExactDelay())
  GO_RUN_TEST(TestProcessOctaveDownSettles())
  GO_RUN_TEST(TestProcessNormalisedCurve())
  GO_RUN_TEST(TestProcessBypassAfterActiveAndRestart())
  GO_RUN_TEST(TestProcessCrossfade())
  GO_RUN_TEST(TestProcessCrossfadeReactivation())
  GO_RUN_TEST(TestProcessCrossfadeContinuity())
  GO_RUN_TEST(TestProcessPitchDeviation())
  GO_RUN_TEST(TestProcessGarbageNotLeaked())
}
