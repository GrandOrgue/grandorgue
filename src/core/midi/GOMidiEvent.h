/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIEVENT_H
#define GOMIDIEVENT_H

#include <cstdint>
#include <vector>

#include "GOTime.h"

class GOMidiMap;

#define MIDI_CTRL_BANK_SELECT_MSB 0
#define MIDI_CTRL_BANK_SELECT_LSB 32
#define MIDI_CTRL_RPN_MSB 101
#define MIDI_CTRL_RPN_LSB 100
#define MIDI_CTRL_NRPN_MSB 99
#define MIDI_CTRL_NRPN_LSB 98
#define MIDI_CTRL_DATA_ENTRY 6
#define MIDI_CTRL_SOUNDS_OFF 120
#define MIDI_CTRL_NOTES_OFF 123

class GOMidiEvent {
public:
  enum MidiType {
    MIDI_NONE,
    MIDI_RESET,
    MIDI_NOTE,
    MIDI_AFTERTOUCH,
    MIDI_CTRL_CHANGE,
    MIDI_PGM_CHANGE,
    MIDI_RPN,
    MIDI_NRPN,
    MIDI_SYSEX_JOHANNUS_9,
    MIDI_SYSEX_JOHANNUS_11,
    MIDI_SYSEX_VISCOUNT,
    MIDI_SYSEX_AHLBORN_GALANTI,
    MIDI_SYSEX_GO_CLEAR,
    MIDI_SYSEX_GO_SETUP,
    MIDI_SYSEX_GO_SAMPLESET,
    MIDI_SYSEX_HW_STRING,
    MIDI_SYSEX_HW_LCD,
    MIDI_SYSEX_RODGERS_STOP_CHANGE,
  };

private:
  MidiType m_MidiType;
  int m_channel, m_key, m_value;
  unsigned m_device;
  GOTime m_time;
  wxString m_string;
  std::vector<uint8_t> m_data;
  bool m_IsToUseNoteOff;

public:
  GOMidiEvent();
  GOMidiEvent(const GOMidiEvent &e);

  MidiType GetMidiType() const { return m_MidiType; }
  void SetMidiType(MidiType t) { m_MidiType = t; }

  void SetDevice(unsigned dev) { m_device = dev; }

  unsigned GetDevice() const { return m_device; }

  int GetChannel() const { return m_channel; }
  void SetChannel(int ch) { m_channel = ch; }

  int GetKey() const { return m_key; }
  void SetKey(int k) { m_key = k; }

  int GetValue() const { return m_value; }
  void SetValue(int v) { m_value = v; }

  GOTime GetTime() const { return m_time; }
  void SetTime(GOTime t) { m_time = t; }

  const wxString &GetString() const { return m_string; }
  void SetString(const wxString &str) { m_string = str; }
  void SetString(const wxString &str, unsigned length);

  const std::vector<uint8_t> &GetData() const { return m_data; }
  void SetData(const std::vector<uint8_t> &data) { m_data = data; }

  void FromMidi(const std::vector<unsigned char> &msg, GOMidiMap &map);
  void ToMidi(
    std::vector<std::vector<unsigned char>> &msg, GOMidiMap &map) const;

  wxString ToString(GOMidiMap &map) const;

  void SetUseNoteOff(bool useNoteOff) { m_IsToUseNoteOff = useNoteOff; }
  bool IsUsingNoteOff() const { return m_IsToUseNoteOff; }
};

#endif
