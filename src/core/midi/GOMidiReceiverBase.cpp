/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiReceiverBase.h"

#include "GOMidiEvent.h"
#include "GOMidiMap.h"
#include "GORodgers.h"
#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"

GOMidiReceiverBase::GOMidiReceiverBase(GOMidiReceiverType type)
  : GOMidiReceiverEventPatternList(type),
    m_ElementID(-1),
    m_last(),
    m_Internal() {}

void GOMidiReceiverBase::SetElementID(int id) { m_ElementID = id; }

const struct IniFileEnumEntry GOMidiReceiverBase::m_MidiTypes[] = {
  {wxT("ControlChange"), MIDI_M_CTRL_CHANGE},
  {wxT("Note"), MIDI_M_NOTE},
  {wxT("ProgramChange"), MIDI_M_PGM_CHANGE},
  {wxT("ProgramRange"), MIDI_M_PGM_RANGE},
  {wxT("SysExJohannus"), MIDI_M_SYSEX_JOHANNUS_9},
  {wxT("SysExJohannus11"), MIDI_M_SYSEX_JOHANNUS_11},
  {wxT("SysExViscount"), MIDI_M_SYSEX_VISCOUNT},
  {wxT("SysExViscountToggle"), MIDI_M_SYSEX_VISCOUNT_TOGGLE},
  {wxT("SysExRodgersStopChange"), MIDI_M_SYSEX_RODGERS_STOP_CHANGE},
  {wxT("SysExAhlbornGalanti"), MIDI_M_SYSEX_AHLBORN_GALANTI},
  {wxT("SysExAhlbornGalantiToggle"), MIDI_M_SYSEX_AHLBORN_GALANTI_TOGGLE},
  {wxT("RPN"), MIDI_M_RPN},
  {wxT("NRPN"), MIDI_M_NRPN},
  {wxT("RPNRange"), MIDI_M_RPN_RANGE},
  {wxT("NRPNRange"), MIDI_M_NRPN_RANGE},
  {wxT("RPNOn"), MIDI_M_RPN_ON},
  {wxT("RPNOff"), MIDI_M_RPN_OFF},
  {wxT("RPNOnOff"), MIDI_M_RPN_ON_OFF},
  {wxT("NRPNOn"), MIDI_M_NRPN_ON},
  {wxT("NRPNOff"), MIDI_M_NRPN_OFF},
  {wxT("NRPNOnOff"), MIDI_M_NRPN_ON_OFF},
  {wxT("ControlChangeBitfield"), MIDI_M_CTRL_BIT},
  {wxT("ControlChangeOn"), MIDI_M_CTRL_CHANGE_ON},
  {wxT("ControlChangeOff"), MIDI_M_CTRL_CHANGE_OFF},
  {wxT("ControlChangeOnOff"), MIDI_M_CTRL_CHANGE_ON_OFF},
  {wxT("ControlChangeFixed"), MIDI_M_CTRL_CHANGE_FIXED},
  {wxT("ControlChangeFixedOn"), MIDI_M_CTRL_CHANGE_FIXED_ON},
  {wxT("ControlChangeFixedOff"), MIDI_M_CTRL_CHANGE_FIXED_OFF},
  {wxT("ControlChangeFixedOnOff"), MIDI_M_CTRL_CHANGE_FIXED_ON_OFF},
  {wxT("NoteOn"), MIDI_M_NOTE_ON},
  {wxT("NoteOff"), MIDI_M_NOTE_OFF},
  {wxT("NoteOnOff"), MIDI_M_NOTE_ON_OFF},
  {wxT("NoteNoVelocity"), MIDI_M_NOTE_NO_VELOCITY},
  {wxT("NoteShortOctave"), MIDI_M_NOTE_SHORT_OCTAVE},
  {wxT("NoteNormal"), MIDI_M_NOTE_NORMAL},
};

void GOMidiReceiverBase::Load(
  GOConfigReader &cfg, wxString group, GOMidiMap &map) {
  m_events.resize(0);

  int event_cnt = cfg.ReadInteger(
    CMBSetting, group, wxT("NumberOfMIDIEvents"), -1, 255, false);
  if (event_cnt >= 0) {
    m_events.resize(event_cnt);
    for (unsigned i = 0; i < m_events.size(); i++) {
      m_events[i].deviceId = map.GetDeviceIdByLogicalName(cfg.ReadString(
        CMBSetting,
        group,
        wxString::Format(wxT("MIDIDevice%03d"), i + 1),
        false));
      GOMidiReceiverMessageType default_type = MIDI_M_PGM_CHANGE;
      if (m_type == MIDI_RECV_MANUAL)
        default_type = MIDI_M_NOTE;
      if (m_type == MIDI_RECV_ENCLOSURE)
        default_type = MIDI_M_CTRL_CHANGE;
      m_events[i].type = (GOMidiReceiverMessageType)cfg.ReadEnum(
        CMBSetting,
        group,
        wxString::Format(wxT("MIDIEventType%03d"), i + 1),
        m_MidiTypes,
        sizeof(m_MidiTypes) / sizeof(m_MidiTypes[0]),
        false,
        default_type);
      if (HasChannel(m_events[i].type))
        m_events[i].channel = cfg.ReadInteger(
          CMBSetting,
          group,
          wxString::Format(wxT("MIDIChannel%03d"), i + 1),
          -1,
          16);
      else
        m_events[i].channel = -1;
      if (HasDebounce(m_events[i].type))
        m_events[i].debounce_time = cfg.ReadInteger(
          CMBSetting,
          group,
          wxString::Format(wxT("MIDIDebounce%03d"), i + 1),
          0,
          3000,
          false,
          0);
      if (HasLowKey(m_events[i].type))
        m_events[i].low_key = cfg.ReadInteger(
          CMBSetting,
          group,
          wxString::Format(wxT("MIDILowerKey%03d"), i + 1),
          0,
          127,
          false,
          0);
      if (HasHighKey(m_events[i].type))
        m_events[i].high_key = cfg.ReadInteger(
          CMBSetting,
          group,
          wxString::Format(wxT("MIDIUpperKey%03d"), i + 1),
          0,
          127,
          false,
          127);

      if (m_type == MIDI_RECV_MANUAL)
        m_events[i].key = cfg.ReadInteger(
          CMBSetting,
          group,
          wxString::Format(wxT("MIDIKeyShift%03d"), i + 1),
          -35,
          35);
      else if (HasKey(m_events[i].type))
        m_events[i].key = cfg.ReadInteger(
          CMBSetting,
          group,
          wxString::Format(wxT("MIDIKey%03d"), i + 1),
          0,
          0x200000);

      if (HasLowerLimit(m_events[i].type))
        m_events[i].low_value = cfg.ReadInteger(
          CMBSetting,
          group,
          wxString::Format(wxT("MIDILowerLimit%03d"), i + 1),
          0,
          0x200000,
          false,
          cfg.ReadInteger(
            CMBSetting,
            group,
            wxString::Format(wxT("MIDILowerVelocity%03d"), i + 1),
            0,
            127,
            false,
            1));

      if (HasUpperLimit(m_events[i].type))
        m_events[i].high_value = cfg.ReadInteger(
          CMBSetting,
          group,
          wxString::Format(wxT("MIDIUpperLimit%03d"), i + 1),
          0,
          0x200000,
          false,
          cfg.ReadInteger(
            CMBSetting,
            group,
            wxString::Format(wxT("MIDIUpperVelocity%03d"), i + 1),
            0,
            127,
            false,
            127));
    }
  } else {
    m_events.resize(0);
    Preconfigure(cfg, group);
  }
}

void GOMidiReceiverBase::Preconfigure(GOConfigReader &cfg, wxString group) {}

int GOMidiReceiverBase::GetTranspose() { return 0; }

void GOMidiReceiverBase::Save(
  GOConfigWriter &cfg, wxString group, GOMidiMap &map) {
  cfg.WriteInteger(group, wxT("NumberOfMIDIEvents"), m_events.size());
  for (unsigned i = 0; i < m_events.size(); i++) {
    cfg.WriteString(
      group,
      wxString::Format(wxT("MIDIDevice%03d"), i + 1),
      map.GetDeviceLogicalNameById(m_events[i].deviceId));
    cfg.WriteEnum(
      group,
      wxString::Format(wxT("MIDIEventType%03d"), i + 1),
      m_events[i].type,
      m_MidiTypes,
      sizeof(m_MidiTypes) / sizeof(m_MidiTypes[0]));
    if (HasChannel(m_events[i].type))
      cfg.WriteInteger(
        group,
        wxString::Format(wxT("MIDIChannel%03d"), i + 1),
        m_events[i].channel);
    if (HasDebounce(m_events[i].type))
      cfg.WriteInteger(
        group,
        wxString::Format(wxT("MIDIDebounce%03d"), i + 1),
        m_events[i].debounce_time);

    if (HasLowKey(m_events[i].type))
      cfg.WriteInteger(
        group,
        wxString::Format(wxT("MIDILowerKey%03d"), i + 1),
        m_events[i].low_key);
    if (HasHighKey(m_events[i].type))
      cfg.WriteInteger(
        group,
        wxString::Format(wxT("MIDIUpperKey%03d"), i + 1),
        m_events[i].high_key);

    if (m_type == MIDI_RECV_MANUAL)
      cfg.WriteInteger(
        group,
        wxString::Format(wxT("MIDIKeyShift%03d"), i + 1),
        m_events[i].key);
    else if (HasKey(m_events[i].type))
      cfg.WriteInteger(
        group, wxString::Format(wxT("MIDIKey%03d"), i + 1), m_events[i].key);

    if (HasLowerLimit(m_events[i].type))
      cfg.WriteInteger(
        group,
        wxString::Format(wxT("MIDILowerLimit%03d"), i + 1),
        m_events[i].low_value);
    if (HasUpperLimit(m_events[i].type))
      cfg.WriteInteger(
        group,
        wxString::Format(wxT("MIDIUpperLimit%03d"), i + 1),
        m_events[i].high_value);
  }
}

bool GOMidiReceiverBase::HasChannel(GOMidiReceiverMessageType type) {
  if (
    type == MIDI_M_NOTE || type == MIDI_M_CTRL_CHANGE
    || type == MIDI_M_PGM_CHANGE || type == MIDI_M_PGM_RANGE
    || type == MIDI_M_RPN_RANGE || type == MIDI_M_NRPN_RANGE
    || type == MIDI_M_CTRL_BIT || type == MIDI_M_CTRL_CHANGE_FIXED
    || type == MIDI_M_RPN || type == MIDI_M_NRPN || type == MIDI_M_NOTE_ON
    || type == MIDI_M_NOTE_OFF || type == MIDI_M_NOTE_ON_OFF
    || type == MIDI_M_CTRL_CHANGE_ON || type == MIDI_M_CTRL_CHANGE_OFF
    || type == MIDI_M_CTRL_CHANGE_ON_OFF || type == MIDI_M_CTRL_CHANGE_FIXED_ON
    || type == MIDI_M_CTRL_CHANGE_FIXED_OFF
    || type == MIDI_M_CTRL_CHANGE_FIXED_ON_OFF || type == MIDI_M_RPN_ON
    || type == MIDI_M_RPN_OFF || type == MIDI_M_RPN_ON_OFF
    || type == MIDI_M_NRPN_ON || type == MIDI_M_NRPN_OFF
    || type == MIDI_M_NRPN_ON_OFF || type == MIDI_M_NOTE_NO_VELOCITY
    || type == MIDI_M_NOTE_NORMAL || type == MIDI_M_NOTE_SHORT_OCTAVE)
    return true;
  return false;
}

bool GOMidiReceiverBase::HasKey(GOMidiReceiverMessageType type) {
  if (
    type == MIDI_M_NOTE || type == MIDI_M_CTRL_CHANGE
    || type == MIDI_M_PGM_CHANGE || type == MIDI_M_RPN_RANGE
    || type == MIDI_M_NRPN_RANGE || type == MIDI_M_SYSEX_JOHANNUS_9
    || type == MIDI_M_SYSEX_JOHANNUS_11
    || type == MIDI_M_SYSEX_RODGERS_STOP_CHANGE || type == MIDI_M_CTRL_BIT
    || type == MIDI_M_CTRL_CHANGE_FIXED || type == MIDI_M_RPN
    || type == MIDI_M_NRPN || type == MIDI_M_NOTE_ON || type == MIDI_M_NOTE_OFF
    || type == MIDI_M_NOTE_ON_OFF || type == MIDI_M_CTRL_CHANGE_ON
    || type == MIDI_M_CTRL_CHANGE_OFF || type == MIDI_M_CTRL_CHANGE_ON_OFF
    || type == MIDI_M_CTRL_CHANGE_FIXED_ON
    || type == MIDI_M_CTRL_CHANGE_FIXED_OFF
    || type == MIDI_M_CTRL_CHANGE_FIXED_ON_OFF || type == MIDI_M_RPN_ON
    || type == MIDI_M_RPN_OFF || type == MIDI_M_RPN_ON_OFF
    || type == MIDI_M_NRPN_ON || type == MIDI_M_NRPN_OFF
    || type == MIDI_M_NRPN_ON_OFF || type == MIDI_M_NOTE_NO_VELOCITY
    || type == MIDI_M_NOTE_NORMAL || type == MIDI_M_NOTE_SHORT_OCTAVE)
    return true;
  return false;
}

bool GOMidiReceiverBase::HasLowKey(GOMidiReceiverMessageType type) {
  if (m_type != MIDI_RECV_MANUAL)
    return false;
  if (
    type == MIDI_M_NOTE || type == MIDI_M_NOTE_NO_VELOCITY
    || type == MIDI_M_NOTE_NORMAL || type == MIDI_M_NOTE_SHORT_OCTAVE)
    return true;
  return false;
}

bool GOMidiReceiverBase::HasHighKey(GOMidiReceiverMessageType type) {
  if (m_type != MIDI_RECV_MANUAL)
    return false;
  if (
    type == MIDI_M_NOTE || type == MIDI_M_NOTE_NO_VELOCITY
    || type == MIDI_M_NOTE_NORMAL || type == MIDI_M_NOTE_SHORT_OCTAVE)
    return true;
  return false;
}

bool GOMidiReceiverBase::HasDebounce(GOMidiReceiverMessageType type) {
  if (m_type == MIDI_RECV_MANUAL)
    return false;
  if (m_type == MIDI_RECV_ENCLOSURE)
    return false;
  if (
    type == MIDI_M_PGM_CHANGE || type == MIDI_M_NOTE_ON
    || type == MIDI_M_NOTE_OFF || type == MIDI_M_NOTE_ON_OFF
    || type == MIDI_M_CTRL_CHANGE || type == MIDI_M_CTRL_CHANGE_ON
    || type == MIDI_M_CTRL_CHANGE_OFF || type == MIDI_M_CTRL_CHANGE_ON_OFF
    || type == MIDI_M_CTRL_CHANGE_FIXED_ON
    || type == MIDI_M_CTRL_CHANGE_FIXED_OFF
    || type == MIDI_M_CTRL_CHANGE_FIXED_ON_OFF || type == MIDI_M_RPN_ON
    || type == MIDI_M_RPN_OFF || type == MIDI_M_RPN_ON_OFF
    || type == MIDI_M_NRPN_ON || type == MIDI_M_NRPN_OFF
    || type == MIDI_M_NRPN_ON_OFF || type == MIDI_M_SYSEX_VISCOUNT_TOGGLE
    || type == MIDI_M_SYSEX_JOHANNUS_9 || type == MIDI_M_SYSEX_JOHANNUS_11)
    return true;
  return false;
}

bool GOMidiReceiverBase::HasLowerLimit(GOMidiReceiverMessageType type) {
  if (
    type == MIDI_M_NOTE || type == MIDI_M_PGM_RANGE || type == MIDI_M_RPN_RANGE
    || type == MIDI_M_NRPN_RANGE || type == MIDI_M_CTRL_CHANGE
    || type == MIDI_M_CTRL_CHANGE_FIXED || type == MIDI_M_CTRL_BIT
    || type == MIDI_M_RPN || type == MIDI_M_NRPN || type == MIDI_M_NOTE_OFF
    || type == MIDI_M_NOTE_ON_OFF || type == MIDI_M_CTRL_CHANGE_OFF
    || type == MIDI_M_CTRL_CHANGE_ON_OFF || type == MIDI_M_CTRL_CHANGE_FIXED_OFF
    || type == MIDI_M_CTRL_CHANGE_FIXED_ON_OFF || type == MIDI_M_RPN_OFF
    || type == MIDI_M_RPN_ON_OFF || type == MIDI_M_NRPN_OFF
    || type == MIDI_M_NRPN_ON_OFF || type == MIDI_M_NOTE_NO_VELOCITY
    || type == MIDI_M_NOTE_NORMAL || type == MIDI_M_SYSEX_VISCOUNT
    || type == MIDI_M_SYSEX_VISCOUNT_TOGGLE
    || type == MIDI_M_SYSEX_AHLBORN_GALANTI
    || type == MIDI_M_SYSEX_AHLBORN_GALANTI_TOGGLE
    || type == MIDI_M_SYSEX_JOHANNUS_11
    || type == MIDI_M_SYSEX_RODGERS_STOP_CHANGE
    || type == MIDI_M_NOTE_SHORT_OCTAVE)
    return true;
  return false;
}

bool GOMidiReceiverBase::HasUpperLimit(GOMidiReceiverMessageType type) {
  if (
    type == MIDI_M_NOTE || type == MIDI_M_PGM_RANGE || type == MIDI_M_RPN_RANGE
    || type == MIDI_M_NRPN_RANGE || type == MIDI_M_CTRL_CHANGE
    || type == MIDI_M_CTRL_CHANGE_FIXED || type == MIDI_M_RPN
    || type == MIDI_M_NRPN || type == MIDI_M_NOTE_ON
    || type == MIDI_M_NOTE_ON_OFF || type == MIDI_M_CTRL_CHANGE_ON
    || type == MIDI_M_CTRL_CHANGE_ON_OFF || type == MIDI_M_CTRL_CHANGE_FIXED_ON
    || type == MIDI_M_CTRL_CHANGE_FIXED_ON_OFF || type == MIDI_M_RPN_ON
    || type == MIDI_M_RPN_ON_OFF || type == MIDI_M_NRPN_ON
    || type == MIDI_M_NRPN_ON_OFF || type == MIDI_M_NOTE_NO_VELOCITY
    || type == MIDI_M_NOTE_NORMAL || type == MIDI_M_SYSEX_VISCOUNT
    || type == MIDI_M_SYSEX_AHLBORN_GALANTI
    || type == MIDI_M_SYSEX_AHLBORN_GALANTI_TOGGLE
    || type == MIDI_M_SYSEX_JOHANNUS_11 || type == MIDI_M_NOTE_SHORT_OCTAVE)
    return true;
  return false;
}

unsigned GOMidiReceiverBase::KeyLimit(GOMidiReceiverMessageType type) {
  if (type == MIDI_M_PGM_CHANGE)
    return 0x200000;
  if (
    type == MIDI_M_RPN_ON || type == MIDI_M_RPN_OFF || type == MIDI_M_RPN_ON_OFF
    || type == MIDI_M_RPN || type == MIDI_M_NRPN || type == MIDI_M_NRPN_ON
    || type == MIDI_M_NRPN_OFF || type == MIDI_M_NRPN_ON_OFF)
    return 0x3fff;
  return 0x7f;
}

unsigned GOMidiReceiverBase::LowerValueLimit(GOMidiReceiverMessageType type) {
  if (
    type == MIDI_M_RPN_RANGE || type == MIDI_M_NRPN_RANGE
    || type == MIDI_M_SYSEX_AHLBORN_GALANTI
    || type == MIDI_M_SYSEX_AHLBORN_GALANTI_TOGGLE)
    return 0x3fff;

  if (type == MIDI_M_SYSEX_VISCOUNT || type == MIDI_M_SYSEX_VISCOUNT_TOGGLE)
    return 0x1FFFFF;

  if (type == MIDI_M_PGM_RANGE)
    return 0x200000;

  if (type == MIDI_M_CTRL_BIT)
    return 7;

  if (type == MIDI_M_SYSEX_RODGERS_STOP_CHANGE)
    return 35 * 7;

  return 0x7f;
}

unsigned GOMidiReceiverBase::UpperValueLimit(GOMidiReceiverMessageType type) {
  if (
    type == MIDI_M_RPN_RANGE || type == MIDI_M_NRPN_RANGE
    || type == MIDI_M_SYSEX_AHLBORN_GALANTI
    || type == MIDI_M_SYSEX_AHLBORN_GALANTI_TOGGLE)
    return 0x3fff;

  if (type == MIDI_M_SYSEX_VISCOUNT)
    return 0x1FFFFF;

  if (type == MIDI_M_PGM_RANGE)
    return 0x200000;

  return 0x7f;
}

GOMidiMatchType GOMidiReceiverBase::debounce(
  const GOMidiEvent &e, GOMidiMatchType event, unsigned index) {
  if (m_events.size() != m_last.size())
    m_last.resize(m_events.size());
  if (e.GetTime() < m_last[index] + m_events[index].debounce_time)
    return MIDI_MATCH_NONE;
  m_last[index] = e.GetTime();
  return event;
}

void GOMidiReceiverBase::deleteInternal(unsigned device) {
  for (unsigned i = 0; i < m_Internal.size(); i++)
    if (m_Internal[i].device == device) {
      m_Internal[i] = m_Internal[m_Internal.size() - 1];
      m_Internal.resize(m_Internal.size() - 1);
      break;
    }
}
unsigned GOMidiReceiverBase::createInternal(unsigned device) {
  unsigned pos = 0;
  while (pos < m_Internal.size() && m_Internal[pos].device != device)
    pos++;
  if (pos >= m_Internal.size()) {
    m_Internal.resize(m_Internal.size() + 1);
    m_Internal[pos].device = device;
  }
  return pos;
}

GOMidiMatchType GOMidiReceiverBase::Match(const GOMidiEvent &e) {
  int tmp;
  return Match(e, tmp);
}

GOMidiMatchType GOMidiReceiverBase::Match(const GOMidiEvent &e, int &value) {
  int tmp;
  return Match(e, NULL, tmp, value);
}

GOMidiMatchType GOMidiReceiverBase::Match(
  const GOMidiEvent &e, const unsigned midi_map[128], int &key, int &value) {
  value = 0;

  if (
    e.GetMidiType() == MIDI_SYSEX_GO_CLEAR
    || e.GetMidiType() == MIDI_SYSEX_GO_SAMPLESET) {
    if (e.GetMidiType() == MIDI_SYSEX_GO_CLEAR && e.GetChannel() == 0)
      deleteInternal(e.GetDevice());
    else {
      unsigned pos = createInternal(e.GetDevice());
      m_Internal[pos].channel = -1;
    }
    return MIDI_MATCH_NONE;
  }
  if (e.GetMidiType() == MIDI_SYSEX_GO_SETUP) {
    if (m_ElementID == -1)
      return MIDI_MATCH_NONE;
    if (m_ElementID != e.GetKey())
      return MIDI_MATCH_NONE;

    unsigned pos = createInternal(e.GetDevice());
    m_Internal[pos].channel = e.GetChannel();
    m_Internal[pos].key = e.GetValue();
    return MIDI_MATCH_NONE;
  }

  for (unsigned i = 0; i < m_Internal.size(); i++)
    if (m_Internal[i].device == e.GetDevice()) {
      if (m_type == MIDI_RECV_MANUAL) {
        if (
          e.GetMidiType() == MIDI_NOTE
          && e.GetChannel() == m_Internal[i].channel) {
          key = e.GetKey() + GetTranspose();
          value = e.GetValue();
          if (key < 0)
            return MIDI_MATCH_NONE;
          if (key > 127)
            return MIDI_MATCH_NONE;
          if (value == 0)
            return MIDI_MATCH_OFF;
          else
            return MIDI_MATCH_ON;
        }

        if (
          e.GetMidiType() == MIDI_CTRL_CHANGE
          && e.GetKey() == MIDI_CTRL_NOTES_OFF
          && e.GetChannel() == m_Internal[i].channel)
          return MIDI_MATCH_RESET;
      } else {
        if (
          e.GetMidiType() == MIDI_NRPN
          && e.GetChannel() == m_Internal[i].channel
          && e.GetKey() == m_Internal[i].key) {
          value = e.GetValue();
          if (m_type == MIDI_RECV_ENCLOSURE)
            return MIDI_MATCH_CHANGE;
          if (value == 0)
            return MIDI_MATCH_OFF;
          else
            return MIDI_MATCH_ON;
        }
      }
      return MIDI_MATCH_NONE;
    }

  for (unsigned i = 0; i < m_events.size(); i++) {
    if (
      m_events[i].channel != -1 && m_events[i].channel != e.GetChannel()
      && HasChannel(m_events[i].type))
      continue;
    if (m_events[i].deviceId != 0 && m_events[i].deviceId != e.GetDevice())
      continue;
    if (m_type == MIDI_RECV_MANUAL) {
      if (
        m_events[i].type != MIDI_M_NOTE
        && m_events[i].type != MIDI_M_NOTE_NO_VELOCITY
        && m_events[i].type != MIDI_M_NOTE_SHORT_OCTAVE
        && m_events[i].type != MIDI_M_NOTE_NORMAL)
        continue;
      if (e.GetMidiType() == MIDI_NOTE || e.GetMidiType() == MIDI_AFTERTOUCH) {
        if (
          e.GetKey() < m_events[i].low_key || e.GetKey() > m_events[i].high_key)
          continue;
        key = e.GetKey();
        if (m_events[i].type == MIDI_M_NOTE_SHORT_OCTAVE) {
          int no = e.GetKey() - m_events[i].low_key;
          if (no <= 3)
            continue;
          if (no == 4 || no == 6 || no == 8)
            key -= 4;
        }
        key = key + GetTranspose() + m_events[i].key;
        if (key < 0)
          continue;
        if (key > 127)
          continue;
        if (
          midi_map && m_events[i].type != MIDI_M_NOTE_SHORT_OCTAVE
          && m_events[i].type != MIDI_M_NOTE_NORMAL)
          key = midi_map[key];
        if (m_events[i].type == MIDI_M_NOTE_NO_VELOCITY) {
          value = e.GetValue() ? 127 : 0;
          if (e.GetMidiType() == MIDI_AFTERTOUCH)
            continue;
        } else
          value = m_events[i].ConvertSrcValueToInt(e.GetValue());
        if (m_events[i].low_value <= m_events[i].high_value) {
          if (e.GetValue() < m_events[i].low_value)
            return MIDI_MATCH_OFF;
          if (e.GetValue() <= m_events[i].high_value)
            return MIDI_MATCH_ON;
        } else {
          if (e.GetValue() >= m_events[i].low_value)
            return MIDI_MATCH_OFF;
          if (e.GetValue() >= m_events[i].high_value)
            return MIDI_MATCH_ON;
        }
        continue;
      }
      if (
        e.GetMidiType() == MIDI_CTRL_CHANGE
        && e.GetKey() == MIDI_CTRL_NOTES_OFF)
        return MIDI_MATCH_RESET;

      if (
        e.GetMidiType() == MIDI_CTRL_CHANGE
        && e.GetKey() == MIDI_CTRL_SOUNDS_OFF)
        return MIDI_MATCH_RESET;

      continue;
    }
    if (m_type == MIDI_RECV_ENCLOSURE) {
      if (
        m_events[i].type == MIDI_M_CTRL_CHANGE
        && e.GetMidiType() == MIDI_CTRL_CHANGE
        && m_events[i].key == e.GetKey()) {
        value = m_events[i].ConvertSrcValueToInt(e.GetValue());
        return MIDI_MATCH_CHANGE;
      }
      if (
        m_events[i].type == MIDI_M_RPN && e.GetMidiType() == MIDI_RPN
        && m_events[i].key == e.GetKey()) {
        value = m_events[i].ConvertSrcValueToInt(e.GetValue());
        return MIDI_MATCH_CHANGE;
      }
      if (
        m_events[i].type == MIDI_M_NRPN && e.GetMidiType() == MIDI_NRPN
        && m_events[i].key == e.GetKey()) {
        value = m_events[i].ConvertSrcValueToInt(e.GetValue());
        return MIDI_MATCH_CHANGE;
      }
      if (
        m_events[i].type == MIDI_M_PGM_RANGE
        && e.GetMidiType() == MIDI_PGM_CHANGE)
        if (
          (m_events[i].low_value <= e.GetKey()
           && e.GetKey() <= m_events[i].high_value)
          || (m_events[i].high_value <= e.GetKey() && e.GetKey() <= m_events[i].low_value)) {
          value = m_events[i].ConvertSrcValueToInt(e.GetValue());
          return MIDI_MATCH_CHANGE;
        }
      continue;
    }
    if (
      e.GetMidiType() == MIDI_NOTE && m_events[i].type == MIDI_M_NOTE
      && m_events[i].key == e.GetKey()) {
      if (m_events[i].low_value <= m_events[i].high_value) {
        if (e.GetValue() <= m_events[i].low_value)
          return MIDI_MATCH_OFF;
        if (e.GetValue() >= m_events[i].high_value)
          return MIDI_MATCH_ON;
      } else {
        if (e.GetValue() >= m_events[i].low_value)
          return MIDI_MATCH_OFF;
        if (e.GetValue() <= m_events[i].high_value)
          return MIDI_MATCH_ON;
      }
      continue;
    }
    if (
      e.GetMidiType() == MIDI_NOTE && m_events[i].type == MIDI_M_NOTE_ON
      && m_events[i].key == e.GetKey()
      && e.GetValue() >= m_events[i].high_value)
      return debounce(e, MIDI_MATCH_CHANGE, i);
    if (
      e.GetMidiType() == MIDI_NOTE && m_events[i].type == MIDI_M_NOTE_OFF
      && m_events[i].key == e.GetKey() && e.GetValue() <= m_events[i].low_value)
      return debounce(e, MIDI_MATCH_CHANGE, i);
    if (
      e.GetMidiType() == MIDI_NOTE && m_events[i].type == MIDI_M_NOTE_ON_OFF
      && m_events[i].key == e.GetKey()
      && e.GetValue() >= m_events[i].high_value)
      return debounce(e, MIDI_MATCH_CHANGE, i);
    if (
      e.GetMidiType() == MIDI_NOTE && m_events[i].type == MIDI_M_NOTE_ON_OFF
      && m_events[i].key == e.GetKey() && e.GetValue() <= m_events[i].low_value)
      return debounce(e, MIDI_MATCH_CHANGE, i);

    if (
      e.GetMidiType() == MIDI_CTRL_CHANGE
      && m_events[i].type == MIDI_M_CTRL_CHANGE
      && m_events[i].key == e.GetKey()) {
      if (m_events[i].low_value <= m_events[i].high_value) {
        if (e.GetValue() <= m_events[i].low_value)
          return debounce(e, MIDI_MATCH_OFF, i);
        if (e.GetValue() >= m_events[i].high_value)
          return debounce(e, MIDI_MATCH_ON, i);
      } else {
        if (e.GetValue() >= m_events[i].low_value)
          return debounce(e, MIDI_MATCH_OFF, i);
        if (e.GetValue() <= m_events[i].high_value)
          return debounce(e, MIDI_MATCH_ON, i);
      }
      continue;
    }
    if (
      e.GetMidiType() == MIDI_CTRL_CHANGE
      && m_events[i].type == MIDI_M_CTRL_CHANGE_ON
      && m_events[i].key == e.GetKey()
      && e.GetValue() >= m_events[i].high_value)
      return debounce(e, MIDI_MATCH_CHANGE, i);
    if (
      e.GetMidiType() == MIDI_CTRL_CHANGE
      && m_events[i].type == MIDI_M_CTRL_CHANGE_OFF
      && m_events[i].key == e.GetKey() && e.GetValue() <= m_events[i].low_value)
      return debounce(e, MIDI_MATCH_CHANGE, i);
    if (
      e.GetMidiType() == MIDI_CTRL_CHANGE
      && m_events[i].type == MIDI_M_CTRL_CHANGE_ON_OFF
      && m_events[i].key == e.GetKey()
      && e.GetValue() >= m_events[i].high_value)
      return debounce(e, MIDI_MATCH_CHANGE, i);
    if (
      e.GetMidiType() == MIDI_CTRL_CHANGE
      && m_events[i].type == MIDI_M_CTRL_CHANGE_ON_OFF
      && m_events[i].key == e.GetKey() && e.GetValue() <= m_events[i].low_value)
      return debounce(e, MIDI_MATCH_CHANGE, i);
    if (
      e.GetMidiType() == MIDI_CTRL_CHANGE
      && m_events[i].type == MIDI_M_CTRL_CHANGE_FIXED
      && m_events[i].key == e.GetKey()) {
      if (e.GetValue() == m_events[i].low_value)
        return debounce(e, MIDI_MATCH_OFF, i);
      if (e.GetValue() == m_events[i].high_value)
        return debounce(e, MIDI_MATCH_ON, i);
      continue;
    }
    if (
      e.GetMidiType() == MIDI_CTRL_CHANGE
      && m_events[i].type == MIDI_M_CTRL_CHANGE_FIXED_ON
      && m_events[i].key == e.GetKey()
      && e.GetValue() == m_events[i].high_value)
      return debounce(e, MIDI_MATCH_CHANGE, i);
    if (
      e.GetMidiType() == MIDI_CTRL_CHANGE
      && m_events[i].type == MIDI_M_CTRL_CHANGE_FIXED_OFF
      && m_events[i].key == e.GetKey() && e.GetValue() == m_events[i].low_value)
      return debounce(e, MIDI_MATCH_CHANGE, i);
    if (
      e.GetMidiType() == MIDI_CTRL_CHANGE
      && m_events[i].type == MIDI_M_CTRL_CHANGE_FIXED_ON_OFF
      && m_events[i].key == e.GetKey()
      && e.GetValue() == m_events[i].high_value)
      return debounce(e, MIDI_MATCH_CHANGE, i);
    if (
      e.GetMidiType() == MIDI_CTRL_CHANGE
      && m_events[i].type == MIDI_M_CTRL_CHANGE_FIXED_ON_OFF
      && m_events[i].key == e.GetKey() && e.GetValue() == m_events[i].low_value)
      return debounce(e, MIDI_MATCH_CHANGE, i);
    if (
      e.GetMidiType() == MIDI_CTRL_CHANGE && m_events[i].type == MIDI_M_CTRL_BIT
      && m_events[i].key == e.GetKey()) {
      unsigned mask = 1 << m_events[i].low_value;
      if (e.GetValue() & mask)
        return debounce(e, MIDI_MATCH_ON, i);
      else
        return debounce(e, MIDI_MATCH_OFF, i);
    }

    if (
      e.GetMidiType() == MIDI_RPN && m_events[i].type == MIDI_M_RPN
      && m_events[i].key == e.GetKey()) {
      if (m_events[i].low_value <= m_events[i].high_value) {
        if (e.GetValue() <= m_events[i].low_value)
          return MIDI_MATCH_OFF;
        if (e.GetValue() >= m_events[i].high_value)
          return MIDI_MATCH_ON;
      } else {
        if (e.GetValue() >= m_events[i].low_value)
          return MIDI_MATCH_OFF;
        if (e.GetValue() <= m_events[i].high_value)
          return MIDI_MATCH_ON;
      }
      continue;
    }
    if (
      e.GetMidiType() == MIDI_RPN && m_events[i].type == MIDI_M_RPN_ON
      && m_events[i].key == e.GetKey()
      && e.GetValue() >= m_events[i].high_value)
      return debounce(e, MIDI_MATCH_CHANGE, i);
    if (
      e.GetMidiType() == MIDI_RPN && m_events[i].type == MIDI_M_RPN_OFF
      && m_events[i].key == e.GetKey() && e.GetValue() <= m_events[i].low_value)
      return debounce(e, MIDI_MATCH_CHANGE, i);
    if (
      e.GetMidiType() == MIDI_RPN && m_events[i].type == MIDI_M_RPN_ON_OFF
      && m_events[i].key == e.GetKey()
      && e.GetValue() >= m_events[i].high_value)
      return debounce(e, MIDI_MATCH_CHANGE, i);
    if (
      e.GetMidiType() == MIDI_RPN && m_events[i].type == MIDI_M_RPN_ON_OFF
      && m_events[i].key == e.GetKey() && e.GetValue() <= m_events[i].low_value)
      return debounce(e, MIDI_MATCH_CHANGE, i);

    if (
      e.GetMidiType() == MIDI_NRPN && m_events[i].type == MIDI_M_NRPN
      && m_events[i].key == e.GetKey()) {
      if (m_events[i].low_value <= m_events[i].high_value) {
        if (e.GetValue() <= m_events[i].low_value)
          return MIDI_MATCH_OFF;
        if (e.GetValue() >= m_events[i].high_value)
          return MIDI_MATCH_ON;
      } else {
        if (e.GetValue() >= m_events[i].low_value)
          return MIDI_MATCH_OFF;
        if (e.GetValue() <= m_events[i].high_value)
          return MIDI_MATCH_ON;
      }
      continue;
    }
    if (
      e.GetMidiType() == MIDI_NRPN && m_events[i].type == MIDI_M_NRPN_ON
      && m_events[i].key == e.GetKey()
      && e.GetValue() >= m_events[i].high_value)
      return debounce(e, MIDI_MATCH_CHANGE, i);
    if (
      e.GetMidiType() == MIDI_NRPN && m_events[i].type == MIDI_M_NRPN_OFF
      && m_events[i].key == e.GetKey() && e.GetValue() <= m_events[i].low_value)
      return debounce(e, MIDI_MATCH_CHANGE, i);
    if (
      e.GetMidiType() == MIDI_NRPN && m_events[i].type == MIDI_M_NRPN_ON_OFF
      && m_events[i].key == e.GetKey()
      && e.GetValue() >= m_events[i].high_value)
      return debounce(e, MIDI_MATCH_CHANGE, i);
    if (
      e.GetMidiType() == MIDI_NRPN && m_events[i].type == MIDI_M_NRPN_ON_OFF
      && m_events[i].key == e.GetKey() && e.GetValue() <= m_events[i].low_value)
      return debounce(e, MIDI_MATCH_CHANGE, i);

    if (
      e.GetMidiType() == MIDI_PGM_CHANGE
      && m_events[i].type == MIDI_M_PGM_CHANGE
      && m_events[i].key == e.GetKey()) {
      return debounce(e, MIDI_MATCH_CHANGE, i);
    }
    if (
      e.GetMidiType() == MIDI_PGM_CHANGE && m_events[i].type == MIDI_M_PGM_RANGE
      && m_events[i].low_value == e.GetKey())
      return MIDI_MATCH_OFF;
    if (
      e.GetMidiType() == MIDI_PGM_CHANGE && m_events[i].type == MIDI_M_PGM_RANGE
      && m_events[i].high_value == e.GetKey())
      return MIDI_MATCH_ON;
    if (
      e.GetMidiType() == MIDI_SYSEX_JOHANNUS_9
      && m_events[i].type == MIDI_M_SYSEX_JOHANNUS_9
      && m_events[i].key == e.GetKey()) {
      return debounce(e, MIDI_MATCH_CHANGE, i);
    }
    if (
      e.GetMidiType() == MIDI_SYSEX_JOHANNUS_11
      && m_events[i].type == MIDI_M_SYSEX_JOHANNUS_11
      && m_events[i].key == e.GetKey() && m_events[i].low_value <= e.GetValue()
      && m_events[i].high_value >= e.GetValue()) {
      return debounce(e, MIDI_MATCH_CHANGE, i);
    }
    if (
      e.GetMidiType() == MIDI_RPN && m_events[i].type == MIDI_M_RPN_RANGE
      && m_events[i].low_value == e.GetKey() && m_events[i].key == e.GetValue())
      return MIDI_MATCH_OFF;
    if (
      e.GetMidiType() == MIDI_RPN && m_events[i].type == MIDI_M_RPN_RANGE
      && m_events[i].high_value == e.GetKey()
      && m_events[i].key == e.GetValue())
      return MIDI_MATCH_ON;
    if (
      e.GetMidiType() == MIDI_NRPN && m_events[i].type == MIDI_M_NRPN_RANGE
      && m_events[i].low_value == e.GetKey() && m_events[i].key == e.GetValue())
      return MIDI_MATCH_OFF;
    if (
      e.GetMidiType() == MIDI_NRPN && m_events[i].type == MIDI_M_NRPN_RANGE
      && m_events[i].high_value == e.GetKey()
      && m_events[i].key == e.GetValue())
      return MIDI_MATCH_ON;

    if (
      e.GetMidiType() == MIDI_SYSEX_VISCOUNT
      && m_events[i].type == MIDI_M_SYSEX_VISCOUNT
      && m_events[i].low_value == e.GetValue())
      return MIDI_MATCH_OFF;
    if (
      e.GetMidiType() == MIDI_SYSEX_VISCOUNT
      && m_events[i].type == MIDI_M_SYSEX_VISCOUNT
      && m_events[i].high_value == e.GetValue())
      return MIDI_MATCH_ON;
    if (
      e.GetMidiType() == MIDI_SYSEX_VISCOUNT
      && m_events[i].type == MIDI_M_SYSEX_VISCOUNT_TOGGLE
      && m_events[i].low_value == e.GetValue()) {
      return debounce(e, MIDI_MATCH_CHANGE, i);
    }
    if (
      e.GetMidiType() == MIDI_SYSEX_RODGERS_STOP_CHANGE
      && m_events[i].type == MIDI_M_SYSEX_RODGERS_STOP_CHANGE
      && m_events[i].key == e.GetChannel()) {
      switch (GORodgersGetBit(m_events[i].low_value, e.GetKey(), e.GetData())) {
      case MIDI_BIT_STATE::MIDI_BIT_CLEAR:
        return MIDI_MATCH_OFF;

      case MIDI_BIT_STATE::MIDI_BIT_SET:
        return MIDI_MATCH_ON;

      case MIDI_BIT_STATE::MIDI_BIT_NOT_PRESENT:;
      }
    }
    if (
      e.GetMidiType() == MIDI_SYSEX_AHLBORN_GALANTI
      && m_events[i].type == MIDI_M_SYSEX_AHLBORN_GALANTI
      && m_events[i].low_value == e.GetValue())
      return MIDI_MATCH_OFF;
    if (
      e.GetMidiType() == MIDI_SYSEX_AHLBORN_GALANTI
      && m_events[i].type == MIDI_M_SYSEX_AHLBORN_GALANTI
      && m_events[i].high_value == e.GetValue())
      return MIDI_MATCH_ON;
    if (
      e.GetMidiType() == MIDI_SYSEX_AHLBORN_GALANTI
      && m_events[i].type == MIDI_M_SYSEX_AHLBORN_GALANTI_TOGGLE
      && m_events[i].low_value == e.GetValue())
      return debounce(e, MIDI_MATCH_CHANGE, i);
    if (
      e.GetMidiType() == MIDI_SYSEX_AHLBORN_GALANTI
      && m_events[i].type == MIDI_M_SYSEX_AHLBORN_GALANTI_TOGGLE
      && m_events[i].high_value == e.GetValue())
      return debounce(e, MIDI_MATCH_CHANGE, i);
  }
  return MIDI_MATCH_NONE;
}

void GOMidiReceiverBase::Assign(const GOMidiReceiverEventPatternList &data) {
  *(GOMidiReceiverEventPatternList *)this = data;
}

void GOMidiReceiverBase::PreparePlayback() { m_Internal.resize(0); }
