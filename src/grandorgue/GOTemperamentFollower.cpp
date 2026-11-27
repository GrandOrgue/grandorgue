/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTemperamentFollower.h"

#include <cassert>

#include "GOTimer.h"
#include "temperaments/GOTemperament.h"

GOTemperamentFollower::GOTemperamentFollower(
  GOTimer &timer,
  OnChangedFunction fOnChanged,
  OnChangedFunction fOnStatusChanged)
  : r_timer(timer),
    f_OnChanged(fOnChanged),
    f_OnStatusChanged(fOnStatusChanged) {
  m_offsets.fill(0);
}

GOTemperamentFollower::~GOTemperamentFollower() { assert(!p_temperament); }

void GOTemperamentFollower::ReadOffsets(
  std::array<float, N_NOTES> &outOffsets) const {
  for (unsigned noteI = 0; noteI < N_NOTES; ++noteI)
    outOffsets[noteI] = p_temperament->GetOffset(noteI);
}

void GOTemperamentFollower::Follow(const GOTemperament &temperament) {
  Stop();
  if (temperament.IsLive()) {
    p_temperament = &temperament;
    ReadOffsets(m_offsets);
    m_status = temperament.GetStatus();
    r_timer.SetRelativeTimer(INTERVAL_MS, this, INTERVAL_MS);
  }
}

void GOTemperamentFollower::Stop() {
  if (p_temperament) {
    r_timer.DeleteTimer(this);
    p_temperament = nullptr;
  }
}

void GOTemperamentFollower::HandleTimer() {
  if (p_temperament) {
    std::array<float, N_NOTES> offsets;

    ReadOffsets(offsets);
    if (offsets != m_offsets) {
      m_offsets = offsets;
      f_OnChanged(*p_temperament);
    }

    // a master connecting with the equal temperament changes no offset, so
    // the status is compared on its own
    const wxString status = p_temperament->GetStatus();

    if (status != m_status) {
      m_status = status;
      f_OnStatusChanged(*p_temperament);
    }
  }
}
