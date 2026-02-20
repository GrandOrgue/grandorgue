/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDBUFFERBASE_H
#define GOTESTSOUNDBUFFERBASE_H

#include <string>

#include "sound/buffer/GOSoundBuffer.h"

#include "GOTest.h"

/** Base class for GOSoundBuffer tests providing common helpers. */
class GOTestSoundBufferBase : public GOTest {
protected:
  /** Fills data[i] = static_cast<float>(i) + offset for i in [0, nItems). */
  static void fillWithSequential(
    GOSoundBuffer::Item *data, unsigned nItems, float offset);

  /** Asserts gotValue == expectedValue, reporting item index on failure. */
  void AssertItemEqual(
    const std::string &context,
    unsigned itemIndex,
    float expectedValue,
    float gotValue);

  /** Asserts data[i] == static_cast<float>(i) + offset for all i. */
  void AssertSequentialData(
    const std::string &context,
    const GOSoundBuffer::Item *data,
    unsigned nItems,
    float offset);

  /** Asserts buffer dimensions and validity match expected values. */
  void AssertDimensions(
    const std::string &context,
    const GOSoundBuffer &buffer,
    unsigned expectedNChannels,
    unsigned expectedNFrames);
};

#endif /* GOTESTSOUNDBUFFERBASE_H */
