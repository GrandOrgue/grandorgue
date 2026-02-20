/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundBufferBase.h"

#include <format>

#include "sound/buffer/GOSoundBufferMutable.h"

void GOTestSoundBufferBase::fillWithSequential(
  GOSoundBuffer::Item *data, unsigned nItems, float offset) {
  for (unsigned itemI = 0; itemI < nItems; ++itemI)
    data[itemI] = static_cast<float>(itemI) + offset;
}

void GOTestSoundBufferBase::fillWithSequential(
  GOSoundBufferMutable &buffer, float offset) {
  fillWithSequential(buffer.GetData(), buffer.GetNItems(), offset);
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

void GOTestSoundBufferBase::AssertSequentialData(
  const std::string &context, const GOSoundBuffer &buffer, float offset) {
  for (unsigned n = buffer.GetNItems(), itemI = 0; itemI < n; ++itemI)
    AssertItemEqual(
      context,
      itemI,
      static_cast<float>(itemI) + offset,
      buffer.GetData()[itemI]);
}

void GOTestSoundBufferBase::AssertDimensions(
  const std::string &context,
  const GOSoundBuffer &buffer,
  unsigned expectedNChannels,
  unsigned expectedNFrames) {
  const bool expectedValid = expectedNChannels > 0 && expectedNFrames > 0;

  GOAssert(
    buffer.isValid() == expectedValid,
    std::format(
      "{}: buffer should be {}", context, expectedValid ? "valid" : "invalid"));
  GOAssert(
    buffer.GetNChannels() == expectedNChannels,
    std::format(
      "{}: buffer should have {} channels (got: {})",
      context,
      expectedNChannels,
      buffer.GetNChannels()));
  GOAssert(
    buffer.GetNFrames() == expectedNFrames,
    std::format(
      "{}: buffer should have {} frames (got: {})",
      context,
      expectedNFrames,
      buffer.GetNFrames()));
}
