/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDTASKBASE_H
#define GOSOUNDTASKBASE_H

#include <atomic>

#include "scheduler/GOSchedulerTask.h"
#include "threading/GOMutex.h"

class GOSchedulerThread;

/**
 * Common base for GOSchedulerTask implementations that belong to the sound
 * engine. Implements the round life cycle described in GOSchedulerTask
 * (Run() / CompleteRound() / NewRound()) once for every subclass: a task is
 * run at most once per round (tracked by m_RunState) under m_mutex, and
 * CompleteRound()/NewRound() maintain m_IsToComplete for subclasses that
 * need to know the round deadline was reached.
 *
 * A subclass that fits the default one-shot Run() protocol only needs to
 * override DoRun() (the actual work) and, if it accumulates state across
 * rounds, DoNewRound(). The default Run() only ever moves m_RunState between
 * RUN_STATE_NOT_STARTED and RUN_STATE_DONE.
 *
 * A subclass with a fundamentally different execution protocol overrides
 * Run() and/or CompleteRound() directly instead of DoRun():
 *  - GOSoundReleaseTask is a drain loop that is never RUN_STATE_DONE within
 *    a round;
 *  - GOSoundGroupTask is a *cooperative* task: the scheduler may hand the
 *    same task to several worker threads at once (it is IsRepeatable()), and
 *    all of them race into Run() to jointly process one shared sampler list
 *    for this round, each contributing to one shared result buffer. This is
 *    what the two extra RunState values, RUN_STATE_IN_PROGRESS and
 *    RUN_STATE_PARTLY_DONE, are for (see RunState below) - m_mutex still
 *    serialises the bookkeeping around each thread's own chunk of work, but
 *    not the work itself, so several threads make progress in parallel
 *    instead of one thread doing all the work while the others wait;
 *  - GOSoundTouchTask keeps the default DoRun() protocol but overrides
 *    CompleteRound(): its DoRun() always returns false (there is always more
 *    idle memory to touch), so the default CompleteRound() - one Run() call
 *    - would leave it perpetually not-done, which is fine for Run() but not
 *    what CompleteRound() is for here; it only needs to make sure no other
 *    thread is still inside DoRun() when the round ends, so it takes m_mutex
 *    directly instead.
 */
class GOSoundTaskBase : public GOSchedulerTask {
public:
  /**
   * Scheduling priority of a sound task: a task with a smaller value is
   * scheduled earlier. Tasks with the same value form a batch that the
   * scheduler emits together. The order is an optimisation only: a task
   * lazily runs its prerequisites itself (see EnsureBufferReady,
   * GOSoundWindchestTask::GetVolume).
   */
  enum TaskPriority {
    /** Tremulant oscillation, recomputed before the windchests read it */
    PRIORITY_TREMULANT = 10,
    /** Per-windchest enclosure/tremulant volume, read lazily by pipes */
    PRIORITY_WINDCHEST = 20,
    /** Mixing the active samplers of one audio group into its buffer */
    PRIORITY_AUDIOGROUP = 50,
    /** Mixing audio groups into the final device output, reverb, clipping */
    PRIORITY_AUDIOOUTPUT = 100,
    /** Writing the mixed output to the recording file */
    PRIORITY_AUDIORECORDER = 150,
    /** Draining samplers that have finished their release */
    PRIORITY_RELEASE = 160,
    /** Touching pooled memory pages while otherwise idle */
    PRIORITY_TOUCH = 700,
  };

  /**
   * The processing state of the task within the current round.
   *
   * A regular, non-cooperative task (the default Run()/DoRun() protocol)
   * only ever uses RUN_STATE_NOT_STARTED and RUN_STATE_DONE:
   * RUN_STATE_NOT_STARTED until some thread calls DoRun() successfully,
   * RUN_STATE_DONE from then on for the rest of the round.
   *
   * A cooperative task (GOSoundGroupTask, which overrides Run() itself)
   * additionally uses RUN_STATE_IN_PROGRESS and RUN_STATE_PARTLY_DONE to let
   * several worker threads process the same round together instead of one
   * thread doing it alone while the rest wait idle:
   *  - RUN_STATE_NOT_STARTED -> RUN_STATE_IN_PROGRESS: the first thread to
   *    enter Run() claims the round (e.g. moves the pending sampler lists
   *    into a working list) so that later threads joining in know there is
   *    a round to help with;
   *  - each thread (including the first) then does its own share of the
   *    work concurrently, outside m_mutex, into a private/local result;
   *  - RUN_STATE_IN_PROGRESS -> RUN_STATE_PARTLY_DONE: whichever thread is
   *    first to finish its own share stores it as the round's shared
   *    result; every other thread that finishes afterwards merges its own
   *    share into that shared result instead of overwriting it (both under
   *    m_mutex, so the merge itself is not a race - only the work that
   *    produced each share is parallel);
   *  - RUN_STATE_PARTLY_DONE -> RUN_STATE_DONE: once the last thread to be
   *    doing work for this round finishes (tracked separately, e.g. by an
   *    active-worker counter reaching zero), the round's shared result is
   *    final and anyone still waiting on it (e.g. via a condition variable)
   *    is woken.
   */
  enum RunState {
    /** No thread has started processing this round yet */
    RUN_STATE_NOT_STARTED = 0,
    /** A cooperative round has been claimed and threads are working on it */
    RUN_STATE_IN_PROGRESS = 1,
    /** A cooperative round has a shared result but some threads are still
     * merging their share into it */
    RUN_STATE_PARTLY_DONE = 2,
    /** The round is fully processed and its result may be read */
    RUN_STATE_DONE = 3,
  };

private:
  /** This task's scheduling priority, see TaskPriority */
  const TaskPriority m_priority;

  /** Whether this task may be scheduled several times within one round */
  const bool m_IsRepeatable;

protected:
  /** Serialises DoRun()/CompleteRound()/NewRound() for this task */
  GOMutex m_mutex;

  /** Set by CompleteRound() at the round deadline: the task must complete
   * immediately and must not wait. Cleared by NewRound() */
  std::atomic_bool m_IsToComplete;

  /** This task's processing state for the current round, see RunState */
  std::atomic<RunState> m_RunState;

  /**
   * Does the actual work of the task. Called from Run() under m_mutex, only
   * if the task is not yet RUN_STATE_DONE.
   *
   * WARNING: the default CompleteRound() below calls this (via Run()) only
   * once. A DoRun() whose false return is not guaranteed to flip to true
   * within that one remaining call - whether because it is a genuinely
   * incremental protocol or because it may legitimately have nothing to do
   * for an entire round (idle; a feature currently off) - must not rely on
   * the default CompleteRound(): override CompleteRound() itself instead,
   * the way GOSoundTouchTask does. Discovered the hard way: an earlier
   * version of this class had CompleteRound() loop until IsDone() instead,
   * which deadlocked GOSoundRecorderTask (false forever while not recording)
   * and GOSoundReleaseTask (calls this CompleteRound() as a plain
   * "run once" helper from its own CompleteRound() override, while never
   * touching m_RunState at all, so IsDone() there can never become true).
   * @param pThread the calling worker thread, if any
   * @return true if the round is now fully processed (m_RunState becomes
   *   RUN_STATE_DONE); false if Run() may be called again within the same
   *   round
   */
  virtual bool DoRun(GOSchedulerThread *pThread) { return true; }

  /** Resets subclass-specific per-round state. Called from NewRound() under
   * m_mutex */
  virtual void DoNewRound() {}

public:
  /**
   * @param priority this task's scheduling priority, see TaskPriority. Not
   *   yet used by every subclass: some still override GetPriority()
   *   themselves and ignore this value until they are switched to rely on
   *   the base implementation
   * @param isRepeatable whether the task may be scheduled several times
   *   within one round, see GOSchedulerTask::IsRepeatable(). Same caveat as
   *   priority above
   */
  GOSoundTaskBase(
    TaskPriority priority = PRIORITY_AUDIOGROUP, bool isRepeatable = false);

  /** @return this task's scheduling priority, see TaskPriority */
  unsigned GetPriority() const override { return m_priority; }

  /** @return whether this task may be scheduled several times per round */
  bool IsRepeatable() const override { return m_IsRepeatable; }

  /** @return the estimated cost of this task; 0 unless a subclass knows
   * better */
  unsigned GetCost() const override { return 0; }

  /** @return whether the task has fully processed the current round */
  bool IsDone() const { return m_RunState.load() == RUN_STATE_DONE; }

  /** Calls DoRun() at most once per round, cooperatively with other threads
   */
  void Run(GOSchedulerThread *pThread = nullptr) override;

  /** Sets m_IsToComplete and calls Run() once. Does not guarantee IsDone()
   * afterwards unless DoRun() does - see the warning on DoRun() */
  void CompleteRound() override;

  /** Resets the round state to RUN_STATE_NOT_STARTED and calls
   * DoNewRound(). Not meant to be overridden by subclasses - extend via
   * DoNewRound() instead */
  void NewRound() override;

  /** Equivalent to NewRound(): a fresh task has nothing accumulated to
   * discard beyond its round state */
  void DiscardContent() override { NewRound(); }
};

#endif /* GOSOUNDTASKBASE_H */
