/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDSTREAM_H
#define GOTESTSOUNDSTREAM_H

#include <memory>
#include <string>

#include "sound/dsp-kernels/GOSoundResample.h"

#include "GOMemoryPool.h"
#include "GOTest.h"
#include "GOWave.h"

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
   * If isSilent is true, the PCM data only contains quantization-noise-level
   * values (a "BlankLoop"-like placeholder) instead of the usual sawtooth.
   * @param loopCrossfadeMs loop crossfade length in milliseconds
   */
  std::unique_ptr<GOSoundAudioSection> CreateAudioSection(
    unsigned nChannels,
    unsigned nFrames,
    bool isCompressed,
    const GOWaveLoop *pLoop = nullptr,
    bool isSilent = false,
    unsigned loopCrossfadeMs = 0);

  /**
   * Creates an audio section from samples of the given format at 48000 Hz.
   * The items are typed by the format (float for SF_IEEE_FLOAT), so the data
   * is passed as void *, as GOSoundAudioSection::Setup() takes it.
   * @param pPcmData interleaved samples, nChannels * nFrames items
   * @param format the format of the items
   * @param nChannels number of channels, 1 or 2
   * @param nFrames number of frames
   * @param isCompressed whether the data is stored in compressed format
   * @param pLoop if not null, the loop points of the section
   * @param loopCrossfadeMs loop crossfade length in milliseconds
   */
  std::unique_ptr<GOSoundAudioSection> CreateAudioSection(
    const void *pPcmData,
    GOWave::SAMPLE_FORMAT format,
    unsigned nChannels,
    unsigned nFrames,
    bool isCompressed,
    const GOWaveLoop *pLoop = nullptr,
    unsigned loopCrossfadeMs = 0);

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
   * Tests that wrap-around between loop segments with different end_pos
   * keeps the source position consistent. Catches a regression where
   * ReadBlock used the new segment's end_pos instead of the previous one
   * when computing the post-wrap position.
   */
  void TestLoopTransitionAcrossDifferentEndPos();

  /**
   * Tests InitAlignedStream with real release alignment.
   * Verifies that ReadBlock returns true after alignment.
   */
  void TestInitAlignedStream();

  /**
   * Regression test for an off-by-one in
   * StreamCacheReadAheadWindow::Seek() (GOSoundStream.cpp): after a loop
   * wraps to a new start segment, the compressed decoder resumes from a
   * precomputed cache snapshot pinned at that segment's start_offset, while
   * the resampling position typically requests a few samples further in
   * (the wrap remainder) - a genuine skip-ahead Seek() on an otherwise-fresh
   * cache. Using ">= index" instead of "> index" to decide whether a freshly
   * decoded sample should be stored shifted every FIR tap by one sample
   * until it self-healed after windowLen samples.
   *
   * Verified differentially: with LINEAR interpolation and factor 1.0
   * (identity), decoding is lossless, so a compressed and an uncompressed
   * section with identical PCM data and identical loop layout must produce
   * bit-identical ReadBlock() output for every frame, including immediately
   * after a wrap - no assumption about "realistic" sample values is needed.
   */
  void TestCompressedLoopWrapMatchesUncompressed();

  /** Compares bandlimited short-loop playback and a release across a retrigger.
   */
  void TestBandlimitedLoopAndRetrigger();

  /** Measures the 3 kHz, +50-cent image through all three playback modes. */
  void TestRetuningSuppressesImages();

  /** Checks that native-rate playback starts at frame 0 in every mode. */
  void TestNativeRateStartsAtFirstFrame();

  /**
   * Tests that a silent ("BlankLoop"-like) attack is not used for release
   * alignment: SetupStreamAlignment must leave the release aligner null so
   * that playback falls back to the release's natural start position.
   */
  void TestInitAlignedStreamWithSilentAttack();

  /** Compares raw and compressed release alignment across segment boundaries.
   */
  void TestReleaseAlignmentMatchesUncompressed();

  /**
   * Checks that GetHistory() of a float attack, uncompressed and compressed,
   * is the same as that of the same data stored as integers.
   */
  void TestFloatHistoryMatchesInt();

  /**
   * Checks that a float section, uncompressed and compressed, is read the same
   * after SaveCache() and LoadCache().
   */
  void TestFloatCacheRoundTrip();

  /**
   * Checks that GOSoundProviderWave loads a stereo float file with its own
   * depth as a float section and with 24 bits as an integer one.
   */
  void TestProviderLoadsFloatWave();

  /** Checks short releases and note-offs before the window leaves the prologue.
   */
  void TestReleaseAlignmentFallsBackToStart();

  /** Checks start segment numbering after dropping a loop. */
  void TestRejectedLoopKeepsStartSegmentIndex();

  /** Checks that an uncompressed float section is read as it is. */
  void TestFloatRead();

  /**
   * Checks that a looped float section read with the polyphase interpolation
   * matches the same data stored as integers.
   */
  void TestFloatPolyphaseLoopMatchesInt();

  /**
   * Checks that a compressed float loop, including its end segment, is read
   * exactly as the same data stored as integers.
   */
  void TestCompressedFloatMatchesInt();

  /**
   * Checks that the loop crossfade of an uncompressed float section is
   * computed in float: values beyond full scale are not clamped and values
   * below the Q23 resolution are not rounded.
   */
  void TestFloatLoopCrossfadeKeepsFloats();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDSTREAM_H */
