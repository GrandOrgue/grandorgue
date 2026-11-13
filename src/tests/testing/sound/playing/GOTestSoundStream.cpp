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

#include "sound/buffer/GOSoundBufferMutable.h"
#include "sound/playing/GOSoundAudioSection.h"
#include "sound/playing/GOSoundStream.h"

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
  std::vector<GOWaveLoop> loops;

  if (isSilent)
    for (unsigned i = 0; i < nSamples; i++)
      pcmData[i] = (i % 2) ? 1 : -1;
  else
    for (unsigned i = 0; i < nSamples; i++)
      // Keep the per-channel slope compressible for stereo fixtures too.
      pcmData[i] = (int(i % 100) * 30000 - 1500000) / int(nChannels);

  if (pLoop)
    loops.push_back(*pLoop);

  auto pSection = std::make_unique<GOSoundAudioSection>(m_pool);

  pSection->Setup(
    nullptr,
    nullptr,
    pcmData.data(),
    GOWave::SF_SIGNEDINT24_24,
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
  const unsigned nOutputFrames = isBandlimited ? N_FRAMES * 4 / 5 : N_FRAMES;
  const auto pSection = CreateAudioSection(nChannels, N_FRAMES, isCompressed);
  GOSoundResample resample;
  GOSoundStream stream;

  stream.InitStream(&resample, pSection.get(), interpolationType);

  GO_DECLARE_LOCAL_SOUND_BUFFER(buffer, 2, N_FRAMES_PER_BLOCK)

  for (unsigned frameI = 0; frameI < nOutputFrames;
       frameI += N_FRAMES_PER_BLOCK) {
    const bool result = stream.ReadBlock(increment, buffer);

    GOAssert(
      result,
      std::format(
        "ReadBlock should return true while frames available ({})", label));
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
  const GOSoundResample resample;
  const unsigned transitionOffset
    = N_FRAMES - GOSoundAudioSection::getMaxReadAhead();

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
         {GOSoundResample::GO_LINEAR_INTERPOLATION,
          GOSoundResample::GO_POLYPHASE_INTERPOLATION})
      for (unsigned nAttackFrames : {83u, transitionOffset - 1, N_FRAMES - 3})
        GOAssert(
          captureRelease(*pRaw, mode, nAttackFrames)
            == captureRelease(*pCompressed, mode, nAttackFrames),
          std::format(
            "Release must match across decoders (mode {}, {} channels, frame "
            "{})",
            unsigned(mode),
            nChannels,
            nAttackFrames));
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
      && section.GetStartSegment(2).start_offset == loops[2].m_StartPosition,
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

void GOTestSoundStream::TestNativeRateCopiesWindowCenter() {
  using Resample = GOSoundResample;
  constexpr unsigned N_OUTPUT_FRAMES = 777;
  constexpr unsigned N_REFERENCE_FRAMES
    = N_OUTPUT_FRAMES + Resample::BANDLIMITED_LEFT;
  const GOWaveLoop loop = {190, 207};
  const Resample resample;

  for (unsigned nChannels : {1u, 2u})
    for (bool isCompressed : {false, true}) {
      const auto pSection
        = CreateAudioSection(nChannels, N_FRAMES, isCompressed, &loop);
      std::vector<float> expected(2 * N_REFERENCE_FRAMES);
      std::vector<float> actual(2 * N_OUTPUT_FRAMES);
      GOSoundBufferMutable reference(expected.data(), 2, N_REFERENCE_FRAMES);
      GOSoundBufferMutable output(actual.data(), 2, N_OUTPUT_FRAMES);
      GOSoundStream linear;

      linear.InitStream(
        &resample, pSection.get(), Resample::GO_LINEAR_INTERPOLATION);
      GOAssert(
        linear.ReadBlock(FRACTION_INCREMENT, reference),
        "The reference loop must continue");
      for (const auto mode :
           {Resample::GO_LINEAR_INTERPOLATION,
            Resample::GO_POLYPHASE_INTERPOLATION,
            Resample::GO_BANDLIMITED_INTERPOLATION}) {
        const unsigned center = Resample::getVectorLength(mode) / 2 - 1;
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
          std::equal(
            actual.begin(), actual.end(), expected.begin() + 2 * center),
          "Native-rate playback must copy the centre source frame exactly");
      }
    }
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
  GO_RUN_TEST(TestRejectedLoopKeepsStartSegmentIndex())
  GO_RUN_TEST(TestBandlimitedLoopAndRetrigger())
  GO_RUN_TEST(TestRetuningSuppressesImages())
  GO_RUN_TEST(TestNativeRateCopiesWindowCenter())
}
