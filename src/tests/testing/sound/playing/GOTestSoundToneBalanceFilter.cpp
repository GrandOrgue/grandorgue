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
#include "sound/providers/GOSoundProvider.h"

#include "GOTestScope.h"

const std::string GOTestSoundToneBalanceFilter::TEST_NAME
  = "GOTestSoundToneBalanceFilter";

static constexpr unsigned TEST_SAMPLE_RATE = 44100;
static constexpr unsigned TEST_N_FRAMES = 64;

/** Sets the provider up the way GOSoundingPipe::PreparePlayback() does */
static void setup_provider(GOSoundProvider &provider, int8_t value) {
  provider.SetToneBalanceFilterSamplerate(TEST_SAMPLE_RATE);
  provider.SetToneBalanceValue(value);
}

void GOTestSoundToneBalanceFilter::TestInitAndIsToApply() {
  GOSoundProvider provider;
  GOSoundToneBalanceFilter::State state;

  setup_provider(provider, 0);
  state.EnsureCoeffs(provider.GetToneFilterCoeffs());
  GOAssert(!state.IsToApply(), "value == 0 must disable filtering");

  setup_provider(provider, -50);
  state.EnsureCoeffs(provider.GetToneFilterCoeffs());
  GOAssert(state.IsToApply(), "a negative value must enable an LPF");

  setup_provider(provider, 50);
  state.EnsureCoeffs(provider.GetToneFilterCoeffs());
  GOAssert(state.IsToApply(), "a positive value must enable an HPF");
}

void GOTestSoundToneBalanceFilter::TestSetSamplerateKeepsValue() {
  GOSoundProvider provider;
  GOSoundToneBalanceFilter::State state;

  setup_provider(provider, -50);
  state.EnsureCoeffs(provider.GetToneFilterCoeffs());

  const GOSoundOnePoleFilter::Coeffs coeffsAtFirstRate = state.GetCoeffs();

  provider.SetToneBalanceFilterSamplerate(TEST_SAMPLE_RATE * 2);
  GOAssert(
    provider.GetToneBalanceValue() == -50,
    "SetSamplerate() must keep the last value");
  state.EnsureCoeffs(provider.GetToneFilterCoeffs());
  GOAssert(
    state.IsToApply(),
    "SetSamplerate() must recompute the coefficients of the last value "
    "instead of publishing an unfiltered set");
  GOAssert(
    coeffsAtFirstRate.a1 != state.GetCoeffs().a1,
    "the coefficients must be recomputed for the new sample rate");
}

void GOTestSoundToneBalanceFilter::TestSetSamplerateRepublishesOnce() {
  GOSoundProvider provider;
  GOSoundToneBalanceFilter::State state;

  setup_provider(provider, -50);
  state.EnsureCoeffs(provider.GetToneFilterCoeffs());
  GOAssert(state.IsToApply(), "test precondition: filter must be active");

  // A reader that renews after SetSamplerate() must never be able to see an
  // intermediate unfiltered set: it receives the final one at once
  provider.SetToneBalanceFilterSamplerate(TEST_SAMPLE_RATE / 2);

  GOSoundToneBalanceFilter::CoeffsVersioned::Snapshot snapshot;

  GOAssert(
    provider.GetToneFilterCoeffs().RenewSnapshot(snapshot),
    "SetSamplerate() must publish");
  GOAssert(
    snapshot.Get().isActive,
    "the only published set after SetSamplerate() must be the active one");
  GOAssert(
    !provider.GetToneFilterCoeffs().RenewSnapshot(snapshot),
    "SetSamplerate() must publish exactly once");
}

void GOTestSoundToneBalanceFilter::TestStateIsToApply() {
  GOSoundProvider provider;
  GOSoundToneBalanceFilter::State state;

  state.EnsureCoeffs(provider.GetToneFilterCoeffs());
  GOAssert(
    !state.IsToApply(), "a provider with no tone balance must be a no-op");

  setup_provider(provider, 0);
  state.EnsureCoeffs(provider.GetToneFilterCoeffs());
  GOAssert(
    !state.IsToApply(), "a State following a no-op filter must be a no-op");

  setup_provider(provider, -50);
  state.EnsureCoeffs(provider.GetToneFilterCoeffs());
  GOAssert(
    state.IsToApply(),
    "a State following a configured filter must not be a no-op");
}

void GOTestSoundToneBalanceFilter::TestStateProcessBufferFilters() {
  GOSoundProvider provider;

  setup_provider(provider, -99);

  GOSoundToneBalanceFilter::State state;

  state.EnsureCoeffs(provider.GetToneFilterCoeffs());
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
  GOSoundProvider provider;
  GOSoundToneBalanceFilter::State state;

  setup_provider(provider, value);
  state.EnsureCoeffs(provider.GetToneFilterCoeffs());
  return state.GetCoeffs();
}

static bool are_same_coeffs(
  const GOSoundOnePoleFilter::Coeffs &a,
  const GOSoundOnePoleFilter::Coeffs &b) {
  return a.b0 == b.b0 && a.b1 == b.b1 && a.a1 == b.a1
    && a.isActive == b.isActive;
}

void GOTestSoundToneBalanceFilter::TestStateFollowsFilterChanges() {
  GOSoundProvider provider;
  GOSoundToneBalanceFilter::State state;

  setup_provider(provider, 0);
  state.EnsureCoeffs(provider.GetToneFilterCoeffs());
  GOAssert(!state.IsToApply(), "the filter is off at the start");

  provider.SetToneBalanceValue(-50);
  state.EnsureCoeffs(provider.GetToneFilterCoeffs());
  GOAssert(state.IsToApply(), "the State must follow switching the filter on");
  GOAssert(
    are_same_coeffs(state.GetCoeffs(), coeffs_for(-50)),
    "the State must use the new coefficients");

  provider.SetToneBalanceValue(-40);
  state.EnsureCoeffs(provider.GetToneFilterCoeffs());
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

  provider.SetToneBalanceValue(0);
  state.EnsureCoeffs(provider.GetToneFilterCoeffs());
  GOAssert(!state.IsToApply(), "the State must follow switching off");
  provider.SetToneBalanceValue(-40);
  state.EnsureCoeffs(provider.GetToneFilterCoeffs());
  GOAssert(state.IsToApply(), "the State must follow switching on again");

  for (unsigned i = 0; i < buffer.GetNItems(); i++)
    pData[i] = 0.0f;
  state.ProcessBuffer(buffer);
  for (unsigned i = 0; i < buffer.GetNItems(); i++)
    GOAssert(
      pData[i] == 0.0f,
      "the memory of the filter must be cleared when it is switched on again");
}

void GOTestSoundToneBalanceFilter::TestEnsureCoeffsIsIdempotent() {
  GOSoundProvider provider;
  GOSoundToneBalanceFilter::State state;

  setup_provider(provider, -40);
  state.EnsureCoeffs(provider.GetToneFilterCoeffs());

  // build up some memory
  GO_DECLARE_LOCAL_SOUND_BUFFER(buffer, 2, TEST_N_FRAMES);

  float *pData = buffer.GetData();

  for (unsigned i = 0; i < buffer.GetNItems(); i++)
    pData[i] = 1.0f;
  state.ProcessBuffer(buffer);

  state.EnsureCoeffs(provider.GetToneFilterCoeffs());
  state.EnsureCoeffs(provider.GetToneFilterCoeffs());
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

void GOTestSoundToneBalanceFilter::TestStateCopyKeepsMemory() {
  GOSoundProvider provider;
  GOSoundToneBalanceFilter::State state;

  setup_provider(provider, -40);
  state.EnsureCoeffs(provider.GetToneFilterCoeffs());

  // build up some memory
  GO_DECLARE_LOCAL_SOUND_BUFFER(buffer, 2, TEST_N_FRAMES);

  const unsigned nItems = buffer.GetNItems();
  float *pData = buffer.GetData();

  for (unsigned itemI = 0; itemI < nItems; ++itemI)
    pData[itemI] = 1.0f;
  state.ProcessBuffer(buffer);

  // a copy keeps the memory and follows the same provider as the original
  GOSoundToneBalanceFilter::State copy = state;

  // an active-to-active change does not clear the memory
  provider.SetToneBalanceValue(-50);
  state.EnsureCoeffs(provider.GetToneFilterCoeffs());
  copy.EnsureCoeffs(provider.GetToneFilterCoeffs());
  GOAssert(
    are_same_coeffs(copy.GetCoeffs(), coeffs_for(-50)),
    "a copy of a State must follow the same provider");

  GO_DECLARE_LOCAL_SOUND_BUFFER(copyBuffer, 2, TEST_N_FRAMES);

  float *pCopyData = copyBuffer.GetData();

  for (unsigned itemI = 0; itemI < nItems; ++itemI) {
    pData[itemI] = 0.0f;
    pCopyData[itemI] = 0.0f;
  }
  state.ProcessBuffer(buffer);
  copy.ProcessBuffer(copyBuffer);

  bool hasMemory = false;

  for (unsigned itemI = 0; itemI < nItems; ++itemI)
    if (pData[itemI] != 0.0f)
      hasMemory = true;
  GOAssert(hasMemory, "test precondition: the State must carry some memory");
  for (unsigned itemI = 0; itemI < nItems; ++itemI)
    GOAssert(
      pData[itemI] == pCopyData[itemI],
      "a copy of an active State must produce the same output as the "
      "original");
}

void GOTestSoundToneBalanceFilter::TestStateNeverSeesTornCoefficients() {
  const GOSoundOnePoleFilter::Coeffs lowPass = coeffs_for(-99);
  const GOSoundOnePoleFilter::Coeffs highPass = coeffs_for(99);
  const GOSoundOnePoleFilter::Coeffs off = coeffs_for(0);

  GOSoundProvider provider;
  GOSoundToneBalanceFilter::State state;

  setup_provider(provider, -99);

  std::atomic_bool isStopped(false);
  std::thread writer([&]() {
    const int8_t values[] = {-99, 99, 0};

    for (unsigned i = 0; !isStopped.load(); i++)
      provider.SetToneBalanceValue(values[i % 3]);
  });

  unsigned nSeenSets = 0;
  const GOSoundOnePoleFilter::Coeffs *pLastSeen = nullptr;

  for (unsigned i = 0; i < 2000000; i++) {
    state.EnsureCoeffs(provider.GetToneFilterCoeffs());

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
  GO_RUN_TEST(TestStateCopyKeepsMemory())
  GO_RUN_TEST(TestStateNeverSeesTornCoefficients())
  GO_RUN_TEST(TestInitAndIsToApply())
  GO_RUN_TEST(TestSetSamplerateKeepsValue())
  GO_RUN_TEST(TestSetSamplerateRepublishesOnce())
  GO_RUN_TEST(TestStateIsToApply())
  GO_RUN_TEST(TestStateProcessBufferFilters())
}
