/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSCHEDULERTASK_H
#define GOSCHEDULERTASK_H

class GOSchedulerThread;

/**
 * An abstract unit of work executed by GOScheduler.
 *
 * GOScheduler groups tasks into rounds. A round is one pass through all
 * registered tasks, driven by the owner from a single thread (for the sound
 * engine, once per audio period) while GOSchedulerThread workers pull and
 * Run() individual tasks concurrently in the background. For one round, the
 * owner calls, in order:
 *  - Run() — zero or more times, from any worker thread, while there is
 *    still time in the round. A task may be picked up by several threads
 *    and may itself trigger Run() on its own prerequisites (see
 *    GOSoundBufferTaskBase::EnsureBufferReady()), so Run() must be safe to
 *    call concurrently and must be a no-op once the task is done;
 *  - CompleteRound() — exactly once, from the owner thread, when the round's
 *    time budget is exhausted. Finishes whatever Run() left undone,
 *    synchronously and without waiting for anything;
 *  - NewRound() — exactly once, right after CompleteRound(), to reset the
 *    per-round state so the task is ready for Run() again in the next round.
 *
 * DiscardContent() is unrelated to this per-round cycle: GOScheduler::Clear()
 * calls it once, when the task is deregistered from the scheduler, to drop
 * any content the task has accumulated across many rounds (queued samplers,
 * reverb state, meter readings, an open file). GOScheduler::Add() asserts
 * IsEmpty() instead of discarding: a task must arrive at registration with
 * nothing left to drop.
 */
class GOSchedulerTask {
public:
  virtual ~GOSchedulerTask() {}

  /** Scheduling priority: a task with a smaller value is scheduled earlier */
  virtual unsigned GetPriority() const = 0;

  /** Estimated work amount; more expensive tasks are scheduled first */
  virtual unsigned GetCost() const = 0;

  /** Whether the task may be scheduled several times within one round */
  virtual bool IsRepeatable() const = 0;

  /** @return whether DiscardContent() would change nothing - i.e. the task
      has no accumulated content and is not mid-round */
  virtual bool IsEmpty() const = 0;

  /** Processes the task, possibly partially, cooperatively with other threads
   */
  virtual void Run(GOSchedulerThread *pThread = nullptr) = 0;

  /** Called at the round deadline: completes whatever is left immediately,
      without waiting for anything */
  virtual void CompleteRound() = 0;

  /** Prepares the task for the next round: drops the per-round state */
  virtual void NewRound() = 0;

  /** Drops everything the task has accumulated. Called when the task is
      removed from the scheduler */
  virtual void DiscardContent() = 0;
};

#endif
