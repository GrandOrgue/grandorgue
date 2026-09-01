/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDPROCESSINGTESTIMPLS_H
#define GOSOUNDPROCESSINGTESTIMPLS_H

#include <vector>

#include "sound/processing/GOSoundProcessingPrmMapper.h"
#include "sound/processing/GOSoundProcessorState.h"
#include "sound/processing/GOSoundProcessorTyped.h"

/** Fake state that only counts Reset() calls; used by the reach tests. */
class GOCountingProcessorState : public GOSoundProcessorState {
private:
  // Number of Reset() calls seen so far.
  unsigned m_NResets = 0;

public:
  void Reset() override { m_NResets++; }

  unsigned GetNResets() const { return m_NResets; }
};

/**
 * Fake processor that adds a constant to every sample of the buffer.
 * Optionally pushes an id into a shared ordering log, and tracks
 * EnsureSetup() calls/arguments, for the ordering and reach tests.
 */
class GOAddConstProcessor
  : public GOSoundProcessorTyped<GOCountingProcessorState> {
private:
  // Constant added to every sample in Process(); mutable via SetValue() so
  // GOCountingMapper can demonstrate writing a new parameter into a
  // processor.
  float m_value;
  // Optional shared ordering log; null if this instance doesn't use one.
  std::vector<int> *p_log;
  // Value pushed to *p_log on each Process() call.
  const int m_id;
  // Number of EnsureSetup() calls seen so far.
  unsigned m_NSetups = 0;
  // Arguments received by the most recent EnsureSetup() call.
  unsigned m_LastNChannels = 0;
  unsigned m_LastNFrames = 0;
  unsigned m_LastSampleRate = 0;

public:
  explicit GOAddConstProcessor(
    float value, std::vector<int> *pLog = nullptr, int id = 0);

  void EnsureSetup(
    unsigned nChannels, unsigned nFrames, unsigned sampleRate) override;

  unsigned GetNSetups() const { return m_NSetups; }
  unsigned GetLastNChannels() const { return m_LastNChannels; }
  unsigned GetLastNFrames() const { return m_LastNFrames; }
  unsigned GetLastSampleRate() const { return m_LastSampleRate; }

  /** Overwrites the constant added by Process(); the parameter GOCountingMapper
   * writes. */
  void SetValue(float value) { m_value = value; }

protected:
  std::unique_ptr<GOCountingProcessorState> CreateTypedState() const override;

  void Process(
    GOCountingProcessorState &state,
    GOSoundBufferPlanarMutable &buffer) const override;
};

/**
 * Fake processor that multiplies every sample of the buffer by a constant.
 * Paired with GOAddConstProcessor to make chain order observable end-to-end
 * (add-then-scale != scale-then-add), not just via a shared log.
 */
class GOScaleProcessor
  : public GOSoundProcessorTyped<GOCountingProcessorState> {
private:
  // Multiplier applied to every sample in Process().
  const float m_value;

public:
  explicit GOScaleProcessor(float value);

  void EnsureSetup(
    unsigned nChannels, unsigned nFrames, unsigned sampleRate) override {}

protected:
  std::unique_ptr<GOCountingProcessorState> CreateTypedState() const override;

  void Process(
    GOCountingProcessorState &state,
    GOSoundBufferPlanarMutable &buffer) const override;
};

/**
 * Per-channel DSP memory of GOOnePoleProcessor: one carried previous output
 * sample per channel.
 */
class GOOnePoleState : public GOSoundProcessorState {
private:
  // One carried previous-sample value per channel; read and updated by
  // GOOnePoleProcessor::Process(), zeroed by Reset(). Sized once at
  // construction time, from the processor's EnsureSetup()-supplied channel
  // count; never resized afterwards.
  std::vector<float> m_prev;

public:
  /** Zero-fills m_prev to exactly nChannels entries. */
  explicit GOOnePoleState(unsigned nChannels) : m_prev(nChannels, 0.0f) {}

  void Reset() override;

  float GetPrev(unsigned channelI) const { return m_prev[channelI]; }
  void SetPrev(unsigned channelI, float value) { m_prev[channelI] = value; }
};

/**
 * Fake one-pole IIR processor: y[n] = alpha*x[n] + (1-alpha)*y[n-1],
 * carrying y[-1] in the GOOnePoleState across Process() calls. This is the
 * stateful fake the processor-sharing test depends on: two GOOnePoleState
 * objects created from the same GOOnePoleProcessor must evolve
 * independently.
 */
class GOOnePoleProcessor : public GOSoundProcessorTyped<GOOnePoleState> {
private:
  // IIR coefficient, fixed at construction (no mapper needed for this fake).
  const float m_alpha;
  // Channel count from the most recent EnsureSetup() call; used to size
  // every GOOnePoleState this processor creates.
  unsigned m_NChannels = 0;

public:
  explicit GOOnePoleProcessor(float alpha);

  void EnsureSetup(
    unsigned nChannels, unsigned nFrames, unsigned sampleRate) override {
    m_NChannels = nChannels;
  }

protected:
  std::unique_ptr<GOOnePoleState> CreateTypedState() const override;

  void Process(
    GOOnePoleState &state, GOSoundBufferPlanarMutable &buffer) const override;
};

/**
 * Fake mapper that counts EnsureParametersUpToDate() calls, and optionally
 * writes a new constant into its target GOAddConstProcessor, to demonstrate
 * a mapper's real role of pushing a value into a processor's parameters.
 */
class GOCountingMapper : public GOSoundProcessingPrmMapper {
private:
  // Number of EnsureParametersUpToDate() calls seen so far.
  unsigned m_NCalls = 0;
  // Whether EnsureParametersUpToDate() should push m_NewValue into the
  // target processor, or just count the call.
  const bool m_IsPushingValue;
  // New constant to write into the target processor, if m_IsPushingValue.
  const float m_NewValue;

public:
  explicit GOCountingMapper(
    GOAddConstProcessor &processor,
    bool isPushingValue = false,
    float newValue = 0);

  void EnsureParametersUpToDate() override;

  unsigned GetNCalls() const { return m_NCalls; }
};

#endif /* GOSOUNDPROCESSINGTESTIMPLS_H */
