/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDSHELFFILTERPROCESSORSTATE_H
#define GOSOUNDSHELFFILTERPROCESSORSTATE_H

#include <vector>

#include "sound/processing/GOSoundProcessorState.h"

class GOSoundShelfFilterProcessor;
class GOTestSoundShelfFilterProcessor;

/**
 * Per-chain-state DSP memory of GOSoundShelfFilterProcessor: one carried
 * filter-state float per channel per band. The low and high bands are
 * independent one-pole filters, so they need separate memory even though
 * both are applied to the same channel in series.
 */
class GOSoundShelfFilterProcessorState : public GOSoundProcessorState {
private:
  friend class GOSoundShelfFilterProcessor;     // Process() needs direct access
                                                // to the state vectors
  friend class GOTestSoundShelfFilterProcessor; // direct access for unit
                                                // tests

  std::vector<float> m_LowState;
  std::vector<float> m_HighState;

  /** Whether the low/high band was filtered by the last Process() call, i.e.
   * its state vector may be non-zero. Lets Process() detect the transition
   * to an inactive band and clear the stale memory exactly once. */
  bool m_WasLowActive = false;
  bool m_WasHighActive = false;

public:
  /**
   * Zero-fills both state vectors to exactly nChannels entries each.
   * @param nChannels Number of channels this state will be Process()'d with
   */
  explicit GOSoundShelfFilterProcessorState(unsigned nChannels);

  /** Clears both bands' carried filter memory to 0, as if no audio had
   * been processed yet. Does not free any allocation. */
  void Reset() override;
};

#endif /* GOSOUNDSHELFFILTERPROCESSORSTATE_H */
