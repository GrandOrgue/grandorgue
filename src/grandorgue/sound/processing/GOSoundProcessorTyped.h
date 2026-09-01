/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDPROCESSORTYPED_H
#define GOSOUNDPROCESSORTYPED_H

#include <cassert>
#include <memory>
#include <type_traits>

#include "GOSoundProcessor.h"
#include "GOSoundProcessorState.h"

/**
 * Base for a GOSoundProcessor whose concrete GOSoundProcessorState type is
 * known at compile time. A concrete processor derives from
 * GOSoundProcessorTyped<TState> and implements CreateTypedState() and the
 * typed Process(TState&, buffer) instead of the untyped GOSoundProcessor
 * entry points, which this class implements once for every subclass by
 * downcasting the untyped GOSoundProcessorState& it receives.
 *
 * The downcast is an ordinary virtual call to the typed Process(), not a
 * static_cast<Derived*>(this) (that would need TProcessor as a second
 * template parameter and give up compile-time verification that the
 * subclass actually implements the typed overload: a subclass that forgot
 * it would have the call silently recurse into the untyped Process()
 * forever instead of failing to build). The extra virtual call this costs
 * is negligible next to the DSP work Process() does.
 */
template <typename TState>
class GOSoundProcessorTyped : public GOSoundProcessor {
  static_assert(
    std::is_base_of_v<GOSoundProcessorState, TState>,
    "TState must derive from GOSoundProcessorState");

public:
  std::unique_ptr<GOSoundProcessorState> CreateState() const final {
    return CreateTypedState();
  }

  void Process(GOSoundProcessorState &state, GOSoundBufferPlanarMutable &buffer)
    const final {
    assert(dynamic_cast<TState *>(&state));
    Process(static_cast<TState &>(state), buffer);
  }

protected:
  /** @return a fresh, already-reset TState object for one chain state */
  virtual std::unique_ptr<TState> CreateTypedState() const = 0;

  /**
   * The typed entry point a concrete processor implements instead of the
   * untyped GOSoundProcessor::Process().
   * @param state The DSP memory of the calling chain state, already known
   *   to be a TState
   * @param buffer The buffer to process in place
   */
  virtual void Process(
    TState &state, GOSoundBufferPlanarMutable &buffer) const = 0;
};

#endif /* GOSOUNDPROCESSORTYPED_H */
