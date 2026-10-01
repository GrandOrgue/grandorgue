/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundOnePoleFilter.h"

#include "sound/GOSoundOnePoleFilter.h"

const std::string GOTestSoundOnePoleFilter::TEST_NAME
  = "GOTestSoundOnePoleFilter";

static constexpr unsigned TEST_SAMPLE_RATE = 44100;

static void assert_is_identity(
  GOTestSoundOnePoleFilter &test,
  const GOSoundOnePoleFilter::Coeffs &c,
  const std::string &caseName) {
  test.GOAssert(c.isNoop, caseName + ": isNoop must be true");
  test.GOAssert(c.b0 == 1, caseName + ": b0 must be exactly 1");
  test.GOAssert(c.b1 == 0, caseName + ": b1 must be exactly 0");
  test.GOAssert(c.a1 == 0, caseName + ": a1 must be exactly 0");
}

void GOTestSoundOnePoleFilter::TestComputeCoeffsIdentityEdgeCases() {
  GOSoundOnePoleFilter::Coeffs c;

  GOSoundOnePoleFilter::computeCoeffs(
    GOSoundOnePoleFilter::Type::TYPE_LPF, 1000, 0, 0, c);
  assert_is_identity(*this, c, "sampleRate == 0");

  GOSoundOnePoleFilter::computeCoeffs(
    GOSoundOnePoleFilter::Type::TYPE_NONE, 1000, 0, TEST_SAMPLE_RATE, c);
  assert_is_identity(*this, c, "TYPE_NONE");

  GOSoundOnePoleFilter::computeCoeffs(
    GOSoundOnePoleFilter::Type::TYPE_LOW_SHELF, 1000, 0, TEST_SAMPLE_RATE, c);
  assert_is_identity(*this, c, "TYPE_LOW_SHELF, gain == 0");

  GOSoundOnePoleFilter::computeCoeffs(
    GOSoundOnePoleFilter::Type::TYPE_HIGH_SHELF, 1000, 0, TEST_SAMPLE_RATE, c);
  assert_is_identity(*this, c, "TYPE_HIGH_SHELF, gain == 0");
}

void GOTestSoundOnePoleFilter::TestComputeCoeffsPerType() {
  const GOSoundOnePoleFilter::Type types[] = {
    GOSoundOnePoleFilter::Type::TYPE_LPF,
    GOSoundOnePoleFilter::Type::TYPE_HPF,
    GOSoundOnePoleFilter::Type::TYPE_LOW_SHELF,
    GOSoundOnePoleFilter::Type::TYPE_HIGH_SHELF};

  for (GOSoundOnePoleFilter::Type type : types) {
    GOSoundOnePoleFilter::Coeffs c;

    GOSoundOnePoleFilter::computeCoeffs(type, 1000, 6, TEST_SAMPLE_RATE, c);
    GOAssert(!c.isNoop, "a configured filter must not be isNoop");
    GOAssert(
      c.b0 != 1 || c.b1 != 0 || c.a1 != 0,
      "a configured filter's Coeffs must differ from the identity default");
  }
}

void GOTestSoundOnePoleFilter::TestProcessSampleRecurrence() {
  GOSoundOnePoleFilter::Coeffs c;

  c.b0 = 0.5;
  c.b1 = 0.25;
  c.a1 = -0.75;
  c.isNoop = false;

  float state = 0;
  float out0 = GOSoundOnePoleFilter::processSample(c, 1.0f, state);

  GOAssert(out0 == 0.5f, "first sample: out = b0 * in + state(0)");
  GOAssert(state == 0.25f - (-0.75f) * 0.5f, "first sample: state update");

  const float expectedState0 = state;
  float out1 = GOSoundOnePoleFilter::processSample(c, 2.0f, state);

  GOAssert(
    out1 == static_cast<float>(0.5 * 2.0 + expectedState0),
    "second sample: out = b0 * in + carried state");
}

void GOTestSoundOnePoleFilter::run() {
  TestComputeCoeffsIdentityEdgeCases();
  TestComputeCoeffsPerType();
  TestProcessSampleRecurrence();
}
