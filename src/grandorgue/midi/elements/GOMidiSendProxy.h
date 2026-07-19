/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDISENDPROXY_H
#define GOMIDISENDPROXY_H

class GOMidiSystem;
class GOMidiEvent;
class GOMidiRecorder;

class GOMidiSendProxy {
  GOMidiSystem *p_midi = nullptr;
  GOMidiRecorder *p_MidiRecorder = nullptr;

  /** Whether object MIDI sends should reach external MIDI ports. */
  bool m_IsToSendMidi = true;
  /** Whether object MIDI events should reach the MIDI recorder. */
  bool m_IsToRecordMidi = true;

public:
  class StateRestorer {
    GOMidiSendProxy &r_MidiSendProxy;
    const bool m_IsToSendMidi;
    const bool m_IsToRecordMidi;

  public:
    /** Saves the current MIDI send and recording states. */
    StateRestorer(GOMidiSendProxy &midiSendProxy);
    /** Restores the saved MIDI send and recording states. */
    ~StateRestorer();
  };

  void SetMidi(GOMidiSystem *pMidi, GOMidiRecorder *pMidiRecorder);
  void SendMidiMessage(const GOMidiEvent &e);
  void SendMidiRecorderMessage(GOMidiEvent &e);

  /**
   * Returns whether object MIDI sends are forwarded to external MIDI ports.
   */
  bool IsToSendMidi() const { return m_IsToSendMidi; }

  /**
   * Enables or disables forwarding object MIDI sends to external MIDI ports.
   * @param isToSendMidi whether MIDI sends should reach GOMidiSystem
   */
  void SetToSendMidi(bool isToSendMidi) { m_IsToSendMidi = isToSendMidi; }

  /**
   * Returns whether object MIDI events are forwarded to the MIDI recorder.
   */
  bool IsToRecordMidi() const { return m_IsToRecordMidi; }

  /**
   * Enables or disables forwarding object MIDI events to the MIDI recorder.
   * @param isToRecordMidi whether MIDI events should reach GOMidiRecorder
   */
  void SetToRecordMidi(bool isToRecordMidi) {
    m_IsToRecordMidi = isToRecordMidi;
  }
};

#endif /* GOMIDISENDPROXY_H */
