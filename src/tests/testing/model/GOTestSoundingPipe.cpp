/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundingPipe.h"

#include "model/GORank.h"
#include "model/GOSoundingPipe.h"

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

void GOTestSoundingPipe::run() { TestAudioGroup(); }

std::string GOTestSoundingPipe::GetName() { return name; }
