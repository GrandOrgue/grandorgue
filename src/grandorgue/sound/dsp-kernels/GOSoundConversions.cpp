/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundConversions.h"

#include <cmath>

float GOSoundConversions::gainToVolume(float gainDb) {
  assert(std::isfinite(gainDb));
  // The same expression as the hand-written copies it replaces, so the result
  // is bit for bit the same
  return powf(10.0f, gainDb * 0.05f);
}

float GOSoundConversions::gainToVolumeOrMute(float gainDb) {
  return gainDb >= MIN_GAIN_DB && gainDb <= MAX_GAIN_DB ? gainToVolume(gainDb)
                                                        : 0.0f;
}

double GOSoundConversions::centsToRatio(double cents) {
  assert(std::isfinite(cents));
  return std::exp2(cents / CENTS_PER_OCTAVE);
}

double GOSoundConversions::ratioToCents(double ratio) {
  assert(std::isfinite(ratio) && ratio > 0.0);
  return CENTS_PER_OCTAVE * std::log2(ratio);
}
