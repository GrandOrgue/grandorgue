/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundEnclosureShelfMapper.h"

#include "config/GOConfig.h"
#include "model/GOEnclosure.h"
#include "model/GOOrganModel.h"
#include "sound/GOSoundDefs.h"
#include "sound/dsp-kernels/GOSoundOnePoleFilter.h"
#include "sound/effects/GOSoundShelfFilterProcessor.h"
#include "sound/mappers/GOSoundEnclosureShelfMapper.h"

#include "GOTestScope.h"

const std::string GOTestSoundEnclosureShelfMapper::TEST_NAME
  = "GOTestSoundEnclosureShelfMapper";

static constexpr unsigned SAMPLE_RATE = 96000;
static constexpr unsigned N_FRAMES = 32;

// An impossible Coeffs.b0 value: no real computeCoeffs() output can ever
// produce it, so its presence after a call proves that band was *not*
// recomputed, and its absence proves it *was* - the ambiguity plain
// before/after value comparison can't resolve (recomputed vs. recomputed
// to the same value) never arises.
static constexpr float CANARY = -999.0f;

static constexpr uint8_t BASELINE_MIDI = 64;
static constexpr float BASELINE_LOW_FREQUENCY = 200.0f;
static constexpr float BASELINE_LOW_ATTENUATION_DB = 12.0f;
static constexpr float BASELINE_HIGH_FREQUENCY = 5000.0f;
static constexpr float BASELINE_HIGH_ATTENUATION_DB = 6.0f;

namespace {

// A minimal, standalone fixture for GOSoundEnclosureShelfMapper: a real
// GOEnclosure (needs only a GOOrganModel&, no I/O - see
// GOSoundWindchestGroupTestFixture's class comment for the fuller
// justification) driving a real GOSoundShelfFilterProcessor through the
// mapper under test.
struct MapperFixture {
  GOConfig config;
  GOOrganModel organModel;
  GOEnclosure enclosure;
  GOSoundShelfFilterProcessor processor;
  GOSoundEnclosureShelfMapper mapper;

  MapperFixture()
    : config("GOTestSoundEnclosureShelfMapper", ""),
      organModel(config),
      enclosure(organModel),
      mapper(enclosure, processor) {
    processor.EnsureSetup(MAX_OUTPUT_CHANNELS, N_FRAMES, SAMPLE_RATE);
  }

  /** Sets the enclosure to the baseline position/config and calls
   * EnsureParametersUpToDate() once, so a matrix case can start from a
   * known, already-synced starting point and only touch the one input it
   * tests. */
  void SyncToBaseline() {
    enclosure.SetEnclosureValue(BASELINE_MIDI);
    enclosure.SetLowShelfFrequency(BASELINE_LOW_FREQUENCY);
    enclosure.SetLowShelfAttenuationDb(BASELINE_LOW_ATTENUATION_DB);
    enclosure.SetHighShelfFrequency(BASELINE_HIGH_FREQUENCY);
    enclosure.SetHighShelfAttenuationDb(BASELINE_HIGH_ATTENUATION_DB);
    mapper.EnsureParametersUpToDate();
  }
};

} // namespace

void GOTestSoundEnclosureShelfMapper::StompCanary(
  GOSoundShelfFilterProcessor &processor, bool stompLow, bool stompHigh) {
  if (stompLow)
    processor.m_LowCoeffs.b0 = CANARY;
  if (stompHigh)
    processor.m_HighCoeffs.b0 = CANARY;
}

void GOTestSoundEnclosureShelfMapper::AssertBandNotRecomputed(
  const GOSoundShelfFilterProcessor &processor,
  bool isLowBand,
  const std::string &caseName) {
  const double b0
    = isLowBand ? processor.m_LowCoeffs.b0 : processor.m_HighCoeffs.b0;

  GOAssert(
    b0 == CANARY,
    caseName + ": the " + (isLowBand ? std::string("low") : std::string("high"))
      + " band must not have recomputed");
}

void GOTestSoundEnclosureShelfMapper::AssertBandRecomputed(
  const GOSoundShelfFilterProcessor &processor,
  bool isLowBand,
  double expectedFrequency,
  double expectedGainDb,
  const std::string &caseName) {
  const auto &actual
    = isLowBand ? processor.m_LowCoeffs : processor.m_HighCoeffs;
  GOSoundOnePoleFilter::Coeffs expected;

  GOSoundOnePoleFilter::computeCoeffs(
    isLowBand ? GOSoundOnePoleFilter::Type::TYPE_LOW_SHELF
              : GOSoundOnePoleFilter::Type::TYPE_HIGH_SHELF,
    expectedFrequency,
    expectedGainDb,
    SAMPLE_RATE,
    expected);

  const std::string bandName = isLowBand ? "low" : "high";

  GOAssert(
    actual.b0 != CANARY,
    caseName + ": the " + bandName
      + " band must have recomputed (canary gone)");
  GOAssert(
    actual.b0 == expected.b0 && actual.b1 == expected.b1
      && actual.a1 == expected.a1,
    caseName + ": the recomputed " + bandName
      + " band coeffs must match computeCoeffs()'s independent computation "
        "for the new frequency/gain");
}

void GOTestSoundEnclosureShelfMapper::TestFirstCallAlwaysPushesBothBands() {
  MapperFixture fixture;

  fixture.enclosure.SetEnclosureValue(BASELINE_MIDI);
  fixture.enclosure.SetLowShelfFrequency(BASELINE_LOW_FREQUENCY);
  fixture.enclosure.SetLowShelfAttenuationDb(BASELINE_LOW_ATTENUATION_DB);
  fixture.enclosure.SetHighShelfFrequency(BASELINE_HIGH_FREQUENCY);
  fixture.enclosure.SetHighShelfAttenuationDb(BASELINE_HIGH_ATTENUATION_DB);
  StompCanary(fixture.processor, true, true);

  fixture.mapper.EnsureParametersUpToDate();

  AssertBandRecomputed(
    fixture.processor,
    true,
    BASELINE_LOW_FREQUENCY,
    fixture.enclosure.GetCurrentLowShelfGainDb(),
    "first call");
  AssertBandRecomputed(
    fixture.processor,
    false,
    BASELINE_HIGH_FREQUENCY,
    fixture.enclosure.GetCurrentHighShelfGainDb(),
    "first call");
}

void GOTestSoundEnclosureShelfMapper::
  TestNothingChangedRecomputesNeitherBand() {
  MapperFixture fixture;

  fixture.SyncToBaseline();
  StompCanary(fixture.processor, true, true);

  fixture.mapper.EnsureParametersUpToDate();

  AssertBandNotRecomputed(fixture.processor, true, "nothing changed");
  AssertBandNotRecomputed(fixture.processor, false, "nothing changed");
}

void GOTestSoundEnclosureShelfMapper::TestOnlyLowShelfFrequencyChanged() {
  MapperFixture fixture;

  fixture.SyncToBaseline();
  StompCanary(fixture.processor, true, true);
  fixture.enclosure.SetLowShelfFrequency(BASELINE_LOW_FREQUENCY + 100);

  fixture.mapper.EnsureParametersUpToDate();

  AssertBandRecomputed(
    fixture.processor,
    true,
    BASELINE_LOW_FREQUENCY + 100,
    fixture.enclosure.GetCurrentLowShelfGainDb(),
    "only low shelf frequency changed");
  AssertBandNotRecomputed(
    fixture.processor, false, "only low shelf frequency changed");
}

void GOTestSoundEnclosureShelfMapper::TestOnlyLowShelfAttenuationDbChanged() {
  MapperFixture fixture;

  fixture.SyncToBaseline();
  StompCanary(fixture.processor, true, true);
  fixture.enclosure.SetLowShelfAttenuationDb(BASELINE_LOW_ATTENUATION_DB + 3);

  fixture.mapper.EnsureParametersUpToDate();

  AssertBandRecomputed(
    fixture.processor,
    true,
    BASELINE_LOW_FREQUENCY,
    fixture.enclosure.GetCurrentLowShelfGainDb(),
    "only low shelf attenuation changed");
  AssertBandNotRecomputed(
    fixture.processor, false, "only low shelf attenuation changed");
}

void GOTestSoundEnclosureShelfMapper::TestOnlyHighShelfFrequencyChanged() {
  MapperFixture fixture;

  fixture.SyncToBaseline();
  StompCanary(fixture.processor, true, true);
  fixture.enclosure.SetHighShelfFrequency(BASELINE_HIGH_FREQUENCY + 1000);

  fixture.mapper.EnsureParametersUpToDate();

  AssertBandNotRecomputed(
    fixture.processor, true, "only high shelf frequency changed");
  AssertBandRecomputed(
    fixture.processor,
    false,
    BASELINE_HIGH_FREQUENCY + 1000,
    fixture.enclosure.GetCurrentHighShelfGainDb(),
    "only high shelf frequency changed");
}

void GOTestSoundEnclosureShelfMapper::TestOnlyHighShelfAttenuationDbChanged() {
  MapperFixture fixture;

  fixture.SyncToBaseline();
  StompCanary(fixture.processor, true, true);
  fixture.enclosure.SetHighShelfAttenuationDb(BASELINE_HIGH_ATTENUATION_DB + 3);

  fixture.mapper.EnsureParametersUpToDate();

  AssertBandNotRecomputed(
    fixture.processor, true, "only high shelf attenuation changed");
  AssertBandRecomputed(
    fixture.processor,
    false,
    BASELINE_HIGH_FREQUENCY,
    fixture.enclosure.GetCurrentHighShelfGainDb(),
    "only high shelf attenuation changed");
}

void GOTestSoundEnclosureShelfMapper::TestOnlyPositionChanged() {
  MapperFixture fixture;

  fixture.SyncToBaseline();
  StompCanary(fixture.processor, true, true);
  fixture.enclosure.SetEnclosureValue(BASELINE_MIDI + 20);

  fixture.mapper.EnsureParametersUpToDate();

  AssertBandRecomputed(
    fixture.processor,
    true,
    BASELINE_LOW_FREQUENCY,
    fixture.enclosure.GetCurrentLowShelfGainDb(),
    "only position changed");
  AssertBandRecomputed(
    fixture.processor,
    false,
    BASELINE_HIGH_FREQUENCY,
    fixture.enclosure.GetCurrentHighShelfGainDb(),
    "only position changed");
}

void GOTestSoundEnclosureShelfMapper::
  TestPositionAndOneBandConfigChangedTogether() {
  MapperFixture fixture;

  fixture.SyncToBaseline();
  StompCanary(fixture.processor, true, true);
  fixture.enclosure.SetEnclosureValue(BASELINE_MIDI + 20);
  fixture.enclosure.SetLowShelfFrequency(BASELINE_LOW_FREQUENCY + 100);

  fixture.mapper.EnsureParametersUpToDate();

  AssertBandRecomputed(
    fixture.processor,
    true,
    BASELINE_LOW_FREQUENCY + 100,
    fixture.enclosure.GetCurrentLowShelfGainDb(),
    "position + low shelf frequency changed together");
  AssertBandRecomputed(
    fixture.processor,
    false,
    BASELINE_HIGH_FREQUENCY,
    fixture.enclosure.GetCurrentHighShelfGainDb(),
    "position + low shelf frequency changed together");
}

void GOTestSoundEnclosureShelfMapper::run() {
  GO_RUN_TEST(TestFirstCallAlwaysPushesBothBands())
  GO_RUN_TEST(TestNothingChangedRecomputesNeitherBand())
  GO_RUN_TEST(TestOnlyLowShelfFrequencyChanged())
  GO_RUN_TEST(TestOnlyLowShelfAttenuationDbChanged())
  GO_RUN_TEST(TestOnlyHighShelfFrequencyChanged())
  GO_RUN_TEST(TestOnlyHighShelfAttenuationDbChanged())
  GO_RUN_TEST(TestOnlyPositionChanged())
  GO_RUN_TEST(TestPositionAndOneBandConfigChangedTogether())
}
