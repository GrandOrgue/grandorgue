/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundReverb.h"

#include <cmath>

#include "sound/reverb/GOSoundReverb.h"

const std::string GOTestSoundReverb::TEST_NAME = "GOTestSoundReverb";

// resources/test_ir.wav: 10 mono float32 samples, 0.1, 0.2, ..., 1.0, at
// 44100 Hz.
static const std::string TEST_IR_WAV_PATH
  = GO_TEST_RESOURCES_DIR "/sound/reverb/test-ir.wav";
static constexpr unsigned TEST_IR_SAMPLE_RATE = 44100;

static GOSoundReverb::ReverbConfig make_config(
  unsigned channel = 1,
  unsigned startOffset = 0,
  unsigned len = 0,
  unsigned delay = 0,
  float gain = 1.0f,
  bool isDirect = false,
  const std::string &file = TEST_IR_WAV_PATH) {
  return {
    .isEnabled = true,
    .isDirect = isDirect,
    .channel = channel,
    .startOffset = startOffset,
    .len = len,
    .delay = delay,
    .gain = gain,
    .file = file,
  };
}

void GOTestSoundReverb::TestLoadIRDataAppliesGain() {
  const GOSoundReverb::IRData irData = GOSoundReverb::loadIRData(
    make_config(1, 0, 0, 0, 2.0f), TEST_IR_SAMPLE_RATE);

  GOAssert(
    irData.data.size() == 10, "all 10 samples must load with no trimming");
  for (unsigned i = 0; i < irData.data.size(); i++)
    GOAssert(
      std::abs(irData.data[i] - 0.2f * (i + 1)) < 1e-5f,
      "every loaded sample must be multiplied by gain");
}

void GOTestSoundReverb::TestLoadIRDataTrimsOffsetAndLen() {
  const GOSoundReverb::IRData irData
    = GOSoundReverb::loadIRData(make_config(1, 2, 3), TEST_IR_SAMPLE_RATE);

  GOAssert(
    irData.data.size() == 3,
    "startOffset=2, len=3 must keep exactly 3 samples");
  const float expected[] = {0.3f, 0.4f, 0.5f};
  for (unsigned i = 0; i < 3; i++)
    GOAssert(
      std::abs(irData.data[i] - expected[i]) < 1e-5f,
      "the kept samples must be the ones at [startOffset, startOffset+len)");
}

void GOTestSoundReverb::TestLoadIRDataComputesDelayFromMs() {
  const GOSoundReverb::IRData irData
    = GOSoundReverb::loadIRData(make_config(1, 0, 0, 500), TEST_IR_SAMPLE_RATE);

  GOAssert(
    irData.delay == (TEST_IR_SAMPLE_RATE * 500) / 1000,
    "delay must be config.delay milliseconds converted to samples at the "
    "target sample rate");
}

void GOTestSoundReverb::TestLoadIRDataPassesThroughIsDirect() {
  const GOSoundReverb::IRData irDataDirect = GOSoundReverb::loadIRData(
    make_config(1, 0, 0, 0, 1.0f, true), TEST_IR_SAMPLE_RATE);
  const GOSoundReverb::IRData irDataNotDirect = GOSoundReverb::loadIRData(
    make_config(1, 0, 0, 0, 1.0f, false), TEST_IR_SAMPLE_RATE);

  GOAssert(irDataDirect.isDirect, "isDirect=true must pass through");
  GOAssert(!irDataNotDirect.isDirect, "isDirect=false must pass through");
}

void GOTestSoundReverb::TestLoadIRDataThrowsOnMissingFile() {
  bool didThrow = false;

  try {
    GOSoundReverb::loadIRData(
      make_config(1, 0, 0, 0, 1.0f, false, "/nonexistent/path.wav"),
      TEST_IR_SAMPLE_RATE);
  } catch (const wxString &) {
    didThrow = true;
  }

  GOAssert(
    didThrow,
    "loadIRData() must throw wxString on a missing file, same as Setup() "
    "expects to catch");
}

void GOTestSoundReverb::run() {
  TestLoadIRDataAppliesGain();
  TestLoadIRDataTrimsOffsetAndLen();
  TestLoadIRDataComputesDelayFromMs();
  TestLoadIRDataPassesThroughIsDirect();
  TestLoadIRDataThrowsOnMissingFile();
}
