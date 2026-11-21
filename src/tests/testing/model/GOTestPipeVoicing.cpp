/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestPipeVoicing.h"

#include <cmath>
#include <memory>
#include <vector>

#include "config/GOConfigFileReader.h"
#include "config/GOConfigReader.h"
#include "config/GOConfigReaderDB.h"
#include "model/pipe-config/GOPipeConfigNode.h"
#include "model/pipe-config/GOPipeConfigTreeNode.h"
#include "model/pipe-config/GOPipeUpdateCallback.h"
#include "model/pipe-config/GOPipeVoicing.h"

#include "GOOrganController.h"
#include "GOTestScope.h"

namespace {
class CountingCallback : public GOPipeUpdateCallback {
public:
  unsigned nAmplitudeUpdates = 0;
  unsigned nTuningUpdates = 0;
  unsigned nToneBalanceUpdates = 0;

  void UpdateAmplitude() override { nAmplitudeUpdates++; }
  void UpdateTuning() override { nTuningUpdates++; }
  void UpdateAudioGroup() override {}
  void UpdateReleaseTail() override {}
  void UpdateToneBalance() override { nToneBalanceUpdates++; }
};

/* root -> windchest -> rank1 (notes 36, 37), rank2 (notes 36, 37) */
class TestTree {
public:
  CountingCallback callback;
  std::unique_ptr<GOPipeConfigTreeNode> oRoot;
  std::unique_ptr<GOPipeConfigTreeNode> oWindchest;
  std::unique_ptr<GOPipeConfigTreeNode> oRank1;
  std::unique_ptr<GOPipeConfigTreeNode> oRank2;
  std::vector<std::unique_ptr<GOPipeConfigNode>> moPipes;
  // pipes[rank * 2 + noteI]
  std::vector<GOPipeConfigNode *> mpPipes;

  TestTree(GOOrganController &controller) {
    GOConfigFileReader emptyFile;
    GOConfigReaderDB ini(controller.GetConfig().ODFCheck());

    ini.ReadData(emptyFile, CMBSetting, false);

    GOConfigReader cfg(
      ini,
      controller.GetConfig().ODFCheck(),
      controller.GetConfig().ODFHw1Check());

    oRoot
      = std::make_unique<GOPipeConfigTreeNode>(nullptr, &controller, &callback);
    oWindchest = std::make_unique<GOPipeConfigTreeNode>(
      oRoot.get(), &controller, &callback);
    oRank1 = std::make_unique<GOPipeConfigTreeNode>(
      oWindchest.get(), &controller, &callback);
    oRank2 = std::make_unique<GOPipeConfigTreeNode>(
      oWindchest.get(), &controller, &callback);
    oRoot->Init(cfg, wxT("Root"), wxEmptyString);
    oWindchest->Init(cfg, wxT("Windchest"), wxEmptyString);
    oRank1->Init(cfg, wxT("Rank1"), wxEmptyString);
    oRank2->Init(cfg, wxT("Rank2"), wxEmptyString);
    for (unsigned rankI = 0; rankI < 2; rankI++) {
      GOPipeConfigNode *pRank = rankI ? oRank2.get() : oRank1.get();

      for (unsigned noteI = 0; noteI < 2; noteI++) {
        auto oPipe = std::make_unique<GOPipeConfigNode>(
          pRank, controller, &callback, nullptr);

        oPipe->SetMidiNoteNumber(36 + noteI);
        oPipe->Init(
          cfg, wxString::Format(wxT("Pipe%u"), rankI * 2 + noteI), wxT(""));
        mpPipes.push_back(oPipe.get());
        moPipes.push_back(std::move(oPipe));
      }
    }
  }
};

float effective_linear_gain(const GOPipeConfigNode &pipe) {
  return pipe.GetEffectiveAmplitude()
    * std::pow(10.0f, pipe.GetEffectiveGain() / 20.0f);
}
} // namespace

GOTestPipeVoicing::~GOTestPipeVoicing() {}

void GOTestPipeVoicing::run() {
  GO_RUN_TEST(TestOffsetsWriteThroughImmediately())
  GO_RUN_TEST(TestFirstPipeIsDisplayed())
  GO_RUN_TEST(TestResetRestoresSelectedValues())
  GO_RUN_TEST(TestAllAndRamp())
  GO_RUN_TEST(TestOffsetsAreClamped())
  GO_RUN_TEST(TestOffsetsWorkOnTopOfUpperLevels())
  GO_RUN_TEST(TestPipeIsSelectedOnce())
}

void GOTestPipeVoicing::TestOffsetsWriteThroughImmediately() {
  TestTree tree(*this->controller);
  GOPipeVoicing voicing;

  voicing.Select({tree.oRank1.get(), tree.oRank2.get()});

  this->GOAssert(
    voicing.GetNotes() == std::vector<int>({36, 37}),
    "The selected notes should be 36 and 37");
  this->GOAssert(
    voicing.GetOffset(GOPipeVoicing::GAIN, 36) == 0.0f,
    "An untouched pipe should read 0");

  const unsigned nAmplitudeBefore = tree.callback.nAmplitudeUpdates;

  voicing.SetOffset(GOPipeVoicing::GAIN, 36, 3.0f);

  this->GOAssert(
    tree.callback.nAmplitudeUpdates > nAmplitudeBefore,
    "The sound should be told about the gain change at once");
  this->GOAssert(
    tree.mpPipes[0]->GetPipeConfig().GetGain() == 3.0f
      && tree.mpPipes[2]->GetPipeConfig().GetGain() == 3.0f,
    "The pipes at the note in every selected rank should get the gain");
  this->GOAssert(
    tree.mpPipes[1]->GetPipeConfig().GetGain() == 0.0f,
    "The pipes at the other notes should not change");
  this->GOAssert(
    voicing.GetOffset(GOPipeVoicing::GAIN, 36) == 3.0f,
    "The offset should read back");

  const unsigned nTuningBefore = tree.callback.nTuningUpdates;
  const unsigned nToneBefore = tree.callback.nToneBalanceUpdates;

  voicing.SetOffset(GOPipeVoicing::MANUAL_TUNING, 37, -12.5f);
  voicing.SetOffset(GOPipeVoicing::TONE_BALANCE, 37, 40.0f);

  this->GOAssert(
    tree.callback.nTuningUpdates > nTuningBefore
      && tree.callback.nToneBalanceUpdates > nToneBefore,
    "The sound should be told about the tuning and tone changes at once");
  this->GOAssert(
    tree.mpPipes[1]->GetPipeConfig().GetManualTuning() == -12.5f
      && tree.mpPipes[3]->GetPipeConfig().GetToneBalanceValue() == 40,
    "The tuning and the tone balance should be written");
}

void GOTestPipeVoicing::TestFirstPipeIsDisplayed() {
  TestTree tree(*this->controller);
  GOPipeVoicing voicing;

  tree.mpPipes[0]->GetPipeConfig().SetGain(1.0f);
  tree.mpPipes[2]->GetPipeConfig().SetGain(5.0f);
  voicing.Select({tree.oRank1.get(), tree.oRank2.get()});

  this->GOAssert(
    voicing.GetOffset(GOPipeVoicing::GAIN, 36) == 1.0f,
    "The first rank should be displayed");
  this->GOAssert(
    voicing.GetOffset(GOPipeVoicing::GAIN, 99) == 0.0f,
    "A missing note should read 0");
}

void GOTestPipeVoicing::TestResetRestoresSelectedValues() {
  TestTree tree(*this->controller);
  GOPipeVoicing voicing;

  tree.mpPipes[0]->GetPipeConfig().SetGain(2.0f);
  tree.mpPipes[1]->GetPipeConfig().SetManualTuning(4.0f);
  voicing.Select({tree.oRank1.get()});

  this->GOAssert(!voicing.IsModified(), "Nothing is modified at Select()");

  voicing.SetAllOffsets(GOPipeVoicing::GAIN, -6.0f);
  voicing.SetOffset(GOPipeVoicing::MANUAL_TUNING, 37, 9.0f);
  voicing.SetOffset(GOPipeVoicing::TONE_BALANCE, 36, -30.0f);
  voicing.SetOffset(GOPipeVoicing::AUTO_TUNING_CORRECTION, 36, 2.0f);

  this->GOAssert(voicing.IsModified(), "The changes should be noticed");

  voicing.Reset();

  const GOPipeConfig &first = tree.mpPipes[0]->GetPipeConfig();
  const GOPipeConfig &second = tree.mpPipes[1]->GetPipeConfig();

  this->GOAssert(
    first.GetGain() == 2.0f && second.GetGain() == 0.0f
      && second.GetManualTuning() == 4.0f && first.GetToneBalanceValue() == 0
      && first.GetAutoTuningCorrection() == 0.0f,
    "Reset should restore the values at Select()");
  this->GOAssert(!voicing.IsModified(), "Nothing is modified after Reset");
}

void GOTestPipeVoicing::TestAllAndRamp() {
  TestTree tree(*this->controller);
  GOPipeVoicing voicing;

  voicing.Select({tree.oRank1.get()});
  voicing.SetAllOffsets(GOPipeVoicing::GAIN, 1.5f);

  this->GOAssert(
    voicing.GetOffset(GOPipeVoicing::GAIN, 36) == 1.5f
      && voicing.GetOffset(GOPipeVoicing::GAIN, 37) == 1.5f,
    "SetAllOffsets should set every pipe");

  voicing.SetRamp(GOPipeVoicing::GAIN, 36, 37, -2.0f, 4.0f);

  this->GOAssert(
    voicing.GetOffset(GOPipeVoicing::GAIN, 36) == -2.0f
      && voicing.GetOffset(GOPipeVoicing::GAIN, 37) == 4.0f,
    "SetRamp should put the end values at the ends of the range");
  voicing.SetRamp(GOPipeVoicing::GAIN, 36, 36, 8.0f, 9.0f);
  this->GOAssert(
    voicing.GetOffset(GOPipeVoicing::GAIN, 36) == 8.0f
      && voicing.GetOffset(GOPipeVoicing::GAIN, 37) == 4.0f,
    "A one-note ramp should set just that note");
}

void GOTestPipeVoicing::TestOffsetsAreClamped() {
  TestTree tree(*this->controller);
  GOPipeVoicing voicing;

  voicing.Select({tree.oRank1.get()});
  voicing.SetOffset(GOPipeVoicing::TONE_BALANCE, 36, 150.0f);
  voicing.SetOffset(GOPipeVoicing::GAIN, 36, 500.0f);
  voicing.SetOffset(GOPipeVoicing::MANUAL_TUNING, 36, 5000.0f);

  const GOPipeConfig &config = tree.mpPipes[0]->GetPipeConfig();

  this->GOAssert(
    config.GetToneBalanceValue() == 100, "Tone balance should stop at 100");
  this->GOAssert(config.GetGain() == 40.0f, "Gain should stop at 40 dB");
  this->GOAssert(
    config.GetManualTuning() == 1800.0f, "Tuning should stop at 1800 cent");
}

void GOTestPipeVoicing::TestOffsetsWorkOnTopOfUpperLevels() {
  TestTree tree(*this->controller);
  GOPipeVoicing voicing;

  tree.oRoot->GetPipeConfig().SetGain(3.0f);
  tree.oRoot->GetPipeConfig().SetManualTuning(7.0f);
  tree.oWindchest->GetPipeConfig().SetAmplitude(50.0f);
  tree.oRank1->GetPipeConfig().SetGain(-2.0f);
  tree.oRank1->GetPipeConfig().SetToneBalanceValue(20);

  const float levelBefore = effective_linear_gain(*tree.mpPipes[0]);

  voicing.Select({tree.oRank1.get()});

  this->GOAssert(
    voicing.GetOffset(GOPipeVoicing::GAIN, 36) == 0.0f
      && voicing.GetOffset(GOPipeVoicing::TONE_BALANCE, 36) == 0.0f,
    "A pipe that inherits everything should read 0");

  voicing.SetOffset(GOPipeVoicing::GAIN, 36, 6.0f);

  this->GOAssert(
    std::fabs(
      effective_linear_gain(*tree.mpPipes[0])
      - levelBefore * std::pow(10.0f, 6.0f / 20.0f))
      < 1e-5f * levelBefore,
    "The pipe offset should be added to the upper levels");
  this->GOAssert(
    tree.oRoot->GetPipeConfig().GetGain() == 3.0f
      && tree.oWindchest->GetPipeConfig().GetAmplitude() == 50.0f
      && tree.oRank1->GetPipeConfig().GetGain() == -2.0f
      && tree.oRank1->GetPipeConfig().GetToneBalanceValue() == 20
      && tree.oRoot->GetPipeConfig().GetManualTuning() == 7.0f,
    "The upper levels should not be touched by the voicing");
  this->GOAssert(
    tree.mpPipes[1]->GetEffectiveToneBalanceValue() == 20,
    "A pipe without an own tone balance should inherit the rank one");

  voicing.SetOffset(GOPipeVoicing::TONE_BALANCE, 37, -5.0f);

  this->GOAssert(
    tree.mpPipes[1]->GetEffectiveToneBalanceValue() == -5
      && tree.mpPipes[0]->GetEffectiveToneBalanceValue() == 20,
    "A non-zero pipe tone balance should override the inherited one");

  voicing.Reset();

  this->GOAssert(
    std::fabs(effective_linear_gain(*tree.mpPipes[0]) - levelBefore)
        < 1e-5f * levelBefore
      && tree.mpPipes[1]->GetEffectiveToneBalanceValue() == 20,
    "Reset should bring back the values inherited from the upper levels");
}

void GOTestPipeVoicing::TestPipeIsSelectedOnce() {
  TestTree tree(*this->controller);
  GOPipeVoicing voicing;

  voicing.Select({tree.oRoot.get(), tree.oRank1.get(), tree.oWindchest.get()});
  voicing.SetOffset(GOPipeVoicing::GAIN, 36, 2.0f);

  this->GOAssert(
    tree.mpPipes[0]->GetPipeConfig().GetGain() == 2.0f
      && tree.mpPipes[2]->GetPipeConfig().GetGain() == 2.0f,
    "All four pipes should be reachable from the root");

  voicing.Reset();

  this->GOAssert(
    voicing.GetNotes().size() == 2 && !voicing.IsModified(),
    "Overlapping selections should not duplicate notes");
}

std::string GOTestPipeVoicing::GetName() { return name; }
