/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundOrganEngineFactories.h"

#include <format>
#include <vector>

#include "config/GOAudioDeviceConfig.h"
#include "sound/GOSoundOrganEngine.h"
#include "sound/tasks/GOSoundBufferTaskBase.h"
#include "sound/tasks/GOSoundOutputTask.h"

const std::string GOTestSoundOrganEngineFactories::TEST_NAME
  = "GOTestSoundOrganEngineFactories";

namespace {

// A stub 2-channel group-task input, filled with a known constant per
// channel, standing in for a real GOSoundGroupTask.
class StubGroupTask : public GOSoundBufferTaskBase {
public:
  StubGroupTask(unsigned nFrames, float leftValue, float rightValue)
    : GOSoundBufferTaskBase(2, nFrames) {
    for (unsigned frameI = 0; frameI < nFrames; frameI++) {
      GetData()[GetItemIndex(frameI, 0)] = leftValue;
      GetData()[GetItemIndex(frameI, 1)] = rightValue;
    }
  }

  unsigned GetGroup() override { return AUDIOGROUP; }
  unsigned GetCost() override { return 0; }
  bool GetRepeat() override { return false; }
  void Run(GOSoundThread *) override {}
  void Exec() override {}
  void Finish(bool, GOSoundThread *) override {}
  void Clear() override {}
  void Reset() override {}
};

std::vector<float> convertGainsToScaleFactors(const std::vector<float> &gains) {
  std::vector<float> scaleFactors(gains.size());

  for (unsigned i = 0, n = gains.size(); i < n; i++)
    scaleFactors[i] = GOSoundOrganEngine::convertGainToScaleFactor(gains[i]);
  return scaleFactors;
}

} // namespace

void GOTestSoundOrganEngineFactories::TestDownmixGainsWithOneGroup() {
  const std::vector<float> gains = GOSoundOrganEngine::createDownmixGains(1);

  GOAssert(gains.size() == 4, "1 group: gains should have 4 entries");
  GOAssert(gains[0] == 0.0f, "1 group: L should take group0.left");
  GOAssert(
    gains[1] == GOAudioDeviceConfig::MUTE_VOLUME,
    "1 group: L should not take group0.right");
  GOAssert(
    gains[2] == GOAudioDeviceConfig::MUTE_VOLUME,
    "1 group: R should not take group0.left");
  GOAssert(gains[3] == 0.0f, "1 group: R should take group0.right");
}

void GOTestSoundOrganEngineFactories::TestDownmixGainsWithTwoGroups() {
  static constexpr unsigned N_FRAMES = 4;
  static constexpr float GROUP0_LEFT = 0.1f;
  static constexpr float GROUP0_RIGHT = 0.2f;
  static constexpr float GROUP1_LEFT = 0.4f;
  static constexpr float GROUP1_RIGHT = 0.8f;

  StubGroupTask group0(N_FRAMES, GROUP0_LEFT, GROUP0_RIGHT);
  StubGroupTask group1(N_FRAMES, GROUP1_LEFT, GROUP1_RIGHT);

  GOSoundOutputTask downmixTask(
    2,
    convertGainsToScaleFactors(GOSoundOrganEngine::createDownmixGains(2)),
    N_FRAMES);

  downmixTask.SetOutputs({&group0, &group1});
  downmixTask.Run();

  const float *pData = downmixTask.GetData();
  const float expectedLeft = GROUP0_LEFT + GROUP1_LEFT;
  const float expectedRight = GROUP0_RIGHT + GROUP1_RIGHT;

  for (unsigned frameI = 0; frameI < N_FRAMES; frameI++) {
    GOAssert(
      pData[downmixTask.GetItemIndex(frameI, 0)] == expectedLeft,
      std::format(
        "2 groups, frame {}: downmix L should be group0.left + group1.left, "
        "not a group0.left + group1.right mixup",
        frameI));
    GOAssert(
      pData[downmixTask.GetItemIndex(frameI, 1)] == expectedRight,
      std::format(
        "2 groups, frame {}: downmix R should be group0.right + "
        "group1.right, not a group0.left + group1.right mixup",
        frameI));
  }
}

void GOTestSoundOrganEngineFactories::TestDefaultOutputConfigs() {
  for (unsigned nAudioGroups = 1; nAudioGroups <= 2; nAudioGroups++) {
    const std::vector<GOSoundOrganEngine::AudioOutputConfig> configs
      = GOSoundOrganEngine::createDefaultOutputConfigs(nAudioGroups);

    GOAssert(
      configs.size() == 1,
      std::format(
        "{} group(s): should produce exactly one output", nAudioGroups));

    const GOSoundOrganEngine::AudioOutputConfig &config = configs[0];

    GOAssert(
      config.channels == 2,
      std::format("{} group(s): output should be stereo", nAudioGroups));
    GOAssert(
      config.scaleFactors.size() == 2,
      std::format(
        "{} group(s): scaleFactors should have one entry per channel",
        nAudioGroups));

    for (unsigned groupI = 0; groupI < nAudioGroups; groupI++) {
      GOAssert(
        config.scaleFactors[0][groupI * 2] == 0.0f,
        std::format(
          "{} group(s): L should pass through group {}'s left channel",
          nAudioGroups,
          groupI));
      GOAssert(
        config.scaleFactors[0][groupI * 2 + 1]
          == GOAudioDeviceConfig::MUTE_VOLUME,
        std::format(
          "{} group(s): L should mute group {}'s right channel",
          nAudioGroups,
          groupI));
      GOAssert(
        config.scaleFactors[1][groupI * 2] == GOAudioDeviceConfig::MUTE_VOLUME,
        std::format(
          "{} group(s): R should mute group {}'s left channel",
          nAudioGroups,
          groupI));
      GOAssert(
        config.scaleFactors[1][groupI * 2 + 1] == 0.0f,
        std::format(
          "{} group(s): R should pass through group {}'s right channel",
          nAudioGroups,
          groupI));
    }
  }
}

void GOTestSoundOrganEngineFactories::
  TestDefaultOutputConfigsMatchesDownmixGains() {
  for (unsigned nAudioGroups = 1; nAudioGroups <= 2; nAudioGroups++) {
    const std::vector<GOSoundOrganEngine::AudioOutputConfig> configs
      = GOSoundOrganEngine::createDefaultOutputConfigs(nAudioGroups);
    const GOSoundOrganEngine::AudioOutputConfig &config = configs[0];
    const std::vector<float> gains
      = GOSoundOrganEngine::createDownmixGains(nAudioGroups);
    const unsigned nOutputCount = nAudioGroups * 2;

    for (unsigned channelI = 0; channelI < 2; channelI++)
      for (unsigned k = 0; k < nOutputCount; k++)
        GOAssert(
          config.scaleFactors[channelI][k]
            == gains[channelI * nOutputCount + k],
          std::format(
            "{} group(s), channel {}, index {}: createDefaultOutputConfigs() "
            "must reuse createDownmixGains()'s pattern, not a diverging copy "
            "of it",
            nAudioGroups,
            channelI,
            k));
  }
}

void GOTestSoundOrganEngineFactories::run() {
  TestDownmixGainsWithOneGroup();
  TestDownmixGainsWithTwoGroups();
  TestDefaultOutputConfigs();
  TestDefaultOutputConfigsMatchesDownmixGains();
}
