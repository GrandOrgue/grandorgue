/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundFilter.h"

GOSoundFilter::GOSoundFilter() {
  m_samplerate = 0;
  Init(GO_FILTER_TYPE::LPF, 0, 0);
}

void GOSoundFilter::Init(GO_FILTER_TYPE type, double frequency, double gain) {
  m_type = type;
  m_B0 = 0;
  m_B1 = 0;
  m_A1 = 0;

  if (m_samplerate == 0) {
    m_isToApply = false;
    return;
  } else {
    double a0 = 1;
    double a1 = 0;
    double b0 = 0;
    double b1 = 0;
    double amp = pow(10, (gain / 40.0));
    double w0 = 2.0 * 3.14159265358979323846 * frequency / m_samplerate;
    double cosW0 = cos(w0);
    double sinW0 = sin(w0);

    switch (m_type) {
    case GO_FILTER_TYPE::LPF:
      b0 = sinW0;
      b1 = sinW0;
      a0 = sinW0 + cosW0 + 1.0;
      a1 = (sinW0 - cosW0 - 1.0);
      break;
    case GO_FILTER_TYPE::HPF:
      b0 = 1.0 + cosW0;
      b1 = -(1.0 + cosW0);
      a0 = sinW0 + cosW0 + 1.0;
      a1 = (sinW0 - cosW0 - 1.0);
      break;
    case GO_FILTER_TYPE::LOW_SHELF:
      b0 = amp * sinW0 + cosW0 + 1.0;
      b1 = amp * sinW0 - cosW0 - 1.0;
      a0 = 1.0 / amp * sinW0 + cosW0 + 1.0;
      a1 = 1.0 / amp * sinW0 - cosW0 - 1.0;
      break;
    case GO_FILTER_TYPE::HIGH_SHELF:
      b0 = sinW0 + amp + amp * cosW0;
      b1 = sinW0 - amp - amp * cosW0;
      a0 = sinW0 + 1.0 / amp + 1.0 / amp * cosW0;
      a1 = sinW0 - 1.0 / amp - 1.0 / amp * cosW0;
      break;
    default:
      break;
    }
    m_B0 = b0 / a0;
    m_B1 = b1 / a0;
    m_A1 = a1 / a0;
  }
}
