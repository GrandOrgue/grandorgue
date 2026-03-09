/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDSTREAM_H
#define GOTESTSOUNDSTREAM_H

#include <memory>
#include <string>

#include "sound/playing/GOSoundResample.h"

#include "GOMemoryPool.h"
#include "GOTest.h"

class GOSoundAudioSection;
struct GOWaveLoop;

class GOTestSoundStream : public GOTest {
private:
  static const std::string TEST_NAME;

  GOMemoryPool m_pool;

  /**
   * Creates an audio section with nChannels channels and nFrames frames
   * of synthetic GOInt24 PCM data at 48000 Hz sample rate.
   * If isCompressed is true, the data is stored in compressed format.
   * If pLoop is not null, the section is created with the given loop points.
   */
  std::unique_ptr<GOSoundAudioSection> CreateAudioSection(
    unsigned nChannels,
    unsigned nFrames,
    bool isCompressed,
    const GOWaveLoop *pLoop = nullptr);

  /**
   * Tests ReadBlock for the given interpolation, compression and channel
   * combination. Verifies true while frames available, false on exhaustion,
   * and zeros after end.
   */
  void TestReadBlock(
    GOSoundResample::InterpolationType interpolationType,
    bool isCompressed,
    unsigned nChannels);

  /**
   * Tests that ReadBlock always returns true for a looped stream,
   * running 100 iterations of 50 frames each.
   */
  void TestLoopedStreamAlwaysReturnsTrue();

  /**
   * Tests InitAlignedStream with real release alignment.
   * Verifies that ReadBlock returns true after alignment.
   */
  void TestInitAlignedStream();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDSTREAM_H */
