/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundingPipe.h"

#include <cmath>

#include "model/GORank.h"
#include "model/GOSoundingPipe.h"
#include "temperaments/GOTemperamentCent.h"
#include "temperaments/GOTemperamentMtsEsp.h"

#include "GOTestScope.h"

GOTestSoundingPipe::~GOTestSoundingPipe() {}

void GOTestSoundingPipe::TestAudioGroup() {
  this->controller->GetConfig().SetAudioGroups({"Group A", "Group B"});

  GORank rank(*this->controller);
  GOSoundingPipe pipe(this->controller, &rank, 1, 60, 8, 100, 100, false);

  this->GOAssert(
    pipe.GetEffectiveAudioGroupId() == 0,
    "A pipe whose rank has no audio group set resolves to an empty audio "
    "group name, which GOConfig::GetAudioGroupId() falls back to id 0 for "
    "(same as an unmatched name), not because id 0 is 'the first group'");

  rank.GetPipeConfig().GetPipeConfig().SetAudioGroup("Group B");

  this->GOAssert(
    pipe.GetEffectiveAudioGroupId() == 1,
    "GetEffectiveAudioGroupId() should reflect the rank's new audio group "
    "immediately, with no need for PreparePlayback()/UpdateAudioGroup() to "
    "have run first");
}

void GOTestSoundingPipe::TestTemperamentByNoteNumber() {
  // a fake master: every note is retuned by its note number in cents
  const GOTemperamentMtsEsp temperament(
    [](unsigned noteNumber) { return noteNumber / 100.0; });
  GORank rank(*this->controller);
  GOSoundingPipe pipe61(this->controller, &rank, 1, 61, 8, 100, 100, true);
  GOSoundingPipe pipe73(this->controller, &rank, 1, 73, 8, 100, 100, true);
  GOSoundingPipe pipeFixed(this->controller, &rank, 1, 73, 8, 100, 100, false);

  // as a rank does: through GOPipe, where SetTemperament() is public
  for (GOPipe *pPipe :
       {(GOPipe *)&pipe61, (GOPipe *)&pipe73, (GOPipe *)&pipeFixed})
    pPipe->SetTemperament(temperament);
  this->GOAssert(
    std::abs(pipe61.GetTemperamentOffset() - 61) < 1e-3,
    "The pipe of note 61 should get the offset of note 61");
  this->GOAssert(
    std::abs(pipe73.GetTemperamentOffset() - 73) < 1e-3,
    "The pipe of note 73 should get the offset of note 73, not of its pitch "
    "class");
  this->GOAssert(
    pipeFixed.GetTemperamentOffset() == 0,
    "A pipe that is not retuned should get no temperament offset");
}

void GOTestSoundingPipe::TestAbsolutePitchTemperament() {
  // a fake master in the equal temperament
  const GOTemperamentMtsEsp master([](unsigned noteNumber) { return 0.0; });
  const GOTemperamentCent equal(
    wxT("Equal"), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  GOPipeConfigNode &organNode = this->controller->GetRootPipeConfigNode();
  GORank rank(*this->controller);

  // the organ-wide correction sets the pitch of the whole organ, the rank one
  // voices the rank
  rank.GetPipeConfig().SetParent(&organNode);
  organNode.GetPipeConfig().SetAutoTuningCorrection(50);
  rank.GetPipeConfig().GetPipeConfig().SetAutoTuningCorrection(15);

  GOSoundingPipe pipe(this->controller, &rank, 1, 69, 8, 100, 100, true);
  GOPipe &asPipe = pipe;
  const auto fGetCents
    = [&pipe]() { return 1200 * std::log2(pipe.GetTuning()); };

  asPipe.SetTemperament(equal);
  this->GOAssert(
    std::abs(fGetCents() - 65) < 1e-2,
    "In the equal temperament both the organ-wide and the rank corrections "
    "should apply");
  asPipe.SetTemperament(master);
  this->GOAssert(
    std::abs(fGetCents() - 15) < 1e-2,
    "Under a temperament of absolute pitch only the rank correction should "
    "apply, so the pipe sounds at the master's pitch");
  asPipe.SetTemperament(equal);
  this->GOAssert(
    std::abs(fGetCents() - 65) < 1e-2,
    "Leaving the temperament of absolute pitch should restore the organ-wide "
    "correction");
  organNode.GetPipeConfig().SetAutoTuningCorrection(0);
}

void GOTestSoundingPipe::run(){GO_RUN_TEST(TestAudioGroup())
                                 GO_RUN_TEST(TestTemperamentByNoteNumber())
                                   GO_RUN_TEST(TestAbsolutePitchTemperament())}

std::string GOTestSoundingPipe::GetName() {
  return name;
}
