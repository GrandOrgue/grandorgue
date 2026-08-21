/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDCALLBACKCONNECTOR_H
#define GOSOUNDCALLBACKCONNECTOR_H

#include <atomic>

#include "threading/GOCondition.h"
#include "threading/GOMutex.h"

class GOSoundBufferMutable;
class GOSoundOrganEngine;

/**
 * Base class that manages the connection between audio ports and a
 * GOSoundOrganEngine. Responsible for routing real-time audio callbacks
 * to the currently connected engine and synchronizing engine
 * connect/disconnect with the audio callback thread.
 *
 * GOSoundSystem inherits from this class and adds audio hardware
 * management (ports, MIDI, recording).
 */
class GOSoundCallbackConnector {
private:
  std::atomic<GOSoundOrganEngine *> p_OrganEngine{nullptr};

  // counter of audio callbacks that have been entered but have not yet been
  // exited
  std::atomic_uint m_NCallbacksEntered{0};

  // For waiting for and notifying when m_NCallbacksEntered becomes 0
  GOMutex m_CallbackMutex;
  GOCondition m_CallbackCondition{m_CallbackMutex};

  unsigned m_SamplesPerBuffer = 0;
  unsigned m_SampleRate = 0;

  /*
   * Virtual hooks
   */

  /**
   * Called by ConnectToEngine() before the engine pointer is stored.
   * Override to add subclass-specific precondition checks.
   */
  virtual void OnBeforeConnectToEngine() {}

  /**
   * Called by DisconnectFromEngine() after all callbacks have exited
   * and the engine has been fully disconnected.
   * Override to perform cleanup in subclasses.
   */
  virtual void OnAfterDisconnectFromEngine() {}

  /**
   * Called by AudioCallback() each time a new audio period begins
   * (i.e. when ProcessAudioCallback returns true for the last output).
   * Override to update meters or perform other per-period tasks.
   */
  virtual void OnNewAudioPeriod() {}

public:
  /** Returns the number of audio frames per buffer used by the connected ports.
   */
  unsigned GetSamplesPerBuffer() const { return m_SamplesPerBuffer; }
  void SetSamplesPerBuffer(unsigned nSamplesPerBuffer) {
    m_SamplesPerBuffer = nSamplesPerBuffer;
  }

  /** Returns the audio sample rate in Hz used by the connected ports. */
  unsigned GetSampleRate() const { return m_SampleRate; }
  void SetSampleRate(unsigned sampleRate) { m_SampleRate = sampleRate; }

  /** Returns true if an engine is currently connected. */
  bool IsEngineConnected() const { return p_OrganEngine.load(); }

  /*
   * Engine connection
   */

  /**
   * Connects the given engine to receive audio callbacks.
   * The engine must be in the WORKING state (built but not yet used).
   * Calls OnBeforeConnectToEngine() to allow subclasses to validate
   * preconditions before the connection is established.
   */
  void ConnectToEngine(GOSoundOrganEngine &engine);

  /**
   * Disconnects the engine from audio callbacks and waits for all
   * in-flight callbacks to finish before returning.
   * Calls OnAfterDisconnectFromEngine() once all callbacks have exited.
   */
  void DisconnectFromEngine(GOSoundOrganEngine &engine);

  /*
   * Audio callback
   */

  /**
   * Routes an audio callback from a sound port to the connected engine.
   * If no engine is connected, fills the output buffer with silence.
   * Thread-safe: may be called concurrently from multiple audio port
   * callback threads.
   * @return always true (keep the audio stream running)
   */
  bool AudioCallback(unsigned devIndex, GOSoundBufferMutable &outBuffer);
};

#endif
