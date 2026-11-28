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
  unsigned m_NAmplitudeUpdates = 0;
  unsigned m_NTuningUpdates = 0;
  unsigned m_NToneBalanceUpdates = 0;

  void UpdateAmplitude() override { m_NAmplitudeUpdates++; }
  void UpdateTuning() override { m_NTuningUpdates++; }
  void UpdateAudioGroup() override {}
  void UpdateReleaseTail() override {}
  void UpdateToneBalance() override { m_NToneBalanceUpdates++; }
};

/* root -> windchest -> rank1 (notes 36, 37), rank2 (notes 36, 37) */
class TestTree {
public:
  CountingCallback m_callback;
  std::unique_ptr<GOPipeConfigTreeNode> o_root;
  std::unique_ptr<GOPipeConfigTreeNode> o_windchest;
  std::unique_ptr<GOPipeConfigTreeNode> o_rank1;
  std::unique_ptr<GOPipeConfigTreeNode> o_rank2;
  std::vector<std::unique_ptr<GOPipeConfigNode>> mo_pipes;
  // mp_pipes[rank * 2 + noteI]
  std::vector<GOPipeConfigNode *> mp_pipes;

  TestTree(GOOrganController &controller) {
    GOConfigFileReader emptyFile;
    GOConfigReaderDB ini(controller.GetConfig().ODFCheck());

    ini.ReadData(emptyFile, CMBSetting, false);

    GOConfigReader cfg(
      ini,
      controller.GetConfig().ODFCheck(),
      controller.GetConfig().ODFHw1Check());

    o_root = std::make_unique<GOPipeConfigTreeNode>(
      nullptr, &controller, &m_callback);
    o_windchest = std::make_unique<GOPipeConfigTreeNode>(
      o_root.get(), &controller, &m_callback);
    o_rank1 = std::make_unique<GOPipeConfigTreeNode>(
      o_windchest.get(), &controller, &m_callback);
    o_rank2 = std::make_unique<GOPipeConfigTreeNode>(
      o_windchest.get(), &controller, &m_callback);
    o_root->Init(cfg, wxT("Root"), wxEmptyString);
    o_windchest->Init(cfg, wxT("Windchest"), wxEmptyString);
    o_rank1->Init(cfg, wxT("Rank1"), wxEmptyString);
    o_rank2->Init(cfg, wxT("Rank2"), wxEmptyString);
    for (unsigned rankI = 0; rankI < 2; rankI++) {
      GOPipeConfigNode *pRank = rankI ? o_rank2.get() : o_rank1.get();

      for (unsigned noteI = 0; noteI < 2; noteI++) {
        auto oPipe = std::make_unique<GOPipeConfigNode>(
          pRank, controller, &m_callback, nullptr, 36 + noteI);

        oPipe->Init(
          cfg, wxString::Format(wxT("Pipe%u"), rankI * 2 + noteI), wxT(""));
        mp_pipes.push_back(oPipe.get());
        mo_pipes.push_back(std::move(oPipe));
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
  GO_RUN_TEST(TestApplyWritesThroughImmediately())
  GO_RUN_TEST(TestUnchangedValuesAreNotApplied())
  GO_RUN_TEST(TestValueIsEmptyWhenPipesDiffer())
  GO_RUN_TEST(TestRevertRestoresSavedSettings())
  GO_RUN_TEST(TestAllAndRamp())
  GO_RUN_TEST(TestValuesAreClamped())
  GO_RUN_TEST(TestVoicingWorksOnTopOfUpperLevels())
  GO_RUN_TEST(TestPipeIsVoicedOnce())
}

void GOTestPipeVoicing::TestApplyWritesThroughImmediately() {
  TestTree tree(*this->controller);
  GOPipeVoicing voicing;

  this->GOAssert(!voicing.HasVoicedPipes(), "Nothing is voiced at first");

  voicing.SetVoicedNodes({tree.o_rank1.get(), tree.o_rank2.get()});

  this->GOAssert(voicing.HasVoicedPipes(), "The pipes should be voiced");
  this->GOAssert(
    voicing.GetVoicedMidiNotes() == std::vector<unsigned>({36, 37}),
    "The voiced notes should be 36 and 37");
  this->GOAssert(
    voicing.TakeSettingValueFromMidiNote(36, GOPipeVoicing::SETTING_GAIN)
      == tree.mp_pipes[0]->GetPipeConfig().GetGain(),
    "The value of an untouched pipe should be the stored one");
  this->GOAssert(
    voicing.TakeSettingValueFromMidiNote(
      36, GOPipeVoicing::SETTING_TONE_BALANCE)
      == 0.0f,
    "A pipe without an own tone balance should read 0");

  const unsigned nAmplitudeBefore = tree.m_callback.m_NAmplitudeUpdates;

  voicing.ApplySettingValueToMidiNote(36, GOPipeVoicing::SETTING_GAIN, 3.0f);

  this->GOAssert(
    tree.m_callback.m_NAmplitudeUpdates > nAmplitudeBefore,
    "The sound should be told about the gain change at once");
  this->GOAssert(
    tree.mp_pipes[0]->GetPipeConfig().GetGain() == 3.0f
      && tree.mp_pipes[2]->GetPipeConfig().GetGain() == 3.0f,
    "The pipes at the note in every voiced rank should get the gain");
  this->GOAssert(
    tree.mp_pipes[1]->GetPipeConfig().GetGain()
      == tree.mp_pipes[1]->GetPipeConfig().GetDefaultGain(),
    "The pipes at the other notes should not change");
  this->GOAssert(
    voicing.TakeSettingValueFromMidiNote(36, GOPipeVoicing::SETTING_GAIN)
      == 3.0f,
    "The value should read back as it is stored");

  const unsigned nTuningBefore = tree.m_callback.m_NTuningUpdates;
  const unsigned nToneBefore = tree.m_callback.m_NToneBalanceUpdates;

  voicing.ApplySettingValueToMidiNote(
    37, GOPipeVoicing::SETTING_MANUAL_TUNING, -12.5f);
  voicing.ApplySettingValueToMidiNote(
    37, GOPipeVoicing::SETTING_TONE_BALANCE, 40.0f);
  voicing.ApplySettingValueToMidiNote(
    37, GOPipeVoicing::SETTING_AUTO_TUNING_CORRECTION, 2.5f);

  this->GOAssert(
    tree.m_callback.m_NTuningUpdates > nTuningBefore
      && tree.m_callback.m_NToneBalanceUpdates > nToneBefore,
    "The sound should be told about the tuning and tone changes at once");
  this->GOAssert(
    tree.mp_pipes[1]->GetPipeConfig().GetManualTuning() == -12.5f
      && tree.mp_pipes[3]->GetPipeConfig().GetToneBalanceValue() == 40
      && tree.mp_pipes[1]->GetPipeConfig().GetAutoTuningCorrection() == 2.5f,
    "The tunings and the tone balance should be written");
}

void GOTestPipeVoicing::TestUnchangedValuesAreNotApplied() {
  TestTree tree(*this->controller);
  GOPipeVoicing voicing;

  voicing.SetVoicedNodes({tree.o_rank1.get()});

  const GOPipeConfig &config = tree.mp_pipes[0]->GetPipeConfig();
  const unsigned nAmplitudeBefore = tree.m_callback.m_NAmplitudeUpdates;
  const unsigned nTuningBefore = tree.m_callback.m_NTuningUpdates;
  const unsigned nToneBefore = tree.m_callback.m_NToneBalanceUpdates;

  // Applying the stored values must not notify anything
  voicing.ApplySettingValueToAllPipes(
    GOPipeVoicing::SETTING_GAIN, config.GetGain());
  voicing.ApplySettingValueToAllPipes(
    GOPipeVoicing::SETTING_MANUAL_TUNING, config.GetManualTuning());
  voicing.ApplySettingValueToAllPipes(
    GOPipeVoicing::SETTING_AUTO_TUNING_CORRECTION,
    config.GetAutoTuningCorrection());
  voicing.ApplySettingValueToAllPipes(
    GOPipeVoicing::SETTING_TONE_BALANCE, config.GetToneBalanceValue());

  this->GOAssert(
    tree.m_callback.m_NAmplitudeUpdates == nAmplitudeBefore
      && tree.m_callback.m_NTuningUpdates == nTuningBefore
      && tree.m_callback.m_NToneBalanceUpdates == nToneBefore,
    "Applying the values that are already set should notify nothing");

  voicing.ApplySettingValueToMidiNote(36, GOPipeVoicing::SETTING_GAIN, 2.0f);

  const unsigned nAmplitudeAfterChange = tree.m_callback.m_NAmplitudeUpdates;

  this->GOAssert(
    nAmplitudeAfterChange == nAmplitudeBefore + 1,
    "A changed value should notify once");

  voicing.ApplySettingValueToMidiNote(36, GOPipeVoicing::SETTING_GAIN, 2.0f);
  voicing.ApplySettingRampToMidiNotes(
    36, 36, GOPipeVoicing::SETTING_GAIN, 2.0f, 2.0f);

  this->GOAssert(
    tree.m_callback.m_NAmplitudeUpdates == nAmplitudeAfterChange,
    "Applying the same value again should notify nothing");

  // A value beyond the range is applied as the clamped one, so applying
  // another value beyond the range is not a change either
  voicing.ApplySettingValueToMidiNote(
    36, GOPipeVoicing::SETTING_MANUAL_TUNING, 5000.0f);

  const unsigned nTuningAfterChange = tree.m_callback.m_NTuningUpdates;

  voicing.ApplySettingValueToMidiNote(
    36, GOPipeVoicing::SETTING_MANUAL_TUNING, 6000.0f);

  this->GOAssert(
    nTuningAfterChange == nTuningBefore + 1
      && tree.m_callback.m_NTuningUpdates == nTuningAfterChange,
    "Two values beyond the range should be applied as one clamped value");
}

void GOTestPipeVoicing::TestValueIsEmptyWhenPipesDiffer() {
  TestTree tree(*this->controller);
  GOPipeVoicing voicing;

  tree.mp_pipes[0]->GetPipeConfig().SetGain(1.0f);
  tree.mp_pipes[2]->GetPipeConfig().SetGain(5.0f);
  tree.mp_pipes[1]->GetPipeConfig().SetGain(2.0f);
  tree.mp_pipes[3]->GetPipeConfig().SetGain(2.0f);
  voicing.SetVoicedNodes({tree.o_rank1.get(), tree.o_rank2.get()});

  this->GOAssert(
    !voicing.TakeSettingValueFromMidiNote(36, GOPipeVoicing::SETTING_GAIN)
       .has_value(),
    "The value should be empty if the pipes of the note have different ones");
  this->GOAssert(
    voicing.TakeSettingValueFromMidiNote(37, GOPipeVoicing::SETTING_GAIN)
      == 2.0f,
    "The value should be present if the pipes of the note have the same one");
  this->GOAssert(
    voicing
      .TakeSettingValueFromMidiNote(36, GOPipeVoicing::SETTING_TONE_BALANCE)
      .has_value(),
    "Another setting that is the same for the pipes should be present");

  voicing.ApplySettingValueToMidiNote(36, GOPipeVoicing::SETTING_GAIN, 3.0f);

  this->GOAssert(
    voicing.TakeSettingValueFromMidiNote(36, GOPipeVoicing::SETTING_GAIN)
      == 3.0f,
    "Applying one value should make the pipes of the note equal");
}

void GOTestPipeVoicing::TestRevertRestoresSavedSettings() {
  TestTree tree(*this->controller);
  GOPipeVoicing voicing;

  tree.mp_pipes[0]->GetPipeConfig().SetGain(2.0f);
  tree.mp_pipes[1]->GetPipeConfig().SetManualTuning(4.0f);
  voicing.SetVoicedNodes({tree.o_rank1.get()});

  this->GOAssert(
    !voicing.IsAnyVoicedPipeModified(),
    "Nothing is modified at SetVoicedNodes()");

  voicing.ApplySettingValueToAllPipes(GOPipeVoicing::SETTING_GAIN, -6.0f);
  voicing.ApplySettingValueToMidiNote(
    37, GOPipeVoicing::SETTING_MANUAL_TUNING, 9.0f);
  voicing.ApplySettingValueToMidiNote(
    36, GOPipeVoicing::SETTING_TONE_BALANCE, -30.0f);
  voicing.ApplySettingValueToMidiNote(
    36, GOPipeVoicing::SETTING_AUTO_TUNING_CORRECTION, 2.0f);

  this->GOAssert(
    voicing.IsAnyVoicedPipeModified(), "The changes should be noticed");

  const unsigned nToneBefore = tree.m_callback.m_NToneBalanceUpdates;
  const unsigned nTuningBefore = tree.m_callback.m_NTuningUpdates;

  // Only the pipe at 36 has a changed tone balance and auto-tuning correction,
  // the pipe at 37 has a changed manual tuning
  voicing.RevertVoicedPipeSettings();

  const GOPipeConfig &first = tree.mp_pipes[0]->GetPipeConfig();
  const GOPipeConfig &second = tree.mp_pipes[1]->GetPipeConfig();

  this->GOAssert(
    first.GetGain() == 2.0f && second.GetGain() == second.GetDefaultGain()
      && second.GetManualTuning() == 4.0f && first.GetToneBalanceValue() == 0
      && first.GetAutoTuningCorrection() == 0.0f,
    "Revert should restore the settings at SetVoicedNodes()");
  this->GOAssert(
    !voicing.IsAnyVoicedPipeModified(), "Nothing is modified after Revert");
  this->GOAssert(
    tree.m_callback.m_NToneBalanceUpdates == nToneBefore + 1
      && tree.m_callback.m_NTuningUpdates == nTuningBefore + 2,
    "Revert should apply only the differing settings");
}

void GOTestPipeVoicing::TestAllAndRamp() {
  TestTree tree(*this->controller);
  GOPipeVoicing voicing;

  voicing.SetVoicedNodes({tree.o_rank1.get()});
  voicing.ApplySettingValueToAllPipes(GOPipeVoicing::SETTING_GAIN, 1.5f);

  this->GOAssert(
    voicing.TakeSettingValueFromMidiNote(36, GOPipeVoicing::SETTING_GAIN)
        == 1.5f
      && voicing.TakeSettingValueFromMidiNote(37, GOPipeVoicing::SETTING_GAIN)
        == 1.5f,
    "ApplySettingValueToAllPipes should set every pipe");

  voicing.ApplySettingRampToMidiNotes(
    36, 37, GOPipeVoicing::SETTING_GAIN, -2.0f, 4.0f);

  this->GOAssert(
    voicing.TakeSettingValueFromMidiNote(36, GOPipeVoicing::SETTING_GAIN)
        == -2.0f
      && voicing.TakeSettingValueFromMidiNote(37, GOPipeVoicing::SETTING_GAIN)
        == 4.0f,
    "The ramp should put the end values at the ends of the range");

  voicing.ApplySettingRampToMidiNotes(
    36, 36, GOPipeVoicing::SETTING_GAIN, 8.0f, 9.0f);

  this->GOAssert(
    voicing.TakeSettingValueFromMidiNote(36, GOPipeVoicing::SETTING_GAIN)
        == 8.0f
      && voicing.TakeSettingValueFromMidiNote(37, GOPipeVoicing::SETTING_GAIN)
        == 4.0f,
    "A one-note ramp should set just that note");

  voicing.ApplySettingRampToMidiNotes(
    34, 38, GOPipeVoicing::SETTING_GAIN, 0.0f, 10.0f);

  this->GOAssert(
    voicing.TakeSettingValueFromMidiNote(36, GOPipeVoicing::SETTING_GAIN)
        == 5.0f
      && voicing.TakeSettingValueFromMidiNote(37, GOPipeVoicing::SETTING_GAIN)
        == 7.5f,
    "The ramp should skip the notes without voiced pipes and interpolate "
    "the others");
}

void GOTestPipeVoicing::TestValuesAreClamped() {
  TestTree tree(*this->controller);
  GOPipeVoicing voicing;

  voicing.SetVoicedNodes({tree.o_rank1.get()});
  voicing.ApplySettingValueToMidiNote(
    36, GOPipeVoicing::SETTING_TONE_BALANCE, 150.0f);
  voicing.ApplySettingValueToMidiNote(36, GOPipeVoicing::SETTING_GAIN, 500.0f);
  voicing.ApplySettingValueToMidiNote(
    36, GOPipeVoicing::SETTING_MANUAL_TUNING, 5000.0f);
  voicing.ApplySettingValueToMidiNote(
    36, GOPipeVoicing::SETTING_AUTO_TUNING_CORRECTION, -5000.0f);
  voicing.ApplySettingValueToMidiNote(37, GOPipeVoicing::SETTING_GAIN, -500.0f);

  const GOPipeConfig &config = tree.mp_pipes[0]->GetPipeConfig();

  this->GOAssert(
    config.GetToneBalanceValue() == 100, "Tone balance should stop at 100");
  this->GOAssert(config.GetGain() == 40.0f, "Gain should stop at 40 dB");
  this->GOAssert(
    config.GetManualTuning() == 1800.0f, "Tuning should stop at 1800 cent");
  this->GOAssert(
    config.GetAutoTuningCorrection() == -1800.0f,
    "Auto-tuning correction should stop at -1800 cent");
  this->GOAssert(
    tree.mp_pipes[1]->GetPipeConfig().GetGain() == -120.0f,
    "Gain should stop at -120 dB");
}

void GOTestPipeVoicing::TestVoicingWorksOnTopOfUpperLevels() {
  TestTree tree(*this->controller);
  GOPipeVoicing voicing;

  tree.o_root->GetPipeConfig().SetGain(3.0f);
  tree.o_root->GetPipeConfig().SetManualTuning(7.0f);
  tree.o_windchest->GetPipeConfig().SetAmplitude(50.0f);
  tree.o_rank1->GetPipeConfig().SetGain(-2.0f);
  tree.o_rank1->GetPipeConfig().SetToneBalanceValue(20);

  const float levelBefore = effective_linear_gain(*tree.mp_pipes[0]);

  voicing.SetVoicedNodes({tree.o_rank1.get()});

  this->GOAssert(
    voicing.TakeSettingValueFromMidiNote(
      36, GOPipeVoicing::SETTING_TONE_BALANCE)
      == 0.0f,
    "A pipe that inherits the tone balance should read 0");

  voicing.ApplySettingValueToMidiNote(
    36,
    GOPipeVoicing::SETTING_GAIN,
    tree.mp_pipes[0]->GetPipeConfig().GetGain() + 6.0f);

  this->GOAssert(
    std::fabs(
      effective_linear_gain(*tree.mp_pipes[0])
      - levelBefore * std::pow(10.0f, 6.0f / 20.0f))
      < 1e-5f * levelBefore,
    "The pipe gain should be added to the upper levels");
  this->GOAssert(
    tree.o_root->GetPipeConfig().GetGain() == 3.0f
      && tree.o_windchest->GetPipeConfig().GetAmplitude() == 50.0f
      && tree.o_rank1->GetPipeConfig().GetGain() == -2.0f
      && tree.o_rank1->GetPipeConfig().GetToneBalanceValue() == 20
      && tree.o_root->GetPipeConfig().GetManualTuning() == 7.0f,
    "The upper levels should not be touched by the voicing");
  this->GOAssert(
    tree.mp_pipes[1]->GetEffectiveToneBalanceValue() == 20,
    "A pipe without an own tone balance should inherit the rank one");

  voicing.ApplySettingValueToMidiNote(
    37, GOPipeVoicing::SETTING_TONE_BALANCE, -5.0f);

  this->GOAssert(
    tree.mp_pipes[1]->GetEffectiveToneBalanceValue() == -5
      && tree.mp_pipes[0]->GetEffectiveToneBalanceValue() == 20,
    "A non-zero pipe tone balance should replace the inherited one");

  voicing.RevertVoicedPipeSettings();

  this->GOAssert(
    std::fabs(effective_linear_gain(*tree.mp_pipes[0]) - levelBefore)
        < 1e-5f * levelBefore
      && tree.mp_pipes[1]->GetEffectiveToneBalanceValue() == 20,
    "Revert should bring back the values inherited from the upper levels");
}

void GOTestPipeVoicing::TestPipeIsVoicedOnce() {
  TestTree tree(*this->controller);
  GOPipeVoicing voicing;

  voicing.SetVoicedNodes(
    {tree.o_root.get(), tree.o_rank1.get(), tree.o_windchest.get()});

  const unsigned nAmplitudeBefore = tree.m_callback.m_NAmplitudeUpdates;

  voicing.ApplySettingValueToMidiNote(36, GOPipeVoicing::SETTING_GAIN, 2.0f);

  this->GOAssert(
    tree.mp_pipes[0]->GetPipeConfig().GetGain() == 2.0f
      && tree.mp_pipes[2]->GetPipeConfig().GetGain() == 2.0f,
    "All pipes at the note should be reachable from the root");

  voicing.RevertVoicedPipeSettings();

  this->GOAssert(
    voicing.GetVoicedMidiNotes().size() == 2
      && !voicing.IsAnyVoicedPipeModified(),
    "Overlapping subtrees should not duplicate notes");
  this->GOAssert(
    tree.m_callback.m_NAmplitudeUpdates == nAmplitudeBefore + 4,
    "The two pipes at the note should be changed and reverted exactly once");

  voicing.SetVoicedNodes({tree.o_rank2.get()});
  voicing.ApplySettingValueToMidiNote(36, GOPipeVoicing::SETTING_GAIN, 5.0f);

  const GOPipeConfig &rank1Config = tree.mp_pipes[0]->GetPipeConfig();

  this->GOAssert(
    tree.mp_pipes[2]->GetPipeConfig().GetGain() == 5.0f
      && rank1Config.GetGain() == rank1Config.GetDefaultGain(),
    "SetVoicedNodes should replace the voiced pipes");
}

std::string GOTestPipeVoicing::GetName() { return name; }
