/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundStream.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <format>
#include <numbers>
#include <vector>

#include <wx/file.h>
#include <wx/filename.h>
#include <wx/wfstream.h>

#include "config/GOConfig.h"
#include "loader/GOFileStore.h"
#include "loader/cache/GOCache.h"
#include "loader/cache/GOCacheWriter.h"
#include "sound/buffer/GOSoundBufferMutable.h"
#include "sound/playing/GOSoundAudioSection.h"
#include "sound/playing/GOSoundStream.h"
#include "sound/providers/GOSoundProviderWave.h"
#include "testing/sound/GOTestWaveFileBuilder.h"

#include "GOInt.h"
#include "GOTestScope.h"
#include "GOWave.h"
#include "GOWaveLoop.h"

const std::string GOTestSoundStream::TEST_NAME = "GOTestSoundStream";

static constexpr unsigned SECTION_RATE = 48000;
static constexpr float SAMPLE_RATE_ADJUSTMENT = 1.0f / SECTION_RATE;
// The resampling increment for a section of SECTION_RATE: the same rate
static constexpr unsigned FRACTION_INCREMENT
  = GOSoundResample::rateToFractionIncrement(
    SAMPLE_RATE_ADJUSTMENT * SECTION_RATE);
static constexpr unsigned N_FRAMES = 500;
static constexpr unsigned N_ATTACK_FRAMES = 100;
static constexpr unsigned N_FRAMES_PER_BLOCK = 50;

std::unique_ptr<GOSoundAudioSection> GOTestSoundStream::CreateAudioSection(
  unsigned nChannels,
  unsigned nFrames,
  bool isCompressed,
  const GOWaveLoop *pLoop,
  bool isSilent,
  unsigned loopCrossfadeMs) {
  const unsigned nSamples = nChannels * nFrames;
  std::vector<GOInt24> pcmData(nSamples);

  if (isSilent)
    for (unsigned i = 0; i < nSamples; i++)
      pcmData[i] = (i % 2) ? 1 : -1;
  else
    for (unsigned i = 0; i < nSamples; i++)
      // Keep the per-channel slope compressible for stereo fixtures too.
      pcmData[i] = (int(i % 100) * 30000 - 1500000) / int(nChannels);

  return CreateAudioSection(
    pcmData.data(),
    GOWave::SF_SIGNEDINT24_24,
    nChannels,
    nFrames,
    isCompressed,
    pLoop,
    loopCrossfadeMs);
}

std::unique_ptr<GOSoundAudioSection> GOTestSoundStream::CreateAudioSection(
  const void *pPcmData,
  GOWave::SAMPLE_FORMAT format,
  unsigned nChannels,
  unsigned nFrames,
  bool isCompressed,
  const GOWaveLoop *pLoop,
  unsigned loopCrossfadeMs) {
  std::vector<GOWaveLoop> loops;
  auto pSection = std::make_unique<GOSoundAudioSection>(m_pool);

  if (pLoop)
    loops.push_back(*pLoop);
  pSection->Setup(
    nullptr,
    nullptr,
    pPcmData,
    format,
    nChannels,
    SECTION_RATE,
    nFrames,
    &loops,
    BOOL3_DEFAULT,
    isCompressed,
    loopCrossfadeMs,
    0);
  return pSection;
}

void GOTestSoundStream::TestReadBlock(
  GOSoundResample::InterpolationType interpolationType,
  bool isCompressed,
  unsigned nChannels) {
  const std::string label = std::format(
    "interpolation={} compressed={} channels={}",
    unsigned(interpolationType),
    isCompressed,
    nChannels);
  const bool isBandlimited
    = interpolationType == GOSoundResample::GO_BANDLIMITED_INTERPOLATION;
  const unsigned increment
    = isBandlimited ? FRACTION_INCREMENT * 5 / 4 : FRACTION_INCREMENT;
  const unsigned centrePoint
    = GOSoundResample::getCentrePoint(interpolationType);
  const auto pSection = CreateAudioSection(nChannels, N_FRAMES, isCompressed);
  GOSoundResample resample;
  GOSoundResample::ResamplingPosition startPosition;
  GOSoundStream stream;

  GOAssert(pSection->GetLength() == N_FRAMES, "Length excludes the prologue");
  startPosition.Init();
  const unsigned nOutputFrames
    = startPosition.AvailableTargetSamples(N_FRAMES + centrePoint, increment);

  stream.InitStream(&resample, pSection.get(), interpolationType);

  GO_DECLARE_LOCAL_SOUND_BUFFER(buffer, 2, N_FRAMES_PER_BLOCK)

  for (unsigned frameI = 0; frameI + N_FRAMES_PER_BLOCK <= nOutputFrames;
       frameI += N_FRAMES_PER_BLOCK) {
    const bool result = stream.ReadBlock(increment, buffer);

    GOAssert(
      result,
      std::format(
        "ReadBlock should return true while frames available ({})", label));
  }

  const unsigned nRemainingFrames = nOutputFrames % N_FRAMES_PER_BLOCK;

  if (nRemainingFrames) {
    auto tail = buffer.GetSubBuffer(0, nRemainingFrames);

    GOAssert(
      stream.ReadBlock(increment, tail),
      std::format(
        "ReadBlock should render the last partial block ({})", label));
  }

  // ReadBlock should return false after exhaustion
  const bool resultAfterEnd = stream.ReadBlock(increment, buffer);

  GOAssert(
    !resultAfterEnd,
    std::format("ReadBlock should return false after exhaustion ({})", label));

  // Buffer should be filled with zeros after end
  const float *pData = buffer.GetData();

  for (unsigned n = buffer.GetNItems(), i = 0; i < n; i++)
    GOAssert(
      pData[i] == 0.0f,
      std::format(
        "Buffer[{}] should be 0.0f after end ({}, got: {})",
        i,
        label,
        pData[i]));

  // TODO: verify decoded frame values
}

void GOTestSoundStream::TestLoopedStreamAlwaysReturnsTrue() {
  constexpr unsigned N_ITERATIONS = 100;
  const GOWaveLoop loop = {100, 499};
  const auto pSection = CreateAudioSection(1, N_FRAMES, false, &loop);
  GOSoundResample resample;
  GOSoundStream stream;

  stream.InitStream(
    &resample, pSection.get(), GOSoundResample::GO_LINEAR_INTERPOLATION);

  GO_DECLARE_LOCAL_SOUND_BUFFER(buffer, 2, N_FRAMES_PER_BLOCK)

  for (unsigned iterI = 0; iterI < N_ITERATIONS; iterI++) {
    const bool result = stream.ReadBlock(FRACTION_INCREMENT, buffer);

    GOAssert(
      result,
      std::format(
        "ReadBlock should always return true for looped stream (iteration {})",
        iterI));
  }
}

void GOTestSoundStream::TestInitAlignedStream() {
  const auto pAttack = CreateAudioSection(1, N_FRAMES, false);
  const auto pRelease = CreateAudioSection(1, N_FRAMES, false);

  pRelease->SetupStreamAlignment({pAttack.get()}, 0);

  GOSoundResample resample;
  GOSoundStream attackStream;

  attackStream.InitStream(
    &resample, pAttack.get(), GOSoundResample::GO_LINEAR_INTERPOLATION);

  GO_DECLARE_LOCAL_SOUND_BUFFER(buffer, 2, N_FRAMES_PER_BLOCK)

  for (unsigned frameI = 0; frameI < N_ATTACK_FRAMES;
       frameI += N_FRAMES_PER_BLOCK)
    attackStream.ReadBlock(FRACTION_INCREMENT, buffer);

  GOSoundStream releaseStream;

  releaseStream.InitAlignedStream(
    pRelease.get(), GOSoundResample::GO_LINEAR_INTERPOLATION, &attackStream);

  const bool result = releaseStream.ReadBlock(FRACTION_INCREMENT, buffer);

  GOAssert(result, "ReadBlock should return true after InitAlignedStream");
}

void GOTestSoundStream::TestInitAlignedStreamWithSilentAttack() {
  const auto pSilentAttack
    = CreateAudioSection(1, N_FRAMES, false, nullptr, true);
  const auto pNormalAttack = CreateAudioSection(1, N_FRAMES, false);
  const auto pRelease = CreateAudioSection(1, N_FRAMES, false);

  GOAssert(
    pSilentAttack->IsEssentiallySilent(),
    "A BlankLoop-like near-zero attack should be detected as essentially "
    "silent");
  GOAssert(
    !pNormalAttack->IsEssentiallySilent(),
    "A normal attack with real amplitude should not be detected as "
    "essentially silent");

  // GOSoundProvider::ComputeReleaseAlignmentInfo excludes essentially silent
  // attacks from the joinables it passes to SetupStreamAlignment; simulate
  // that filtering here and confirm the release aligner stays null, so
  // playback falls back to the release's natural start position instead of
  // matching quantization noise inside the release.
  pRelease->SetupStreamAlignment({}, 0);

  GOAssert(
    !pRelease->SupportsStreamAlignment(),
    "A release should have no aligner when the only joinable attack is "
    "silent");
}

void GOTestSoundStream::TestReleaseAlignmentMatchesUncompressed() {
  using Resample = GOSoundResample;
  constexpr unsigned PROLOGUE = GOSoundAudioSection::PROLOGUE_LENGTH;
  const GOSoundResample resample;
  const unsigned transitionOffset
    = PROLOGUE + N_FRAMES - GOSoundAudioSection::getMaxReadAhead();

  for (unsigned nChannels : {1u, 2u}) {
    const auto pRaw = CreateAudioSection(nChannels, N_FRAMES, false);
    const auto pCompressed = CreateAudioSection(nChannels, N_FRAMES, true);
    const auto pRelease = CreateAudioSection(nChannels, 4096, false);

    GOAssert(pCompressed->IsCompressed(), "The attack must use compression");
    pRelease->SetupStreamAlignment({pRaw.get()}, 0);
    GOAssert(
      pRelease->SupportsStreamAlignment(), "The release must be alignable");

    auto captureRelease = [&](
                            const GOSoundAudioSection &section,
                            GOSoundResample::InterpolationType mode,
                            unsigned nAttackFrames) {
      GOSoundStream attack;
      GOSoundStream release;
      GO_DECLARE_LOCAL_SOUND_BUFFER(attackBuffer, 2, nAttackFrames)
      GO_DECLARE_LOCAL_SOUND_BUFFER(releaseBuffer, 2, N_FRAMES_PER_BLOCK)

      attack.InitStream(&resample, &section, mode);
      GOAssert(
        attack.ReadBlock(FRACTION_INCREMENT, attackBuffer), "Attack played");
      release.InitAlignedStream(pRelease.get(), mode, &attack);
      GOAssert(
        release.ReadBlock(FRACTION_INCREMENT, releaseBuffer), "Release played");
      return std::vector<float>(
        releaseBuffer.GetData(),
        releaseBuffer.GetData() + releaseBuffer.GetNItems());
    };

    // Exercise a window in the main data, a history straddling the segment
    // boundary and a history entirely in the end segment.
    for (const auto mode :
         {Resample::GO_LINEAR_INTERPOLATION,
          Resample::GO_POLYPHASE_INTERPOLATION,
          Resample::GO_BANDLIMITED_INTERPOLATION})
      for (unsigned windowStart :
           {PROLOGUE + 83u, transitionOffset - 1, PROLOGUE + N_FRAMES - 3}) {
        const unsigned nAttackFrames
          = windowStart - (PROLOGUE - Resample::getCentrePoint(mode));

        GOAssert(
          captureRelease(*pRaw, mode, nAttackFrames)
            == captureRelease(*pCompressed, mode, nAttackFrames),
          std::format(
            "Release must match across decoders (mode {}, {} channels, window "
            "start {})",
            unsigned(mode),
            nChannels,
            windowStart));
      }
  }
}

void GOTestSoundStream::TestFloatHistoryMatchesInt() {
  using Resample = GOSoundResample;
  constexpr unsigned PROLOGUE = GOSoundAudioSection::PROLOGUE_LENGTH;
  const Resample resample;
  const unsigned transitionOffset
    = PROLOGUE + N_FRAMES - GOSoundAudioSection::getMaxReadAhead();
  std::vector<GOInt24> intData(N_FRAMES);
  std::vector<float> floatData(N_FRAMES);

  for (unsigned frameI = 0; frameI < N_FRAMES; frameI++) {
    const int value = (int)(frameI % 100) * 30000 - 1500000;

    intData[frameI] = value;
    floatData[frameI] = value / GOWave::SCALE_24_BITS;
  }

  const auto pInt = CreateAudioSection(
    intData.data(), GOWave::SF_SIGNEDINT24_24, 1, N_FRAMES, false);
  const auto pFloat = CreateAudioSection(
    floatData.data(), GOWave::SF_IEEE_FLOAT, 1, N_FRAMES, false);
  const auto pFloatCompressed = CreateAudioSection(
    floatData.data(), GOWave::SF_IEEE_FLOAT, 1, N_FRAMES, true);

  GOAssert(pFloatCompressed->IsCompressed(), "The attack must use compression");

  auto captureHistory = [&](
                          const GOSoundAudioSection &section,
                          GOSoundResample::InterpolationType mode,
                          unsigned nAttackFrames) {
    GOSoundStream stream;
    GOSoundReleaseAlignTable::History history;
    GO_DECLARE_LOCAL_SOUND_BUFFER(buffer, 2, nAttackFrames)

    stream.InitStream(&resample, &section, mode);
    GOAssert(stream.ReadBlock(FRACTION_INCREMENT, buffer), "Attack played");
    stream.GetHistory(history);
    return history;
  };

  // A window in the main data, a history straddling the segment boundary and
  // a history entirely in the end segment
  for (const auto mode :
       {Resample::GO_LINEAR_INTERPOLATION,
        Resample::GO_POLYPHASE_INTERPOLATION})
    for (unsigned windowStart :
         {PROLOGUE + 83u, transitionOffset - 1, PROLOGUE + N_FRAMES - 3}) {
      const unsigned nAttackFrames
        = windowStart - (PROLOGUE - Resample::getCentrePoint(mode));
      const auto expected = captureHistory(*pInt, mode, nAttackFrames);

      GOAssert(
        expected != GOSoundReleaseAlignTable::History{},
        std::format("The history must not be empty (window {})", windowStart));
      GOAssert(
        captureHistory(*pFloat, mode, nAttackFrames) == expected,
        std::format("Float history must equal int (window {})", windowStart));
      GOAssert(
        captureHistory(*pFloatCompressed, mode, nAttackFrames) == expected,
        std::format(
          "Compressed float history must equal int (window {})", windowStart));
    }
}

void GOTestSoundStream::TestReleaseAlignmentFallsBackToStart() {
  using Resample = GOSoundResample;
  const Resample resample;
  const auto pShortRelease = CreateAudioSection(1, N_FRAMES, false);
  const auto pLongRelease = CreateAudioSection(1, 4096, false);

  for (bool isCompressed : {false, true}) {
    const auto pAttack = CreateAudioSection(1, N_FRAMES, isCompressed);

    GOAssert(
      pAttack->IsCompressed() == isCompressed,
      "The attack must exercise the selected decoder");
    pShortRelease->SetupStreamAlignment({pAttack.get()}, 0);
    pLongRelease->SetupStreamAlignment({pAttack.get()}, 0);
    GOAssert(
      pShortRelease->SupportsStreamAlignment()
        && pLongRelease->SupportsStreamAlignment(),
      "Both releases must exercise aligned stream initialization");

    auto checkReleaseStart = [&](
                               const GOSoundAudioSection &section,
                               Resample::InterpolationType mode,
                               unsigned nAttackFrames) {
      GOSoundStream attack;
      GOSoundStream release;
      GOSoundStream reference;

      GO_DECLARE_LOCAL_SOUND_BUFFER(attackBuffer, 2, nAttackFrames)
      GO_DECLARE_LOCAL_SOUND_BUFFER(releaseBuffer, 2, N_FRAMES_PER_BLOCK)
      GO_DECLARE_LOCAL_SOUND_BUFFER(expected, 2, N_FRAMES_PER_BLOCK)

      attack.InitStream(&resample, pAttack.get(), mode);
      GOAssert(
        attack.ReadBlock(FRACTION_INCREMENT, attackBuffer), "Attack played");
      release.InitAlignedStream(&section, mode, &attack);
      reference.InitStream(&resample, &section, mode);
      GOAssert(
        release.ReadBlock(FRACTION_INCREMENT, releaseBuffer)
          && reference.ReadBlock(FRACTION_INCREMENT, expected),
        "Release must play");
      GOAssert(
        std::equal(
          releaseBuffer.GetData(),
          releaseBuffer.GetData() + releaseBuffer.GetNItems(),
          expected.GetData()),
        "Release must start at the first PCM frame when alignment is "
        "unavailable");
    };

    // The short release has an empty table. The long one has a valid table,
    // but the attack's matching frames still lie in the zero prologue.
    checkReleaseStart(*pShortRelease, Resample::GO_LINEAR_INTERPOLATION, 83);
    checkReleaseStart(*pLongRelease, Resample::GO_BANDLIMITED_INTERPOLATION, 1);
  }
}

void GOTestSoundStream::TestRejectedLoopKeepsStartSegmentIndex() {
  const std::vector<GOWaveLoop> loops = {{64, 150}, {151, 161}, {200, 399}};
  const std::vector<GOInt24> pcm(N_FRAMES);
  GOSoundAudioSection section(m_pool);

  section.Setup(
    nullptr,
    nullptr,
    pcm.data(),
    GOWave::SF_SIGNEDINT24_24,
    1,
    SECTION_RATE,
    N_FRAMES,
    &loops,
    BOOL3_DEFAULT,
    false,
    1,
    0);

  // The second loop is shorter than the 48-frame crossfade and is rejected.
  GOAssert(
    section.GetEndSegment(1).next_start_segment_index == 2
      && section.GetStartSegment(2).start_offset
        == GOSoundAudioSection::PROLOGUE_LENGTH + loops[2].m_StartPosition,
    "The last loop must point to its accepted start segment");
}

void GOTestSoundStream::TestLoopTransitionAcrossDifferentEndPos() {
  // Two loops with different end_pos: a long loop and a short loop placed
  // so PickEndSegment can pick either for some start segments. With the
  // regression in ReadBlock (using the new segment's end_pos when computing
  // the post-wrap position), wrap-around between segments with different
  // end_pos breaks the source position and eventually triggers the
  // "invalid loop" path, after which ReadBlock returns false.
  constexpr unsigned N_TOTAL_FRAMES = 1100;
  constexpr unsigned N_ITERATIONS = 500;
  // Two long loops with very different end_pos. start_offsets are chosen
  // such that an unsigned underflow in the buggy expression
  //   startOffset + (currSrcOffset - end_pos_new)
  // does NOT happen to wrap back into a valid (< end_pos_new) range — it
  // stays huge, so the broken code reliably enters the "Breaking invalid
  // loop" branch and ReadBlock subsequently returns false.
  const std::vector<GOWaveLoop> loops = {
    {500, 999}, // long loop, start_offset=500, end_pos=1000
    {200, 599}, // long loop, start_offset=200, end_pos=600
  };
  const unsigned nChannels = 1;
  const unsigned nSamples = nChannels * N_TOTAL_FRAMES;
  std::vector<GOInt24> pcmData(nSamples);

  for (unsigned i = 0; i < nSamples; i++)
    pcmData[i] = (i % 100) * 30000 - 1500000;

  auto pSection = std::make_unique<GOSoundAudioSection>(m_pool);

  pSection->Setup(
    nullptr,
    nullptr,
    pcmData.data(),
    GOWave::SF_SIGNEDINT24_24,
    nChannels,
    SECTION_RATE,
    N_TOTAL_FRAMES,
    &loops,
    BOOL3_DEFAULT,
    false,
    0,
    0);

  GOSoundResample resample;
  GOSoundStream stream;

  // PickEndSegment uses rand(); seed for reproducibility of the test path.
  std::srand(0);
  stream.InitStream(
    &resample, pSection.get(), GOSoundResample::GO_LINEAR_INTERPOLATION);

  GO_DECLARE_LOCAL_SOUND_BUFFER(buffer, 2, N_FRAMES_PER_BLOCK)

  for (unsigned iterI = 0; iterI < N_ITERATIONS; iterI++) {
    const bool result = stream.ReadBlock(FRACTION_INCREMENT, buffer);

    GOAssert(
      result,
      std::format(
        "ReadBlock should always return true across loop-segment "
        "transitions with different end_pos (iteration {})",
        iterI));
  }
}

void GOTestSoundStream::TestCompressedLoopWrapMatchesUncompressed() {
  constexpr unsigned N_TOTAL_FRAMES = 1100;
  constexpr unsigned N_ITERATIONS = 500;
  const std::vector<GOWaveLoop> loops = {
    {500, 999}, // long loop, start_offset=500, end_pos=1000
    {200, 599}, // long loop, start_offset=200, end_pos=600
  };
  const unsigned nChannels = 1;
  const unsigned nSamples = nChannels * N_TOTAL_FRAMES;
  std::vector<GOInt24> pcmData(nSamples);

  for (unsigned i = 0; i < nSamples; i++)
    pcmData[i] = (i % 100) * 30000 - 1500000;

  auto runCapture = [&](bool isCompressed) {
    auto pSection = std::make_unique<GOSoundAudioSection>(m_pool);

    pSection->Setup(
      nullptr,
      nullptr,
      pcmData.data(),
      GOWave::SF_SIGNEDINT24_24,
      nChannels,
      SECTION_RATE,
      N_TOTAL_FRAMES,
      &loops,
      BOOL3_DEFAULT,
      isCompressed,
      0,
      0);

    GOSoundResample resample;
    GOSoundStream stream;

    // Same seed for both runs: PickEndSegment() uses rand() to choose among
    // multiple valid end segments, and both runs must pick identically so
    // the wrap timing (and hence the skip-ahead pattern) matches.
    std::srand(0);
    stream.InitStream(
      &resample, pSection.get(), GOSoundResample::GO_LINEAR_INTERPOLATION);

    // GOSoundStream::ReadBlock always writes interleaved stereo output,
    // regardless of the source section's channel count.
    const unsigned nBufferItems
      = GOSoundBuffer::getNItems(2, N_FRAMES_PER_BLOCK);
    std::vector<float> captured(N_ITERATIONS * nBufferItems);

    GO_DECLARE_LOCAL_SOUND_BUFFER(buffer, 2, N_FRAMES_PER_BLOCK)

    for (unsigned iterI = 0; iterI < N_ITERATIONS; iterI++) {
      stream.ReadBlock(FRACTION_INCREMENT, buffer);
      for (unsigned i = 0; i < nBufferItems; i++)
        captured[iterI * nBufferItems + i] = buffer.GetData()[i];
    }
    return captured;
  };

  const std::vector<float> uncompressed = runCapture(false);
  const std::vector<float> compressed = runCapture(true);
  const unsigned nBufferItems = GOSoundBuffer::getNItems(2, N_FRAMES_PER_BLOCK);

  for (unsigned i = 0; i < uncompressed.size(); i++)
    GOAssert(
      compressed[i] == uncompressed[i],
      std::format(
        "compressed/uncompressed mismatch at output sample {} (iteration {}, "
        "offset {}): compressed={} uncompressed={}",
        i,
        i / nBufferItems,
        i % nBufferItems,
        compressed[i],
        uncompressed[i]));
}

void GOTestSoundStream::TestBandlimitedLoopAndRetrigger() {
  constexpr auto MODE = GOSoundResample::GO_BANDLIMITED_INTERPOLATION;
  constexpr unsigned INCREMENT
    = GOSoundResample::rateToFractionIncrement(1.0293f);
  const GOWaveLoop loop = {190, 253};
  const GOSoundResample resample;

  auto capture = [&](
                   bool isCompressed,
                   unsigned nNoteFrames,
                   unsigned nFramesPerRead) {
    const auto pAttack
      = CreateAudioSection(1, N_FRAMES, isCompressed, &loop, false, 1);
    const auto pRelease = CreateAudioSection(1, 4096, isCompressed);
    std::vector<float> audio(2 * (nNoteFrames + 2 * N_FRAMES_PER_BLOCK));
    GOSoundBufferMutable attackAudio(audio.data(), 2, nNoteFrames);
    GOSoundBufferMutable tailAudio(
      audio.data() + 2 * nNoteFrames, 2, N_FRAMES_PER_BLOCK);
    GOSoundBufferMutable repeatAudio(
      tailAudio.GetData() + tailAudio.GetNItems(), 2, N_FRAMES_PER_BLOCK);
    GOSoundStream attack;
    GOSoundStream release;
    GOSoundStream repeat;

    GOAssert(
      pAttack->IsCompressed() == isCompressed
        && pRelease->IsCompressed() == isCompressed,
      "The loop fixture must exercise the requested decoding path");
    pRelease->SetupStreamAlignment({pAttack.get()}, 0);
    GOAssert(
      pRelease->SupportsStreamAlignment(), "The release must be alignable");
    attack.InitStream(&resample, pAttack.get(), MODE);

    for (unsigned frameI = 0; frameI < nNoteFrames; frameI += nFramesPerRead) {
      auto buffer = attackAudio.GetSubBuffer(
        frameI, std::min(nFramesPerRead, nNoteFrames - frameI));

      GOAssert(attack.ReadBlock(INCREMENT, buffer), "Short loop must continue");
    }
    release.InitAlignedStream(pRelease.get(), MODE, &attack);
    repeat.InitStream(&resample, pAttack.get(), MODE);
    GOAssert(release.ReadBlock(INCREMENT, tailAudio), "Release must continue");
    GOAssert(
      repeat.ReadBlock(INCREMENT, repeatAudio), "Repeated attack must play");
    GOAssert(
      std::equal(
        repeatAudio.GetData(),
        repeatAudio.GetData() + repeatAudio.GetNItems(),
        audio.data()),
      "The repeated note must start at the beginning while the tail plays");
    return audio;
  };

  // Align once in the decoded attack and once after wrapping the short loop.
  for (unsigned nNoteFrames : {83u, 263u}) {
    const auto raw = capture(false, nNoteFrames, nNoteFrames);
    const auto compressed = capture(true, nNoteFrames, 17);

    // Release builds may reassociate the sum differently for packed and
    // decoded samples. Allow two PCM units on this 24-bit fixture.
    const auto mismatch = std::mismatch(
      raw.begin(), raw.end(), compressed.begin(), [](float a, float b) {
        return std::abs(a - b) <= 2.0f;
      });

    if (mismatch.first != raw.end())
      GOAssert(
        false,
        std::format(
          "Loop/release differs at item {} of the {}-frame note: raw={}, "
          "compressed={}",
          mismatch.first - raw.begin(),
          nNoteFrames,
          *mismatch.first,
          *mismatch.second));
  }
}

void GOTestSoundStream::TestRetuningSuppressesImages() {
  using Resample = GOSoundResample;
  constexpr unsigned N_SOURCE_FRAMES = 4096;
  constexpr unsigned N_OUTPUT_FRAMES = 8192;
  constexpr unsigned N_BLOCK_FRAMES = 256;
  constexpr double TWO_PI = 2.0 * std::numbers::pi;
  constexpr float INPUT_AMPLITUDE = 1 << 20;
  const unsigned increment
    = Resample::rateToFractionIncrement(std::exp2(50.0f / 1200));
  const double outputFrequency
    = double(increment) / (16 * Resample::UPSAMPLE_FACTOR);
  const double imageFrequency = 1.0 - 15.0 * outputFrequency;
  const std::vector<GOWaveLoop> loops = {{0, N_SOURCE_FRAMES - 1}};
  std::vector<GOInt24> pcm(N_SOURCE_FRAMES);
  GOSoundAudioSection section(m_pool);
  const Resample resample;
  std::vector<double> imageLevels;

  for (unsigned frameI = 0; frameI < N_SOURCE_FRAMES; ++frameI)
    pcm[frameI] = int(INPUT_AMPLITUDE * std::sin(TWO_PI * (frameI % 16) / 16));
  section.Setup(
    nullptr,
    nullptr,
    pcm.data(),
    GOWave::SF_SIGNEDINT24_24,
    1,
    SECTION_RATE,
    N_SOURCE_FRAMES,
    &loops,
    BOOL3_DEFAULT,
    false,
    0,
    0);

  for (const auto mode :
       {Resample::GO_LINEAR_INTERPOLATION,
        Resample::GO_POLYPHASE_INTERPOLATION,
        Resample::GO_BANDLIMITED_INTERPOLATION}) {
    GOSoundStream stream;
    double wantedReal = 0;
    double wantedImaginary = 0;
    double imageReal = 0;
    double imageImaginary = 0;

    GO_DECLARE_LOCAL_SOUND_BUFFER(buffer, 2, N_BLOCK_FRAMES)

    stream.InitStream(&resample, &section, mode);
    // Measure a steady section of the loop.
    stream.ReadBlock(increment, buffer);
    for (unsigned offset = 0; offset < N_OUTPUT_FRAMES;
         offset += N_BLOCK_FRAMES) {
      GOAssert(
        stream.ReadBlock(increment, buffer), "The sine loop must continue");

      for (unsigned frameI = 0; frameI < N_BLOCK_FRAMES; ++frameI) {
        const unsigned outputI = offset + frameI;
        const double sample = buffer.GetData()[2 * frameI];
        const double wantedPhase = TWO_PI * outputFrequency * outputI;
        const double imagePhase = TWO_PI * imageFrequency * outputI;

        wantedReal += sample * std::cos(wantedPhase);
        wantedImaginary += sample * std::sin(wantedPhase);
        imageReal += sample * std::cos(imagePhase);
        imageImaginary += sample * std::sin(imagePhase);
        GOAssert(
          buffer.GetData()[2 * frameI] == buffer.GetData()[2 * frameI + 1],
          "Mono playback must produce identical output channels");
      }
    }

    const double wanted = std::hypot(wantedReal, wantedImaginary);
    const double image = std::hypot(imageReal, imageImaginary);

    GOAssert(
      2 * wanted / N_OUTPUT_FRAMES > INPUT_AMPLITUDE * 0.98,
      "The retuned 3 kHz fundamental must retain its level");
    imageLevels.push_back(20 * std::log10(image / wanted));
  }

  GOAssert(
    imageLevels[0] > -49 && imageLevels[0] < -45,
    "Linear playback must reproduce the measured 1.682 kHz image");
  GOAssert(
    imageLevels[1] > -55 && imageLevels[1] < -51,
    "Polyphase playback must reproduce the measured 1.682 kHz image");
  GOAssert(
    imageLevels[2] < -100 && imageLevels[2] < imageLevels[1] - 40,
    "High-quality sinc must suppress the image below -100 dBc");
}

void GOTestSoundStream::TestNativeRateStartsAtFirstFrame() {
  using Resample = GOSoundResample;
  constexpr unsigned N_OUTPUT_FRAMES = 777;
  // The first loop end segment overlaps the prologue, including when the
  // decoder can read the padded main data of a compressed section.
  const GOWaveLoop loop = {35, 52};
  const Resample resample;

  for (unsigned nChannels : {1u, 2u})
    for (bool isCompressed : {false, true}) {
      const auto pSection
        = CreateAudioSection(nChannels, N_FRAMES, isCompressed, &loop);
      std::vector<float> expected(2 * N_OUTPUT_FRAMES);
      std::vector<float> actual(2 * N_OUTPUT_FRAMES);
      GOSoundBufferMutable reference(expected.data(), 2, N_OUTPUT_FRAMES);
      GOSoundBufferMutable output(actual.data(), 2, N_OUTPUT_FRAMES);
      GOSoundStream linear;

      GOAssert(
        pSection->IsCompressed() == isCompressed,
        "The short loop must exercise the selected decoder");
      linear.InitStream(
        &resample, pSection.get(), Resample::GO_LINEAR_INTERPOLATION);
      GOAssert(
        linear.ReadBlock(FRACTION_INCREMENT, reference),
        "The reference loop must continue");
      GOAssert(
        expected[0] == float(-1500000 / int(nChannels)),
        "Linear playback must start at the first frame of the sample");
      for (const auto mode :
           {Resample::GO_LINEAR_INTERPOLATION,
            Resample::GO_POLYPHASE_INTERPOLATION,
            Resample::GO_BANDLIMITED_INTERPOLATION}) {
        GOSoundStream stream;

        stream.InitStream(&resample, pSection.get(), mode);
        for (unsigned offset = 0; offset < N_OUTPUT_FRAMES; offset += 17) {
          auto block = output.GetSubBuffer(
            offset, std::min(17u, N_OUTPUT_FRAMES - offset));

          GOAssert(
            stream.ReadBlock(FRACTION_INCREMENT, block),
            "Native-rate playback must continue");
        }
        GOAssert(
          actual == expected,
          "Native-rate playback must start at the first frame in every mode");
      }
    }
}

void GOTestSoundStream::TestFloatRead() {
  std::vector<float> pcmData(N_FRAMES);

  for (unsigned frameI = 0; frameI < N_FRAMES; frameI++)
    pcmData[frameI] = 0.001f * (int)(frameI % 100) - 0.05f;

  const auto pSection = CreateAudioSection(
    pcmData.data(), GOWave::SF_IEEE_FLOAT, 1, N_FRAMES, false);
  GOSoundResample resample;
  GOSoundStream stream;

  GOAssert(!pSection->IsCompressed(), "the section must stay float");
  GOAssert(pSection->GetNormGain() == 1.0f, "an uncompressed float has gain 1");
  stream.InitStream(
    &resample, pSection.get(), GOSoundResample::GO_LINEAR_INTERPOLATION);

  GO_DECLARE_LOCAL_SOUND_BUFFER(buffer, 2, N_FRAMES_PER_BLOCK)

  GOAssert(stream.ReadBlock(FRACTION_INCREMENT, buffer), "must have data");
  for (unsigned frameI = 0; frameI < N_FRAMES_PER_BLOCK; frameI++)
    GOAssert(
      buffer.GetData()[frameI * 2] == pcmData[frameI],
      "float samples must be read as they are");
}

void GOTestSoundStream::TestFloatPolyphaseLoopMatchesInt() {
  constexpr float SCALE = GOWave::SCALE_24_BITS;
  const GOWaveLoop loop = {100, N_FRAMES - 1};
  std::vector<float> floatData(N_FRAMES);
  std::vector<GOInt24> intData(N_FRAMES);

  for (unsigned frameI = 0; frameI < N_FRAMES; frameI++) {
    floatData[frameI] = 0.001f * (int)(frameI % 100) - 0.05f;
    intData[frameI] = (int)lroundf(floatData[frameI] * SCALE);
  }

  const auto pFloat = CreateAudioSection(
    floatData.data(), GOWave::SF_IEEE_FLOAT, 1, N_FRAMES, false, &loop);
  const auto pInt = CreateAudioSection(
    intData.data(), GOWave::SF_SIGNEDINT24_24, 1, N_FRAMES, false, &loop);
  GOSoundResample resample;
  GOSoundStream floatStream;
  GOSoundStream intStream;

  floatStream.InitStream(
    &resample, pFloat.get(), GOSoundResample::GO_POLYPHASE_INTERPOLATION);
  intStream.InitStream(
    &resample, pInt.get(), GOSoundResample::GO_POLYPHASE_INTERPOLATION);

  GO_DECLARE_LOCAL_SOUND_BUFFER(floatBuffer, 2, N_FRAMES_PER_BLOCK)
  GO_DECLARE_LOCAL_SOUND_BUFFER(intBuffer, 2, N_FRAMES_PER_BLOCK)

  // Read past the loop end more than once
  for (unsigned blockI = 0; blockI < 25; blockI++) {
    GOAssert(
      floatStream.ReadBlock(FRACTION_INCREMENT, floatBuffer)
        && intStream.ReadBlock(FRACTION_INCREMENT, intBuffer),
      "a looped stream must not end");
    for (unsigned n = floatBuffer.GetNItems(), itemI = 0; itemI < n; itemI++)
      GOAssert(
        std::abs(
          floatBuffer.GetData()[itemI] - intBuffer.GetData()[itemI] / SCALE)
          < 1e-6f,
        "a float loop must match the same data as integers");
  }
}

void GOTestSoundStream::TestCompressedFloatMatchesInt() {
  constexpr float SCALE = GOWave::SCALE_24_BITS;
  const GOWaveLoop loop = {100, N_FRAMES - 1};
  std::vector<float> floatData(N_FRAMES);
  std::vector<GOInt24> intData(N_FRAMES);

  for (unsigned frameI = 0; frameI < N_FRAMES; frameI++) {
    floatData[frameI] = 0.5f * sinf(frameI * 0.05f);
    intData[frameI] = (int)lroundf(floatData[frameI] * SCALE);
  }

  const auto pFloat = CreateAudioSection(
    floatData.data(), GOWave::SF_IEEE_FLOAT, 1, N_FRAMES, true, &loop);
  const auto pInt = CreateAudioSection(
    intData.data(), GOWave::SF_SIGNEDINT24_24, 1, N_FRAMES, true, &loop);

  GOAssert(pFloat->IsCompressed(), "a smooth float loop must compress");
  GOAssert(pFloat->GetNormGain() == 1.0f / SCALE, "gain of int32 raw units");

  GOSoundResample resample;
  GOSoundStream floatStream;
  GOSoundStream intStream;

  floatStream.InitStream(
    &resample, pFloat.get(), GOSoundResample::GO_LINEAR_INTERPOLATION);
  intStream.InitStream(
    &resample, pInt.get(), GOSoundResample::GO_LINEAR_INTERPOLATION);

  GO_DECLARE_LOCAL_SOUND_BUFFER(floatBuffer, 2, N_FRAMES_PER_BLOCK)
  GO_DECLARE_LOCAL_SOUND_BUFFER(intBuffer, 2, N_FRAMES_PER_BLOCK)

  // Read past the loop end, through the end segment
  for (unsigned blockI = 0; blockI < 25; blockI++) {
    GOAssert(
      floatStream.ReadBlock(FRACTION_INCREMENT, floatBuffer)
        && intStream.ReadBlock(FRACTION_INCREMENT, intBuffer),
      "a looped stream must not end");
    GOAssert(
      memcmp(
        floatBuffer.GetData(),
        intBuffer.GetData(),
        floatBuffer.GetNItems() * sizeof(float))
        == 0,
      "a compressed float loop must equal the same data as integers");
  }
}

void GOTestSoundStream::TestFloatLoopCrossfadeKeepsFloats() {
  constexpr unsigned LOOP_START = 100;
  constexpr unsigned CROSSFADE_MS = 1;
  constexpr unsigned N_FADE_FRAMES = CROSSFADE_MS * SECTION_RATE / 1000;
  const GOWaveLoop loop = {LOOP_START, N_FRAMES - 1};
  // {the value before the loop start, the value in the loop}: the first pair
  // is beyond Q23 full scale, the second one is below the Q23 resolution
  const std::pair<float, float> cases[] = {{-1.5f, 1.5f}, {1e-9f, 3e-9f}};

  for (const auto &[beforeLoop, inLoop] : cases) {
    std::vector<float> data(N_FRAMES);

    for (unsigned frameI = 0; frameI < N_FRAMES; frameI++)
      data[frameI] = frameI < LOOP_START ? beforeLoop : inLoop;

    const auto pSection = CreateAudioSection(
      data.data(),
      GOWave::SF_IEEE_FLOAT,
      1,
      N_FRAMES,
      false,
      &loop,
      CROSSFADE_MS);
    const GOSoundAudioSection::EndSegment &endSegment
      = pSection->GetEndSegment(0);
    // the crossfaded frames end at the loop end
    const unsigned fadeStartI
      = endSegment.end_pos - endSegment.transition_offset - N_FADE_FRAMES;
    const float tolerance
      = 1e-6f * std::max(std::abs(beforeLoop), std::abs(inLoop));

    for (unsigned frameI = 0; frameI < N_FADE_FRAMES; frameI++) {
      const float factor
        = (cos(M_PI * (frameI + 0.5) / N_FADE_FRAMES) + 1.0) * 0.5;
      const float expected = inLoop * factor + beforeLoop * (1 - factor);
      float actual;

      memcpy(
        &actual,
        endSegment.end_data + (fadeStartI + frameI) * sizeof(float),
        sizeof(float));
      GOAssert(
        std::abs(actual - expected) <= tolerance,
        "a float loop crossfade must not be clamped or rounded to Q23");
    }
  }
}

void GOTestSoundStream::TestFloatCacheRoundTrip() {
  const GOWaveLoop loop = {100, N_FRAMES - 1};
  std::vector<float> floatData(N_FRAMES);
  const GOSoundResample resample;

  for (unsigned frameI = 0; frameI < N_FRAMES; frameI++)
    floatData[frameI] = 0.5f * sinf(frameI * 0.05f);

  // Reads past the loop end, through the end segment
  auto readSection = [&](const GOSoundAudioSection &section) {
    GOSoundStream stream;
    std::vector<float> out;
    GO_DECLARE_LOCAL_SOUND_BUFFER(buffer, 2, N_FRAMES_PER_BLOCK)

    stream.InitStream(
      &resample, &section, GOSoundResample::GO_POLYPHASE_INTERPOLATION);
    for (unsigned blockI = 0; blockI < 25; blockI++) {
      GOAssert(stream.ReadBlock(FRACTION_INCREMENT, buffer), "must have data");
      out.insert(
        out.end(), buffer.GetData(), buffer.GetData() + buffer.GetNItems());
    }
    return out;
  };

  for (bool isCompressed : {false, true}) {
    const auto pSaved = CreateAudioSection(
      floatData.data(),
      GOWave::SF_IEEE_FLOAT,
      1,
      N_FRAMES,
      isCompressed,
      &loop);
    const wxString path = wxFileName::CreateTempFileName(wxT("goCacheTest"));

    {
      wxFileOutputStream stream(path);
      GOCacheWriter writer(stream, false);

      GOAssert(
        writer.WriteHeader() && pSaved->SaveCache(writer),
        "the section must be saved");
      writer.Close();
    }

    wxFile file(path);
    // The GOCache constructor maps the file into the pool and frees the pool's
    // previous content, so the cache is loaded into a pool of its own, as an
    // organ does. The loaded section must be destroyed before its pool
    GOMemoryPool cachePool;
    GOCache cache(file, cachePool);
    GOSoundAudioSection loaded(cachePool);

    GOAssert(
      cache.ReadHeader() && loaded.LoadCache(cache),
      "the section must be loaded");
    cache.Close();
    GOAssert(
      loaded.IsCompressed() == pSaved->IsCompressed()
        && loaded.GetNormGain() == pSaved->GetNormGain(),
      std::format(
        "The format must survive the cache (compressed {})", isCompressed));
    GOAssert(
      readSection(loaded) == readSection(*pSaved),
      std::format(
        "The loaded section must read as saved (compressed {})", isCompressed));
    file.Close();
    wxRemoveFile(path);
  }
}

void GOTestSoundStream::TestProviderLoadsFloatWave() {
  const GOWaveLoop loop = {100, N_FRAMES - 1};
  std::vector<float> samples(2 * N_FRAMES);

  for (unsigned frameI = 0; frameI < N_FRAMES; frameI++) {
    samples[frameI * 2] = 0.001f * (int)(frameI % 100) - 0.05f;
    samples[frameI * 2 + 1] = -samples[frameI * 2];
  }

  const wxString path = wxFileName::CreateTempFileName(wxT("goProviderTest"));

  GOTestWaveFileBuilder::writeWavFile(
    path, true, 32, 2, samples.data(), N_FRAMES, &loop);

  GOConfig config(GetName(), "");
  GOFileStore fileStore(config);
  const GOSoundResample resample;

  for (const unsigned bits : {GOSoundProviderWave::FILE_OWN_DEPTH, 24u}) {
    const bool isOwn = bits == GOSoundProviderWave::FILE_OWN_DEPTH;
    GOSoundProviderWave provider;
    GOSoundProviderWave::AttackFileInfo attack;

    attack.filename.AssignAbsolute(path);
    attack.min_attack_velocity = 0;
    attack.max_released_time = 0;
    attack.max_playback_time = 0;
    attack.attack_start = 0;
    attack.cue_point = -1;
    attack.release_end = -1;
    attack.m_LoopCrossfadeLength = 0;
    attack.m_ReleaseCrossfadeLength = 0;
    attack.m_WaveTremulantStateFor = BOOL3_DEFAULT;
    attack.load_release = false;
    attack.percussive = false;
    provider.LoadFromMultipleFiles(
      fileStore,
      m_pool,
      {attack},
      {},
      bits,
      2,
      false,
      GOSoundProviderWave::LOOP_LOAD_LONGEST,
      true,
      false);

    const GOSoundAudioSection *pSection = provider.GetAttack(0, 0);

    GOAssert(pSection, "the attack must be loaded");
    GOAssert(
      pSection->GetChannels() == 2
        && pSection->GetBitsPerSample() == (isOwn ? 32u : 24u),
      "the file must be loaded with the requested depth");

    GOSoundStream stream;

    GO_DECLARE_LOCAL_SOUND_BUFFER(buffer, 2, N_FRAMES_PER_BLOCK)

    stream.InitStream(
      &resample, pSection, GOSoundResample::GO_LINEAR_INTERPOLATION);
    GOAssert(stream.ReadBlock(FRACTION_INCREMENT, buffer), "must have data");
    for (unsigned itemI = 0; itemI < 2 * N_FRAMES_PER_BLOCK; itemI++) {
      // Float is read as it is, 24-bit items are in raw units of 2^-23 FS
      const float expected
        = isOwn ? samples[itemI] : samples[itemI] * GOWave::SCALE_24_BITS;

      GOAssert(
        std::abs(buffer.GetData()[itemI] - expected) <= (isOwn ? 0.0f : 1.0f),
        std::format(
          "The loaded sample {} must match the file (bits {})", itemI, bits));
    }
  }
  wxRemoveFile(path);
}

void GOTestSoundStream::run() {
  for (unsigned nChannels : {1u, 2u}) {
    for (bool isCompressed : {false, true}) {
      // The names are printed by --list and typed back as patterns, so they
      // contain neither regex metacharacters (e.g. '[', ']', '=') nor ','
      // (it separates patterns) nor '(' (GOTestScope cuts the test name at it)
      const char *const compression = isCompressed ? "Compressed" : "Raw";
      const std::string linearName = std::format(
        "TestReadBlockLinear{}Channels{}", compression, nChannels);
      const std::string polyphaseName = std::format(
        "TestReadBlockPolyphase{}Channels{}", compression, nChannels);
      const std::string bandlimitedName = std::format(
        "TestReadBlockBandlimited{}Channels{}", compression, nChannels);

      if (GOTestScope::shouldRun(linearName)) {
        GOTestScope scope(GOTestScope::TEST, linearName);

        TestReadBlock(
          GOSoundResample::GO_LINEAR_INTERPOLATION, isCompressed, nChannels);
      }
      if (GOTestScope::shouldRun(polyphaseName)) {
        GOTestScope scope(GOTestScope::TEST, polyphaseName);

        TestReadBlock(
          GOSoundResample::GO_POLYPHASE_INTERPOLATION, isCompressed, nChannels);
      }
      if (GOTestScope::shouldRun(bandlimitedName)) {
        GOTestScope scope(GOTestScope::TEST, bandlimitedName);

        TestReadBlock(
          GOSoundResample::GO_BANDLIMITED_INTERPOLATION,
          isCompressed,
          nChannels);
      }
    }
  }
  GO_RUN_TEST(TestLoopedStreamAlwaysReturnsTrue())
  GO_RUN_TEST(TestLoopTransitionAcrossDifferentEndPos())
  GO_RUN_TEST(TestInitAlignedStream())
  GO_RUN_TEST(TestCompressedLoopWrapMatchesUncompressed())
  GO_RUN_TEST(TestInitAlignedStreamWithSilentAttack())
  GO_RUN_TEST(TestReleaseAlignmentMatchesUncompressed())
  GO_RUN_TEST(TestFloatHistoryMatchesInt())
  GO_RUN_TEST(TestFloatCacheRoundTrip())
  GO_RUN_TEST(TestProviderLoadsFloatWave())
  GO_RUN_TEST(TestReleaseAlignmentFallsBackToStart())
  GO_RUN_TEST(TestRejectedLoopKeepsStartSegmentIndex())
  GO_RUN_TEST(TestFloatRead())
  GO_RUN_TEST(TestFloatPolyphaseLoopMatchesInt())
  GO_RUN_TEST(TestCompressedFloatMatchesInt())
  GO_RUN_TEST(TestFloatLoopCrossfadeKeepsFloats())
  GO_RUN_TEST(TestBandlimitedLoopAndRetrigger())
  GO_RUN_TEST(TestRetuningSuppressesImages())
  GO_RUN_TEST(TestNativeRateStartsAtFirstFrame())
}
