/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundOnePoleFilter.h"

#include <cmath>

void GOSoundOnePoleFilter::computeCoeffs(
  Type type,
  double frequency,
  double gain,
  unsigned sampleRate,
  Coeffs &outCoeffs) {
  if (
    sampleRate == 0 || type == Type::TYPE_NONE
    || ((type == Type::TYPE_LOW_SHELF || type == Type::TYPE_HIGH_SHELF) && gain == 0))
    outCoeffs = Coeffs();
  else {
    double a0 = 1;
    double a1 = 0;
    double b0 = 0;
    double b1 = 0;
    double amp = pow(10, (gain / 40.0));
    double w0 = 2.0 * 3.14159265358979323846 * frequency / sampleRate;
    double cosW0 = cos(w0);
    double sinW0 = sin(w0);

    switch (type) {
    case Type::TYPE_LPF:
      b0 = sinW0;
      b1 = sinW0;
      a0 = sinW0 + cosW0 + 1.0;
      a1 = (sinW0 - cosW0 - 1.0);
      break;
    case Type::TYPE_HPF:
      b0 = 1.0 + cosW0;
      b1 = -(1.0 + cosW0);
      a0 = sinW0 + cosW0 + 1.0;
      a1 = (sinW0 - cosW0 - 1.0);
      break;
    case Type::TYPE_LOW_SHELF:
      b0 = amp * sinW0 + cosW0 + 1.0;
      b1 = amp * sinW0 - cosW0 - 1.0;
      a0 = 1.0 / amp * sinW0 + cosW0 + 1.0;
      a1 = 1.0 / amp * sinW0 - cosW0 - 1.0;
      break;
    case Type::TYPE_HIGH_SHELF:
      b0 = sinW0 + amp + amp * cosW0;
      b1 = sinW0 - amp - amp * cosW0;
      a0 = sinW0 + 1.0 / amp + 1.0 / amp * cosW0;
      a1 = sinW0 - 1.0 / amp - 1.0 / amp * cosW0;
      break;
    default:
      break;
    }
    outCoeffs.b0 = b0 / a0;
    outCoeffs.b1 = b1 / a0;
    outCoeffs.a1 = a1 / a0;
    outCoeffs.isNoop = false;
  }
}
