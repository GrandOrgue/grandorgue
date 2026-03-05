/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDORGANENGINEBASE_H
#define GOTESTSOUNDORGANENGINEBASE_H

#include "GOTest.h"

#include "sound/interfaces/GOSoundCallbackConnector.h"

class GOSoundOrganEngine;

/*
 * Base class for GOSoundOrganEngine tests.
 *
 * Provides helpers for building and stopping the engine.
 */
class GOTestSoundOrganEngineBase : public GOCommonControllerTest {
public:
  /*
   * Number of channels in each output buffer.
   */
  static constexpr unsigned N_OUTPUT_CHANNELS = 2;

  static constexpr unsigned N_SAMPLES_PER_BUFFER = 128;
  static constexpr unsigned SAMPLE_RATE = 96000;

protected:
  GOSoundCallbackConnector m_connector;

  GOTestSoundOrganEngineBase();

  /*
   * Configures and starts the engine with the given parameters.
   * Asserts that the engine is WORKING and not USED after start.
   * Returns a reference to the started engine.
   */
  GOSoundOrganEngine &BuildAndStartEngine(
    unsigned nAudioGroups, unsigned nAuxThreads, unsigned nOutputs);

  /*
   * Stops and destroys the engine.
   * Asserts that the engine is IDLE after stop.
   */
  void StopAndDestroyEngine();
};

#endif /* GOTESTSOUNDORGANENGINEBASE_H */
