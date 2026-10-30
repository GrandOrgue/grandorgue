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

void GOTestSoundSampler::TestIncrementFollowsTuning() {
  GOSoundSampler sampler;

  sampler.InitResamplerIncrement(2.0f, UPSAMPLE_FACTOR);
  GOAssert(
    sampler.m_TargetResamplerIncrement == UPSAMPLE_FACTOR
      && sampler.m_AppliedResamplerIncrement == UPSAMPLE_FACTOR,
    "the initial increment must be both the target and the applied one");
  GOAssert(
    sampler.GetResamplerIncrementFor(2.0f) == UPSAMPLE_FACTOR,
    "the target tuning must give the target increment");
  GOAssert(
    sampler.GetResamplerIncrementFor(4.0f) == 2 * UPSAMPLE_FACTOR,
    "the doubled tuning must double the increment");
  GOAssert(
    sampler.GetResamplerIncrementFor(1.0f) == UPSAMPLE_FACTOR / 2,
    "the halved tuning must halve the increment");
  GOAssert(
    sampler.GetResamplerIncrementFor(1e9f)
      == GOSoundResample::MAX_FRACTION_INCREMENT,
    "a huge tuning must be limited to MAX_FRACTION_INCREMENT");
  GOAssert(
    sampler.GetResamplerIncrementFor(1e-9f) == 1,
    "a tiny tuning must be limited to the increment 1");
}

void GOTestSoundSampler::TestIncrementDoesNotDrift() {
  GOSoundSampler sampler;
  const float centUp = std::pow(2.0f, 100.0f / 1200.0f);

  sampler.InitResamplerIncrement(1.0f, UPSAMPLE_FACTOR);
  for (unsigned i = 0; i < 1000; i++) {
    sampler.m_AppliedResamplerIncrement
      = sampler.GetResamplerIncrementFor(centUp);
    sampler.m_AppliedResamplerIncrement
      = sampler.GetResamplerIncrementFor(1.0f);
  }
  GOAssert(
    sampler.m_AppliedResamplerIncrement == UPSAMPLE_FACTOR,
    "1000 round trips must return exactly to the target increment");
  GOAssert(
    sampler.GetResamplerIncrementFor(centUp)
      == (unsigned)std::lround((double)centUp * UPSAMPLE_FACTOR),
    "the increment must depend on the target and the tuning only");
}

void GOTestSoundSampler::TestInitResamplerIncrementRebasesTarget() {
  GOSoundSampler sampler;

  // A stream took over the rate of another one that played at the tuning 1.0
  // and the increment 2 * UPSAMPLE_FACTOR; now the provider tuning is 1.5
  sampler.InitResamplerIncrement(1.0f, UPSAMPLE_FACTOR);
  sampler.InitResamplerIncrement(1.5f, 2 * UPSAMPLE_FACTOR);
  GOAssert(
    sampler.m_TargetTuning == 1.5f
      && sampler.m_AppliedResamplerIncrement == 2 * UPSAMPLE_FACTOR,
    "the new pair must replace the old one");
  GOAssert(
    sampler.GetResamplerIncrementFor(1.5f) == 2 * UPSAMPLE_FACTOR,
    "the new target tuning must give the new increment");
  GOAssert(
    sampler.GetResamplerIncrementFor(3.0f) == 4 * UPSAMPLE_FACTOR,
    "later tuning changes must be scaled from the new pair");
}

void GOTestSoundSampler::run() {
  GO_RUN_TEST(TestIncrementFollowsTuning())
  GO_RUN_TEST(TestIncrementDoesNotDrift())
  GO_RUN_TEST(TestInitResamplerIncrementRebasesTarget())
}
