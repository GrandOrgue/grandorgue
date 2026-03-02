/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundCallbackConnector.h"

#include <wx/intl.h>
#include <wx/log.h>

#include "buffer/GOSoundBufferMutable.h"
#include "threading/GOMutexLocker.h"

#include "GOSoundOrganEngine.h"

void GOSoundCallbackConnector::ConnectToEngine(GOSoundOrganEngine &engine) {
  OnBeforeConnectToEngine();
  assert(engine.IsWorking());

  engine.SetUsed(true);
  m_NCallbacksEntered.store(0);
  engine.SetStreaming(true);
  p_OrganEngine.store(&engine);
}

void GOSoundCallbackConnector::DisconnectFromEngine(
  GOSoundOrganEngine &engine) {
  // Signal callbacks to stop by clearing the engine pointer
  p_OrganEngine.store(nullptr);

  // Unblock any callbacks waiting at [W1] and prevent new ones from blocking
  engine.SetStreaming(false);

  // wait for all started callbacks to finish
  {
    GOMutexLocker lock(m_CallbackMutex);

    while (m_NCallbacksEntered.load() > 0)
      m_CallbackCondition.WaitOrStop(
        "GOSoundCallbackConnector::DisconnectFromEngine waits for all "
        "callbacks to finish",
        nullptr);
  }

  engine.SetUsed(false);
  OnAfterDisconnectFromEngine();
}

bool GOSoundCallbackConnector::AudioCallback(
  unsigned devIndex, GOSoundBufferMutable &outBuffer) {
  bool wasEntered = false;
  const unsigned nSamples = outBuffer.GetNFrames();

  if (p_OrganEngine.load()) {
    if (nSamples == m_SamplesPerBuffer) {
      m_NCallbacksEntered.fetch_add(1);
      wasEntered = true;
    } else
      wxLogError(
        _("No sound output will happen. Samples per buffer has been "
          "changed by the sound driver to %d"),
        nSamples);
  }
  // assure that p_OrganEngine has not yet been changed after
  // m_NCallbacksEntered.fetch_add, otherwise the control thread may not wait
  GOSoundOrganEngine *pOrganEngine
    = wasEntered ? p_OrganEngine.load() : nullptr;

  if (pOrganEngine) {
    if (pOrganEngine->ProcessAudioCallback(devIndex, outBuffer))
      OnNewAudioPeriod();
  } else
    outBuffer.FillWithSilence();
  if (
    wasEntered && m_NCallbacksEntered.fetch_sub(1) <= 1
    && !p_OrganEngine.load()) {
    // ensure that the control thread enters into m_NCallbackCondition.Wait()
    GOMutexLocker lk(m_CallbackMutex);

    // notify the control thread
    m_CallbackCondition.Broadcast();
  }
  return true;
}
