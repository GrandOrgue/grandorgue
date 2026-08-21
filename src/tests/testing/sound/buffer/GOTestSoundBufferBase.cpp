/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundBufferBase.h"

#include <format>

void GOTestSoundBufferBase::fillWithSequential(
  GOSoundBuffer::Item *data, unsigned nItems, float offset) {
  for (unsigned itemI = 0; itemI < nItems; ++itemI)
    data[itemI] = static_cast<float>(itemI) + offset;
}

void GOTestSoundBufferBase::AssertItemEqual(
  const std::string &context,
  unsigned itemIndex,
  float expectedValue,
  float gotValue) {
  GOAssert(
    gotValue == expectedValue,
    std::format(
      "{}: item {} got: {}, expected: {}",
      context,
      itemIndex,
      gotValue,
      expectedValue));
}
