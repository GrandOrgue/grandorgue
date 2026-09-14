/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOROUNDCOUNTER_H
#define GOROUNDCOUNTER_H

#include <atomic>
#include <cstdint>

/**
 * The round number of one GOScheduler, incremented by its NewRound() every
 * time the registered tasks' per-round state is reset.
 *
 * It exists so that a thread waiting for a round to finish can tell "this
 * round has not started yet" apart from "my round is over and this is the
 * next one" - two situations that GOSoundTaskBase::m_RunState reports
 * identically, as RUN_STATE_NOT_STARTED. A waiter captures the round number
 * before it checks the state and leaves as soon as the number changes;
 * without that, a NewRound() landing between the check and the wait strands
 * the waiter on a round nobody will ever finish - see
 * GOSoundGroupTask::EnsureBufferReady().
 *
 * This is NOT the engine time (GOSoundSamplerPlayer::GetTime()), even though
 * both are monotonic and both advance once per period while streaming. They
 * diverge at GOSoundOrganEngine::StartEngine(), and that is the point: the
 * round number must advance wherever the round state is reset, and
 * StartEngine() resets it through GOScheduler::NewRound() without producing
 * any audio, while the engine time must advance only once per *produced*
 * period - sampler scheduling (GOSoundSampler::time, stop, new_attack) is
 * expressed in it, so a phantom period per resume would shift all of it.
 *
 * The round number is deliberately never reset, not even when the engine is
 * rebuilt: a waiter must never be fooled by a repeated value.
 */
class GORoundCounter {
private:
  /** The number of the round currently open */
  std::atomic<uint64_t> m_RoundNumber{0};

public:
  /** @return the number of the round currently open */
  uint64_t GetRoundNumber() const { return m_RoundNumber.load(); }

  /**
   * Opens the next round. Called by GOScheduler::NewRound() while it holds
   * its own mutex, so that the number changes together with the task states
   * it invalidates.
   *
   * Non-const while GetRoundNumber() is const, so that handing a task a
   * const GORoundCounter & makes "only the scheduler advances the round" a
   * property of the type rather than a convention.
   */
  void AdvanceRound() { m_RoundNumber.fetch_add(1); }
};

#endif /* GOROUNDCOUNTER_H */
