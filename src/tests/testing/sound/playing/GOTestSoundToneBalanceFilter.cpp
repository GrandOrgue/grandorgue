/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundToneBalanceFilter.h"

#include <atomic>
#include <thread>

#include "sound/buffer/GOSoundBufferMutable.h"
#include "sound/playing/GOSoundToneBalanceFilter.h"

#include "GOTestScope.h"

const std::string GOTestSoundToneBalanceFilter::TEST_NAME
  = "GOTestSoundToneBalanceFilter";

static constexpr unsigned TEST_SAMPLE_RATE = 44100;
static constexpr unsigned TEST_N_FRAMES = 64;

void GOTestSoundToneBalanceFilter::TestInitAndIsToApply() {
  GOSoundToneBalanceFilter filter;

  filter.SetSamplerate(TEST_SAMPLE_RATE);
  filter.Init(0);
  GOAssert(!filter.IsToApply(), "value == 0 must disable filtering");

  filter.Init(-50);
  GOAssert(filter.IsToApply(), "a negative value must enable an LPF");

  filter.Init(50);
  GOAssert(filter.IsToApply(), "a positive value must enable an HPF");
}

void GOTestSoundToneBalanceFilter::TestSetSamplerateResetsToIdentity() {
  GOSoundToneBalanceFilter filter;

  filter.SetSamplerate(TEST_SAMPLE_RATE);
  filter.Init(-50);
  GOAssert(filter.IsToApply(), "must be configured before SetSamplerate()");

  filter.SetSamplerate(TEST_SAMPLE_RATE);
  GOAssert(
    !filter.IsToApply(),
    "SetSamplerate() must reset the instance to the identity filter until "
    "Init() is called again");
}

void GOTestSoundToneBalanceFilter::TestStateIsToApply() {
  GOSoundToneBalanceFilter::State state;

  state.EnsureCoeffs();
  GOAssert(!state.IsToApply(), "an unbound (default) State must be a no-op");

  GOSoundToneBalanceFilter filter;

  filter.SetSamplerate(TEST_SAMPLE_RATE);
  filter.Init(0);
  state.Init(&filter);
  state.EnsureCoeffs();
  GOAssert(
    !state.IsToApply(), "a State bound to a no-op filter must be a no-op");

  filter.Init(-50);
  state.Init(&filter);
  state.EnsureCoeffs();
  GOAssert(
    state.IsToApply(),
    "a State bound to a configured filter must not be a no-op");
}

void GOTestSoundToneBalanceFilter::TestStateProcessBufferFilters() {
  GOSoundToneBalanceFilter filter;

  filter.SetSamplerate(TEST_SAMPLE_RATE);
  filter.Init(-99);

  GOSoundToneBalanceFilter::State state;

  state.Init(&filter);
  state.EnsureCoeffs();
  GOAssert(state.IsToApply(), "test precondition: filter must be configured");

  GO_DECLARE_LOCAL_SOUND_BUFFER(buffer, 2, TEST_N_FRAMES);

  float *pData = buffer.GetData();

  for (unsigned i = 0; i < buffer.GetNItems(); i++)
    pData[i] = (i % 2 == 0) ? 1.0f : -1.0f;

  state.ProcessBuffer(buffer);

  bool wasChanged = false;

  for (unsigned i = 0; i < buffer.GetNItems(); i++)
    if (pData[i] != ((i % 2 == 0) ? 1.0f : -1.0f))
      wasChanged = true;

  GOAssert(
    wasChanged,
    "ProcessBuffer() with a configured filter must actually alter the "
    "audio");
}

/** Returns the coefficients that Init(value) produces */
static GOSoundOnePoleFilter::Coeffs coeffs_for(int8_t value) {
  GOSoundToneBalanceFilter filter;
  GOSoundToneBalanceFilter::State state;

  filter.SetSamplerate(TEST_SAMPLE_RATE);
  filter.Init(value);
  state.Init(&filter);
  state.EnsureCoeffs();
  return state.GetCoeffs();
}

static bool are_same_coeffs(
  const GOSoundOnePoleFilter::Coeffs &a,
  const GOSoundOnePoleFilter::Coeffs &b) {
  return a.b0 == b.b0 && a.b1 == b.b1 && a.a1 == b.a1
    && a.isActive == b.isActive;
}

void GOTestSoundToneBalanceFilter::TestStateFollowsFilterChanges() {
  GOSoundToneBalanceFilter filter;
  GOSoundToneBalanceFilter::State state;

  filter.SetSamplerate(TEST_SAMPLE_RATE);
  filter.Init(0);
  state.Init(&filter);
  state.EnsureCoeffs();
  GOAssert(!state.IsToApply(), "the filter is off at the start");

  filter.Init(-50);
  state.EnsureCoeffs();
  GOAssert(state.IsToApply(), "the State must follow switching the filter on");
  GOAssert(
    are_same_coeffs(state.GetCoeffs(), coeffs_for(-50)),
    "the State must use the new coefficients");

  filter.Init(-40);
  state.EnsureCoeffs();
  GOAssert(state.IsToApply(), "the filter stays on");
  GOAssert(
    are_same_coeffs(state.GetCoeffs(), coeffs_for(-40)),
    "the State must follow a change of an active filter");

  // build up some memory, then switch off and on again
  GO_DECLARE_LOCAL_SOUND_BUFFER(buffer, 2, TEST_N_FRAMES);

  float *pData = buffer.GetData();

  for (unsigned i = 0; i < buffer.GetNItems(); i++)
    pData[i] = 1.0f;
  state.ProcessBuffer(buffer);

  filter.Init(0);
  state.EnsureCoeffs();
  GOAssert(!state.IsToApply(), "the State must follow switching off");
  filter.Init(-40);
  state.EnsureCoeffs();
  GOAssert(state.IsToApply(), "the State must follow switching on again");

  for (unsigned i = 0; i < buffer.GetNItems(); i++)
    pData[i] = 0.0f;
  state.ProcessBuffer(buffer);
  for (unsigned i = 0; i < buffer.GetNItems(); i++)
    GOAssert(
      pData[i] == 0.0f,
      "the memory of the filter must be cleared when it is switched off");
}

void GOTestSoundToneBalanceFilter::TestEnsureCoeffsIsIdempotent() {
  GOSoundToneBalanceFilter filter;
  GOSoundToneBalanceFilter::State state;

  filter.SetSamplerate(TEST_SAMPLE_RATE);
  filter.Init(-40);
  state.Init(&filter);
  state.EnsureCoeffs();

  // build up some memory
  GO_DECLARE_LOCAL_SOUND_BUFFER(buffer, 2, TEST_N_FRAMES);

  float *pData = buffer.GetData();

  for (unsigned i = 0; i < buffer.GetNItems(); i++)
    pData[i] = 1.0f;
  state.ProcessBuffer(buffer);

  state.EnsureCoeffs();
  state.EnsureCoeffs();
  GOAssert(
    are_same_coeffs(state.GetCoeffs(), coeffs_for(-40)),
    "repeated calls must keep the coefficients");
  for (unsigned i = 0; i < buffer.GetNItems(); i++)
    pData[i] = 0.0f;
  state.ProcessBuffer(buffer);

  bool hasMemory = false;

  for (unsigned i = 0; i < buffer.GetNItems(); i++)
    if (pData[i] != 0.0f)
      hasMemory = true;
  GOAssert(
    hasMemory,
    "repeated calls without a change must not clear the carried memory");
}

void GOTestSoundToneBalanceFilter::TestStateNeverSeesTornCoefficients() {
  const GOSoundOnePoleFilter::Coeffs lowPass = coeffs_for(-99);
  const GOSoundOnePoleFilter::Coeffs highPass = coeffs_for(99);
  const GOSoundOnePoleFilter::Coeffs off = coeffs_for(0);

  GOSoundToneBalanceFilter filter;
  GOSoundToneBalanceFilter::State state;

  filter.SetSamplerate(TEST_SAMPLE_RATE);
  filter.Init(-99);
  state.Init(&filter);

  std::atomic_bool isStopped(false);
  std::thread writer([&]() {
    const int8_t values[] = {-99, 99, 0};

    for (unsigned i = 0; !isStopped.load(); i++)
      filter.Init(values[i % 3]);
  });

  unsigned nSeenSets = 0;
  const GOSoundOnePoleFilter::Coeffs *pLastSeen = nullptr;

  for (unsigned i = 0; i < 2000000; i++) {
    state.EnsureCoeffs();

    const GOSoundOnePoleFilter::Coeffs &coeffs = state.GetCoeffs();
    const GOSoundOnePoleFilter::Coeffs *pSeen = are_same_coeffs(coeffs, lowPass)
      ? &lowPass
      : are_same_coeffs(coeffs, highPass) ? &highPass
      : are_same_coeffs(coeffs, off)      ? &off
                                          : nullptr;

    if (!pSeen) {
      isStopped.store(true);
      writer.join();
      GOAssert(false, "the State used a torn or unknown set of coefficients");
    }
    if (pSeen != pLastSeen) {
      nSeenSets++;
      pLastSeen = pSeen;
    }
  }
  isStopped.store(true);
  writer.join();
  GOAssert(
    nSeenSets > 3,
    "the test must actually observe the filter being changed while running");
}

void GOTestSoundToneBalanceFilter::run() {
  GO_RUN_TEST(TestStateFollowsFilterChanges())
  GO_RUN_TEST(TestEnsureCoeffsIsIdempotent())
  GO_RUN_TEST(TestStateNeverSeesTornCoefficients())
  GO_RUN_TEST(TestInitAndIsToApply())
  GO_RUN_TEST(TestSetSamplerateResetsToIdentity())
  GO_RUN_TEST(TestStateIsToApply())
  GO_RUN_TEST(TestStateProcessBufferFilters())
}
