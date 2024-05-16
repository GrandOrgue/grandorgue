/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundToneBalanceFilter.h"
#include <cmath>

void GOSoundToneBalanceFilter::Init(int8_t value) {
  if (value == 0)
    m_filter.Init(GOSoundFilter::FilterType::TYPE_NONE, 0);
  else {
    double hz;
    GOSoundFilter::FilterType type;

    if (value < 0) {
      type = GOSoundFilter::FilterType::TYPE_LPF;
      hz = 16000 * pow(20.0 / 16000.0, abs(value) / 99.0);
    } else {
      type = GOSoundFilter::FilterType::TYPE_HPF;
      hz = 20 * pow(800.0, value / 99.0);
    }

    m_filter.Init(type, hz);
  }
}
