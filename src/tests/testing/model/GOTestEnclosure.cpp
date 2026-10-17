/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestEnclosure.h"

#include <cmath>

#include "config/GOConfig.h"
#include "model/GOEnclosure.h"
#include "model/GOOrganModel.h"

#include "GOTestScope.h"

const std::string GOTestEnclosure::TEST_NAME = "GOTestEnclosure";

namespace {

// A minimal, standalone organ model for GOEnclosure's constructor
// reference. Safe without loading an ODF: neither GOConfig nor
// GOOrganModel does any I/O in their constructors - see
// GOSoundWindchestGroupTestFixture's class comment for the fuller
// justification of this pattern.
struct GOEnclosureTestModel {
  GOConfig config;
  GOOrganModel organModel;

  GOEnclosureTestModel() : config("GOTestEnclosure", ""), organModel(config) {}
};

} // namespace

void GOTestEnclosure::TestFreshEnclosureHasZeroShelfConfig() {
  GOEnclosureTestModel model;
  GOEnclosure enclosure(model.organModel);

  GOAssert(
    enclosure.GetDefaultLowShelfFrequency() == 0,
    "default low shelf frequency must start at 0");
  GOAssert(
    enclosure.GetDefaultLowShelfAttenuationDb() == 0,
    "default low shelf attenuation must start at 0");
  GOAssert(
    enclosure.GetDefaultHighShelfFrequency() == 0,
    "default high shelf frequency must start at 0");
  GOAssert(
    enclosure.GetDefaultHighShelfAttenuationDb() == 0,
    "default high shelf attenuation must start at 0");
  GOAssert(
    enclosure.GetLowShelfFrequency() == 0,
    "current low shelf frequency must start at 0");
  GOAssert(
    enclosure.GetLowShelfAttenuationDb() == 0,
    "current low shelf attenuation must start at 0");
  GOAssert(
    enclosure.GetHighShelfFrequency() == 0,
    "current high shelf frequency must start at 0");
  GOAssert(
    enclosure.GetHighShelfAttenuationDb() == 0,
    "current high shelf attenuation must start at 0");
}

void GOTestEnclosure::TestShelfAccessorsRoundTrip() {
  GOEnclosureTestModel model;
  GOEnclosure enclosure(model.organModel);

  enclosure.SetLowShelfFrequency(1220.0f);
  enclosure.SetLowShelfAttenuationDb(12.0f);
  enclosure.SetHighShelfFrequency(18000.0f);
  enclosure.SetHighShelfAttenuationDb(6.0f);

  GOAssert(
    enclosure.GetLowShelfFrequency() == 1220.0f,
    "low shelf frequency must round-trip");
  GOAssert(
    enclosure.GetLowShelfAttenuationDb() == 12.0f,
    "low shelf attenuation must round-trip");
  GOAssert(
    enclosure.GetHighShelfFrequency() == 18000.0f,
    "high shelf frequency must round-trip");
  GOAssert(
    enclosure.GetHighShelfAttenuationDb() == 6.0f,
    "high shelf attenuation must round-trip");
}

void GOTestEnclosure::TestGetAttenuationUnchanged() {
  GOEnclosureTestModel model;
  GOEnclosure enclosure(model.organModel);

  enclosure.SetAmpMinimumLevel(20);

  enclosure.SetEnclosureValue(127);
  GOAssert(
    enclosure.GetAttenuation() == 1.0f,
    "fully open must give full attenuation (1.0), regardless of the "
    "minimum level");

  enclosure.SetEnclosureValue(0);
  /* Not an exact comparison: the release build is compiled with -ffast-math,
   * which turns 20 / 100.0f into 20 * 0.01f, one bit away from 0.2f */
  GOAssert(
    std::fabs(enclosure.GetAttenuation() - 0.2f) < 1e-6f,
    "fully closed must give the minimum level as a fraction (20/100 = 0.2)");

  enclosure.SetEnclosureValue(50);

  const float expected
    = (50 * (100 - 20) + 127 * 20) / 12700.0f; // pre-refactor formula
  GOAssert(
    enclosure.GetAttenuation() == expected,
    "an intermediate position must still match the original "
    "min/100 + midi*(100-min)/12700 formula");
}

void GOTestEnclosure::TestCurrentShelfGainDbTracksPosition() {
  GOEnclosureTestModel model;
  GOEnclosure enclosure(model.organModel);

  enclosure.SetLowShelfAttenuationDb(12.0f);
  enclosure.SetHighShelfAttenuationDb(6.0f);

  enclosure.SetEnclosureValue(127); // fully open
  GOAssert(
    enclosure.GetCurrentLowShelfGainDb() == 0.0f,
    "low band must be a no-op (0 dB) when fully open");
  GOAssert(
    enclosure.GetCurrentHighShelfGainDb() == 0.0f,
    "high band must be a no-op (0 dB) when fully open");

  enclosure.SetEnclosureValue(0); // fully closed
  GOAssert(
    enclosure.GetCurrentLowShelfGainDb() == -12.0f,
    "low band must reach the full configured attenuation when fully "
    "closed");
  GOAssert(
    enclosure.GetCurrentHighShelfGainDb() == -6.0f,
    "high band must reach the full configured attenuation when fully "
    "closed");

  for (uint8_t midiValue : {0, 32, 64, 96, 127}) {
    enclosure.SetEnclosureValue(midiValue);
    GOAssert(
      enclosure.GetCurrentLowShelfGainDb() <= 0.0f,
      "low band gain must never be positive, at any position");
    GOAssert(
      enclosure.GetCurrentHighShelfGainDb() <= 0.0f,
      "high band gain must never be positive, at any position");
  }
}

void GOTestEnclosure::run() {
  GO_RUN_TEST(TestFreshEnclosureHasZeroShelfConfig())
  GO_RUN_TEST(TestShelfAccessorsRoundTrip())
  GO_RUN_TEST(TestGetAttenuationUnchanged())
  GO_RUN_TEST(TestCurrentShelfGainDbTracksPosition())
}
