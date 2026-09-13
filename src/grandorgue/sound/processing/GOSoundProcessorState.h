/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDPROCESSORSTATE_H
#define GOSOUNDPROCESSORSTATE_H

/**
 * Per-instance DSP memory of one GOSoundProcessor. Exactly one
 * GOSoundProcessorState is touched by the Process() calls of exactly one
 * GOSoundProcessingChainState: it is never shared between chain instances,
 * which is what lets a single GOSoundProcessor instance be used by many
 * independent chain states (e.g. one per audio group of a windchest).
 */
class GOSoundProcessorState {
public:
  virtual ~GOSoundProcessorState() = default;

  /**
   * Clears the accumulated DSP memory contents to their initial values, as
   * if no audio had been processed yet. Does not free any allocation.
   */
  virtual void Reset() = 0;
};

#endif /* GOSOUNDPROCESSORSTATE_H */
