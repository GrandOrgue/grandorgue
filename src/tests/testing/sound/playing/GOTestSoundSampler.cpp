/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundSampler.h"

#include <cmath>

#include "sound/dsp-kernels/GOSoundResample.h"
#include "sound/playing/GOSoundSampler.h"

#include "GOTestScope.h"

const std::string GOTestSoundSampler::TEST_NAME = "GOTestSoundSampler";

static constexpr unsigned UPSAMPLE_FACTOR = GOSoundResample::UPSAMPLE_FACTOR;

void GOTestSoundSampler::TestTuningFactorFollowsProvider() {
  GOSoundSampler sampler;

  sampler.InitResamplerIncrement(2.0f, UPSAMPLE_FACTOR);
  GOAssert(
    sampler.m_InitialResamplerIncrement == UPSAMPLE_FACTOR
      && sampler.m_AppliedResamplerIncrement == UPSAMPLE_FACTOR,
    "the initial increment must be both the initial and the applied one");

  sampler.EnsureTuningFactor(2.0f);
  GOAssert(
    sampler.m_AppliedResamplerIncrement == UPSAMPLE_FACTOR,
    "the initial tuning factor must give the initial increment");
  sampler.EnsureTuningFactor(4.0f);
  GOAssert(
    sampler.m_AppliedResamplerIncrement == 2 * UPSAMPLE_FACTOR,
    "the doubled tuning factor must double the increment");
  sampler.EnsureTuningFactor(1.0f);
  GOAssert(
    sampler.m_AppliedResamplerIncrement == UPSAMPLE_FACTOR / 2,
    "the halved tuning factor must halve the increment");
  sampler.EnsureTuningFactor(1e9f);
  GOAssert(
    sampler.m_AppliedResamplerIncrement
      == GOSoundResample::MAX_FRACTION_INCREMENT,
    "a huge tuning factor must be limited to MAX_FRACTION_INCREMENT");
  sampler.EnsureTuningFactor(1e-9f);
  GOAssert(
    sampler.m_AppliedResamplerIncrement
      == GOSoundResample::MIN_FRACTION_INCREMENT,
    "a tiny tuning factor must be limited to MIN_FRACTION_INCREMENT");
}

void GOTestSoundSampler::TestTuningFactorDoesNotDrift() {
  GOSoundSampler sampler;
  const float centUp = std::pow(2.0f, 100.0f / 1200.0f);

  sampler.InitResamplerIncrement(1.0f, UPSAMPLE_FACTOR);
  for (unsigned i = 0; i < 1000; i++) {
    sampler.EnsureTuningFactor(centUp);
    sampler.EnsureTuningFactor(1.0f);
  }
  GOAssert(
    sampler.m_AppliedResamplerIncrement == UPSAMPLE_FACTOR,
    "1000 round trips must return exactly to the initial increment");
  sampler.EnsureTuningFactor(centUp);
  GOAssert(
    sampler.m_AppliedResamplerIncrement
      == (unsigned)std::lround((double)centUp * UPSAMPLE_FACTOR),
    "the increment must depend on the initial values and the tuning factor "
    "only");
}

void GOTestSoundSampler::TestTuningFactorIsEnsuredOnlyOnChange() {
  GOSoundSampler sampler;
  const unsigned marker = 12345;

  sampler.InitResamplerIncrement(1.0f, UPSAMPLE_FACTOR);
  sampler.EnsureTuningFactor(1.5f);
  sampler.m_AppliedResamplerIncrement = marker;
  sampler.EnsureTuningFactor(1.5f);
  GOAssert(
    sampler.m_AppliedResamplerIncrement == marker,
    "the same tuning factor must not recompute the increment");
  sampler.EnsureTuningFactor(2.0f);
  GOAssert(
    sampler.m_AppliedResamplerIncrement == 2 * UPSAMPLE_FACTOR,
    "a changed tuning factor must recompute the increment");
}

void GOTestSoundSampler::TestInitResamplerIncrementRebasesInitial() {
  GOSoundSampler sampler;

  // A stream took over the rate of another one that played at the tuning
  // factor 1.0 and the increment 2 * UPSAMPLE_FACTOR; now the provider tuning
  // factor is 1.5
  sampler.InitResamplerIncrement(1.0f, UPSAMPLE_FACTOR);
  sampler.EnsureTuningFactor(1.2f);
  sampler.InitResamplerIncrement(1.5f, 2 * UPSAMPLE_FACTOR);
  GOAssert(
    sampler.m_InitialTuningFactor == 1.5f
      && sampler.m_AppliedTuningFactor == 1.5f
      && sampler.m_AppliedResamplerIncrement == 2 * UPSAMPLE_FACTOR,
    "the new values must replace the old ones");
  sampler.EnsureTuningFactor(3.0f);
  GOAssert(
    sampler.m_AppliedResamplerIncrement == 4 * UPSAMPLE_FACTOR,
    "later tuning changes must be scaled from the new initial values");
}

void GOTestSoundSampler::TestVolumeFactorFollowsProvider() {
  GOSoundSampler sampler;

  sampler.InitVolumeFactor(0.5f);
  GOAssert(
    sampler.m_AppliedVolumeRatio == 1.0f,
    "the ratio must be 1.0 right after the start");
  sampler.EnsureVolumeFactor(0.5f);
  GOAssert(
    sampler.m_AppliedVolumeRatio == 1.0f,
    "the initial volume factor must keep the ratio 1.0");
  sampler.EnsureVolumeFactor(1.0f);
  GOAssert(
    sampler.m_AppliedVolumeRatio == 2.0f,
    "the doubled volume factor must give the ratio 2.0");
  sampler.EnsureVolumeFactor(0.25f);
  GOAssert(
    sampler.m_AppliedVolumeRatio == 0.5f,
    "the ratio must be computed from the initial factor, not the previous one");
  sampler.EnsureVolumeFactor(0.5f);
  GOAssert(
    sampler.m_AppliedVolumeRatio == 1.0f,
    "a return to the initial volume factor must return exactly to the ratio "
    "1.0");
}

void GOTestSoundSampler::TestVolumeFactorIsEnsuredOnlyOnChange() {
  GOSoundSampler sampler;
  const float marker = 12345.0f;

  sampler.InitVolumeFactor(1.0f);
  sampler.EnsureVolumeFactor(2.0f);
  sampler.m_AppliedVolumeRatio = marker;
  sampler.EnsureVolumeFactor(2.0f);
  GOAssert(
    sampler.m_AppliedVolumeRatio == marker,
    "the same volume factor must not recompute the ratio");
}

void GOTestSoundSampler::TestSilentSamplerIsNotRaised() {
  GOSoundSampler sampler;

  // The fader target volume of a sampler started with the factor 0 is 0, so a
  // ratio could not raise it
  sampler.InitVolumeFactor(0.0f);
  sampler.EnsureVolumeFactor(1.0f);
  GOAssert(
    sampler.m_AppliedVolumeRatio == 1.0f,
    "a sampler started silent must keep the ratio 1.0");
}

void GOTestSoundSampler::run() {
  GO_RUN_TEST(TestTuningFactorFollowsProvider())
  GO_RUN_TEST(TestTuningFactorDoesNotDrift())
  GO_RUN_TEST(TestTuningFactorIsEnsuredOnlyOnChange())
  GO_RUN_TEST(TestInitResamplerIncrementRebasesInitial())
  GO_RUN_TEST(TestVolumeFactorFollowsProvider())
  GO_RUN_TEST(TestVolumeFactorIsEnsuredOnlyOnChange())
  GO_RUN_TEST(TestSilentSamplerIsNotRaised())
}
