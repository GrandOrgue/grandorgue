/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDORGANENGINEFACTORIES_H
#define GOTESTSOUNDORGANENGINEFACTORIES_H

#include <string>

#include "GOTest.h"

/**
 * Exercises GOSoundOrganEngine's static factory functions - the parts of
 * BuildEngine() that can be tested without constructing a full engine.
 */
class GOTestSoundOrganEngineFactories : public GOTest {
private:
  static const std::string TEST_NAME;

  /** With one audio group, createDownmixGains() must keep the pre-fix
   * behaviour: L = group0.left, R = group0.right (0.0f dB at those
   * positions, GOAudioDeviceConfig::MUTE_VOLUME everywhere else). */
  void TestDownmixGainsWithOneGroup();

  /** Regression test for the stereo-downmix scale-factor bug: with two or
   * more audio groups, the gains layout createDownmixGains() returns - once
   * converted to linear via convertGainToScaleFactor() and run through a
   * real GOSoundOutputTask - must match the row-major
   * [outChannelI][groupI * 2 + groupChannelI] indexing
   * GOSoundOutputTask::DoRun() actually reads (row stride nAudioGroups * 2),
   * not a fixed stride of 4 per group - otherwise the downmix's left and
   * right channels end up reading each other's groups' channels instead of
   * summing every group's own left/right channel. */
  void TestDownmixGainsWithTwoGroups();

  /** createDefaultOutputConfigs() must reflect its documented per-group
   * layout: scaleFactors[0][i*2] = 0.0f (L), scaleFactors[1][i*2+1] = 0.0f
   * (R), MUTE_VOLUME everywhere else - for both one and two audio groups. */
  void TestDefaultOutputConfigs();

  /** createDefaultOutputConfigs() must reuse the same group-selection
   * pattern as createDownmixGains(): its two per-channel scaleFactors
   * vectors, concatenated, must equal createDownmixGains()'s flat result -
   * pins the two factory functions together so they cannot silently drift
   * apart the way the pre-fix downmix code once did. */
  void TestDefaultOutputConfigsMatchesDownmixGains();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDORGANENGINEFACTORIES_H */
