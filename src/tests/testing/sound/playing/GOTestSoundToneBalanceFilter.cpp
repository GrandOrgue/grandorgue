/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundToneBalanceFilter.h"

#include "sound/buffer/GOSoundBufferMutable.h"
#include "sound/playing/GOSoundToneBalanceFilter.h"

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

  GOAssert(!state.IsToApply(), "an unbound (default) State must be a no-op");

  GOSoundToneBalanceFilter filter;

  filter.SetSamplerate(TEST_SAMPLE_RATE);
  filter.Init(0);
  state.Init(&filter);
  GOAssert(
    !state.IsToApply(), "a State bound to a no-op filter must be a no-op");

  filter.Init(-50);
  state.Init(&filter);
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

void GOTestSoundToneBalanceFilter::run() {
  TestInitAndIsToApply();
  TestSetSamplerateResetsToIdentity();
  TestStateIsToApply();
  TestStateProcessBufferFilters();
}
