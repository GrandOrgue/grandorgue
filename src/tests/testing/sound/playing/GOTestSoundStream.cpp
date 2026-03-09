/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundStream.h"

#include <format>
#include <vector>

#include "sound/playing/GOSoundAudioSection.h"
#include "sound/playing/GOSoundStream.h"

#include "GOInt.h"
#include "GOWave.h"
#include "GOWaveLoop.h"

const std::string GOTestSoundStream::TEST_NAME = "GOTestSoundStream";

static constexpr unsigned SECTION_RATE = 48000;
static constexpr float SAMPLE_RATE_ADJUSTMENT = 1.0f / SECTION_RATE;
static constexpr unsigned N_FRAMES = 500;
static constexpr unsigned N_ATTACK_FRAMES = 100;
static constexpr unsigned N_FRAMES_PER_BLOCK = 50;
static constexpr unsigned N_BUFFER_ITEMS = N_FRAMES_PER_BLOCK * 2;

std::unique_ptr<GOSoundAudioSection> GOTestSoundStream::CreateAudioSection(
  unsigned nChannels,
  unsigned nFrames,
  bool isCompressed,
  const GOWaveLoop *pLoop) {
  const unsigned nSamples = nChannels * nFrames;
  std::vector<GOInt24> pcmData(nSamples);
  std::vector<GOWaveLoop> loops;

  for (unsigned i = 0; i < nSamples; i++)
    pcmData[i] = (i % 100) * 30000 - 1500000;

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
    0,
    0);
  return pSection;
}

void GOTestSoundStream::TestReadBlock(
  GOSoundResample::InterpolationType interpolationType,
  bool isCompressed,
  unsigned nChannels) {
  const std::string label = std::format(
    "interpolation={} compressed={} channels={}",
    interpolationType == GOSoundResample::GO_LINEAR_INTERPOLATION ? "linear"
                                                                  : "polyphase",
    isCompressed,
    nChannels);
  const auto pSection = CreateAudioSection(nChannels, N_FRAMES, isCompressed);
  GOSoundResample resample;
  GOSoundStream stream;

  stream.InitStream(
    &resample, pSection.get(), interpolationType, SAMPLE_RATE_ADJUSTMENT);

  float buffer[N_BUFFER_ITEMS];

  for (unsigned frameI = 0; frameI < N_FRAMES; frameI += N_FRAMES_PER_BLOCK) {
    const bool result = stream.ReadBlock(buffer, N_FRAMES_PER_BLOCK);

    GOAssert(
      result,
      std::format(
        "ReadBlock should return true while frames available ({})", label));
  }

  // ReadBlock should return false after exhaustion
  const bool resultAfterEnd = stream.ReadBlock(buffer, N_FRAMES_PER_BLOCK);

  GOAssert(
    !resultAfterEnd,
    std::format("ReadBlock should return false after exhaustion ({})", label));

  // Buffer should be filled with zeros after end
  for (unsigned i = 0; i < N_BUFFER_ITEMS; i++)
    GOAssert(
      buffer[i] == 0.0f,
      std::format(
        "Buffer[{}] should be 0.0f after end ({}, got: {})",
        i,
        label,
        buffer[i]));

  // TODO: verify decoded frame values
}

void GOTestSoundStream::TestLoopedStreamAlwaysReturnsTrue() {
  constexpr unsigned N_ITERATIONS = 100;
  const GOWaveLoop loop = {100, 499};
  const auto pSection = CreateAudioSection(1, N_FRAMES, false, &loop);
  GOSoundResample resample;
  GOSoundStream stream;

  stream.InitStream(
    &resample,
    pSection.get(),
    GOSoundResample::GO_LINEAR_INTERPOLATION,
    SAMPLE_RATE_ADJUSTMENT);

  float buffer[N_BUFFER_ITEMS];

  for (unsigned iterI = 0; iterI < N_ITERATIONS; iterI++) {
    const bool result = stream.ReadBlock(buffer, N_FRAMES_PER_BLOCK);

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
    &resample,
    pAttack.get(),
    GOSoundResample::GO_LINEAR_INTERPOLATION,
    SAMPLE_RATE_ADJUSTMENT);

  float buffer[N_BUFFER_ITEMS];

  for (unsigned frameI = 0; frameI < N_ATTACK_FRAMES;
       frameI += N_FRAMES_PER_BLOCK)
    attackStream.ReadBlock(buffer, N_FRAMES_PER_BLOCK);

  GOSoundStream releaseStream;

  releaseStream.InitAlignedStream(
    pRelease.get(), GOSoundResample::GO_LINEAR_INTERPOLATION, &attackStream);

  const bool result = releaseStream.ReadBlock(buffer, N_FRAMES_PER_BLOCK);

  GOAssert(result, "ReadBlock should return true after InitAlignedStream");
}

void GOTestSoundStream::run() {
  for (unsigned nChannels : {1u, 2u}) {
    for (bool isCompressed : {false, true}) {
      TestReadBlock(
        GOSoundResample::GO_LINEAR_INTERPOLATION, isCompressed, nChannels);
      TestReadBlock(
        GOSoundResample::GO_POLYPHASE_INTERPOLATION, isCompressed, nChannels);
    }
  }
  TestLoopedStreamAlwaysReturnsTrue();
  TestInitAlignedStream();
}
