/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTEMPERAMENTFOLLOWER_H
#define GOTEMPERAMENTFOLLOWER_H

#include <array>
#include <functional>

#include <wx/string.h>

#include "GOTimerCallback.h"

class GOTemperament;
class GOTimer;

/**
 * Re-reads a live temperament (GOTemperament::IsLive()) periodically and
 * reports when its offsets have changed, so the organ can re-apply it, and
 * when its status (GOTemperament::GetStatus()) has changed, so the organ can
 * show it.
 *
 * It runs on the GOTimer of the organ, on the main thread. Follow() and Stop()
 * may be called any number of times; Stop() must be called before the
 * temperament or the timer is destroyed.
 */
class GOTemperamentFollower : public GOTimerCallback {
public:
  /**
   * Called with the followed temperament when its offsets or its status have
   * changed
   */
  using OnChangedFunction = std::function<void(const GOTemperament &)>;

  /**
   * How often the temperament is re-read, in milliseconds
   */
  static constexpr unsigned INTERVAL_MS = 100;

  /**
   * How many note numbers are compared: the range of an MTS-ESP master
   */
  static constexpr unsigned N_NOTES = 128;

private:
  GOTimer &r_timer;
  OnChangedFunction f_OnChanged;
  OnChangedFunction f_OnStatusChanged;
  const GOTemperament *p_temperament = nullptr;
  std::array<float, N_NOTES> m_offsets;
  wxString m_status;

  /**
   * Reads the offsets of the followed temperament for all note numbers
   */
  void ReadOffsets(std::array<float, N_NOTES> &outOffsets) const;

public:
  /**
   * Creates a follower that runs on the timer and calls fOnChanged when the
   * offsets change and fOnStatusChanged when the status changes
   */
  GOTemperamentFollower(
    GOTimer &timer,
    OnChangedFunction fOnChanged,
    OnChangedFunction fOnStatusChanged);
  /**
   * Asserts that nothing is followed any more
   */
  ~GOTemperamentFollower();

  /**
   * Follows the temperament if it is live, otherwise stops following
   */
  void Follow(const GOTemperament &temperament);

  /**
   * Stops following. Does nothing if nothing is followed
   */
  void Stop();

  /**
   * Whether a live temperament is being followed
   */
  bool IsFollowing() const { return p_temperament; }

  /**
   * Re-reads the followed temperament and calls f_OnChanged if any offset
   * differs from the last reading, and f_OnStatusChanged if the status
   * differs. Called by the timer
   */
  void HandleTimer() override;
};

#endif /* GOTEMPERAMENTFOLLOWER_H */
