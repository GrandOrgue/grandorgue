/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiSender.h"

#include <wx/intl.h>

#include "config/GOConfigEnum.h"
#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"
#include "midi/GOMidiEvent.h"
#include "midi/GOMidiMap.h"

#include "GOMidiSendProxy.h"

GOMidiSender::GOMidiSender(GOMidiSendProxy &proxy, GOMidiSenderType type)
  : GOMidiSenderEventPatternList(type), r_proxy(proxy), m_ElementID(-1) {}

GOMidiSender::~GOMidiSender() {}

static const GOConfigEnum MIDI_SEND_TYPES({
  {wxT("Note"), MIDI_S_NOTE},
  {wxT("NoteNoVelocity"), MIDI_S_NOTE_NO_VELOCITY},
  {wxT("ControlChange"), MIDI_S_CTRL},
  {wxT("RPN"), MIDI_S_RPN},
  {wxT("NRPN"), MIDI_S_NRPN},
  {wxT("ProgramOn"), MIDI_S_PGM_ON},
  {wxT("ProgramOff"), MIDI_S_PGM_OFF},
  {wxT("ProgramRange"), MIDI_S_PGM_RANGE},
  {wxT("RPNRange"), MIDI_S_RPN_RANGE},
  {wxT("NRPNRange"), MIDI_S_NRPN_RANGE},
  {wxT("NoteOn"), MIDI_S_NOTE_ON},
  {wxT("NoteOff"), MIDI_S_NOTE_OFF},
  {wxT("ControlOn"), MIDI_S_CTRL_ON},
  {wxT("ControlOff"), MIDI_S_CTRL_OFF},
  {wxT("RPNOn"), MIDI_S_RPN_ON},
  {wxT("RPNOff"), MIDI_S_RPN_OFF},
  {wxT("NRPNOn"), MIDI_S_NRPN_ON},
  {wxT("NRPNOff"), MIDI_S_NRPN_OFF},
  {wxT("HWNameLCD"), MIDI_S_HW_NAME_LCD},
  {wxT("HWNameString"), MIDI_S_HW_NAME_STRING},
  {wxT("HWLCD"), MIDI_S_HW_LCD},
  {wxT("HWString"), MIDI_S_HW_STRING},
  {wxT("RodgersStopChange"), MIDI_S_RODGERS_STOP_CHANGE},
});

void GOMidiSender::SetElementID(int id) { m_ElementID = id; }

void GOMidiSender::Load(
  GOConfigReader &cfg, const wxString &group, GOMidiMap &map) {
  m_events.resize(0);

  int event_cnt = cfg.ReadInteger(
    CMBSetting, group, wxT("NumberOfMIDISendEvents"), 0, 255, false);

  m_events.resize(event_cnt);
  for (unsigned i = 0; i < m_events.size(); i++) {
    m_events[i].deviceId = map.GetDeviceIdByLogicalName(cfg.ReadString(
      CMBSetting,
      group,
      wxString::Format(wxT("MIDISendDevice%03d"), i + 1),
      false));

    const GOMidiSenderMessageType eventType
      = (GOMidiSenderMessageType)cfg.ReadEnum(
        CMBSetting,
        group,
        wxString::Format(wxT("MIDISendEventType%03d"), i + 1),
        MIDI_SEND_TYPES);

    m_events[i].type = eventType;
    if (hasChannel(eventType))
      m_events[i].channel = cfg.ReadInteger(
        CMBSetting,
        group,
        wxString::Format(wxT("MIDISendChannel%03d"), i + 1),
        1,
        16);
    if (HasKey(eventType))
      m_events[i].key = cfg.ReadInteger(
        CMBSetting,
        group,
        wxString::Format(wxT("MIDISendKey%03d"), i + 1),
        0,
        0x200000);

    if (isNote(eventType))
      m_events[i].useNoteOff = cfg.ReadBoolean(
        CMBSetting,
        group,
        wxString::Format(wxT("MIDISendNoteOff%03d"), i + 1),
        false,
        true);

    if (hasLowValue(eventType))
      m_events[i].low_value = cfg.ReadInteger(
        CMBSetting,
        group,
        wxString::Format(wxT("MIDISendLowValue%03d"), i + 1),
        0,
        lowValueLimit(eventType),
        false,
        0);

    if (hasHighValue(eventType))
      m_events[i].high_value = cfg.ReadInteger(
        CMBSetting,
        group,
        wxString::Format(wxT("MIDISendHighValue%03d"), i + 1),
        0,
        highValueLimit(eventType),
        false,
        0x7f);

    if (hasStart(eventType))
      m_events[i].start = cfg.ReadInteger(
        CMBSetting,
        group,
        wxString::Format(wxT("MIDISendStart%03d"), i + 1),
        0,
        0x1f,
        false,
        0);
    if (hasLength(eventType)) {
      unsigned maxLength = lengthLimit(eventType);

      m_events[i].length = cfg.ReadInteger(
        CMBSetting,
        group,
        wxString::Format(wxT("MIDISendLength%03d"), i + 1),
        0,
        maxLength,
        false,
        maxLength);
    }
  }
}

void GOMidiSender::Save(
  GOConfigWriter &cfg, const wxString &group, GOMidiMap &map) const {
  if (!m_events.empty()) {
    cfg.WriteInteger(group, wxT("NumberOfMIDISendEvents"), m_events.size());
    for (unsigned i = 0; i < m_events.size(); i++) {
      cfg.WriteString(
        group,
        wxString::Format(wxT("MIDISendDevice%03d"), i + 1),
        map.GetDeviceLogicalNameById(m_events[i].deviceId));
      cfg.WriteEnum(
        group,
        wxString::Format(wxT("MIDISendEventType%03d"), i + 1),
        MIDI_SEND_TYPES,
        m_events[i].type);
      if (hasChannel(m_events[i].type))
        cfg.WriteInteger(
          group,
          wxString::Format(wxT("MIDISendChannel%03d"), i + 1),
          m_events[i].channel);
      if (HasKey(m_events[i].type))
        cfg.WriteInteger(
          group,
          wxString::Format(wxT("MIDISendKey%03d"), i + 1),
          m_events[i].key);

      if (isNote(m_events[i].type))
        cfg.WriteBoolean(
          group,
          wxString::Format(wxT("MIDISendNoteOff%03d"), i + 1),
          m_events[i].useNoteOff);

      if (hasLowValue(m_events[i].type))
        cfg.WriteInteger(
          group,
          wxString::Format(wxT("MIDISendLowValue%03d"), i + 1),
          m_events[i].low_value);

      if (hasHighValue(m_events[i].type))
        cfg.WriteInteger(
          group,
          wxString::Format(wxT("MIDISendHighValue%03d"), i + 1),
          m_events[i].high_value);

      if (hasStart(m_events[i].type))
        cfg.WriteInteger(
          group,
          wxString::Format(wxT("MIDISendStart%03d"), i + 1),
          m_events[i].start);
      if (hasLength(m_events[i].type))
        cfg.WriteInteger(
          group,
          wxString::Format(wxT("MIDISendLength%03d"), i + 1),
          m_events[i].length);
    }
  }
}

bool GOMidiSender::hasChannel(GOMidiSenderMessageType type) {
  if (
    type == MIDI_S_NOTE || type == MIDI_S_NOTE_NO_VELOCITY
    || type == MIDI_S_CTRL || type == MIDI_S_RPN || type == MIDI_S_NRPN
    || type == MIDI_S_PGM_ON || type == MIDI_S_PGM_OFF
    || type == MIDI_S_PGM_RANGE || type == MIDI_S_RPN_RANGE
    || type == MIDI_S_NRPN_RANGE || type == MIDI_S_NOTE_ON
    || type == MIDI_S_NOTE_OFF || type == MIDI_S_CTRL_ON
    || type == MIDI_S_CTRL_OFF || type == MIDI_S_RPN_ON
    || type == MIDI_S_RPN_OFF || type == MIDI_S_NRPN_ON
    || type == MIDI_S_NRPN_OFF)
    return true;

  return false;
}

bool GOMidiSender::HasKey(GOMidiSenderMessageType type) const {
  if (m_type == MIDI_SEND_MANUAL)
    return false;

  if (
    type == MIDI_S_NOTE || type == MIDI_S_NOTE_NO_VELOCITY
    || type == MIDI_S_CTRL || type == MIDI_S_RPN || type == MIDI_S_NRPN
    || type == MIDI_S_RPN_RANGE || type == MIDI_S_NRPN_RANGE
    || type == MIDI_S_PGM_ON || type == MIDI_S_PGM_OFF || type == MIDI_S_NOTE_ON
    || type == MIDI_S_NOTE_OFF || type == MIDI_S_CTRL_ON
    || type == MIDI_S_CTRL_OFF || type == MIDI_S_RPN_ON
    || type == MIDI_S_RPN_OFF || type == MIDI_S_NRPN_ON
    || type == MIDI_S_NRPN_OFF || type == MIDI_S_RODGERS_STOP_CHANGE
    || type == MIDI_S_HW_NAME_STRING || type == MIDI_S_HW_NAME_LCD
    || type == MIDI_S_HW_STRING || type == MIDI_S_HW_LCD)
    return true;

  return false;
}

bool GOMidiSender::hasLowValue(GOMidiSenderMessageType type) {
  if (
    type == MIDI_S_NOTE_OFF || type == MIDI_S_CTRL_OFF || type == MIDI_S_RPN_OFF
    || type == MIDI_S_NRPN_OFF || type == MIDI_S_PGM_RANGE
    || type == MIDI_S_RPN_RANGE || type == MIDI_S_NRPN_RANGE
    || type == MIDI_S_NOTE || type == MIDI_S_NOTE_NO_VELOCITY
    || type == MIDI_S_RPN || type == MIDI_S_NRPN || type == MIDI_S_CTRL
    || type == MIDI_S_RODGERS_STOP_CHANGE || type == MIDI_S_HW_NAME_LCD
    || type == MIDI_S_HW_LCD)
    return true;
  return false;
}

bool GOMidiSender::hasHighValue(GOMidiSenderMessageType type) {
  if (
    type == MIDI_S_NOTE_ON || type == MIDI_S_CTRL_ON || type == MIDI_S_RPN_ON
    || type == MIDI_S_NRPN_ON || type == MIDI_S_PGM_RANGE
    || type == MIDI_S_RPN_RANGE || type == MIDI_S_NRPN_RANGE
    || type == MIDI_S_NOTE || type == MIDI_S_NOTE_NO_VELOCITY
    || type == MIDI_S_RPN || type == MIDI_S_NRPN || type == MIDI_S_CTRL)
    return true;
  return false;
}

bool GOMidiSender::hasStart(GOMidiSenderMessageType type) {
  if (
    type == MIDI_S_HW_NAME_STRING || type == MIDI_S_HW_NAME_LCD
    || type == MIDI_S_HW_STRING || type == MIDI_S_HW_LCD)
    return true;
  return false;
}

bool GOMidiSender::hasLength(GOMidiSenderMessageType type) {
  if (
    type == MIDI_S_HW_NAME_STRING || type == MIDI_S_HW_NAME_LCD
    || type == MIDI_S_HW_STRING || type == MIDI_S_HW_LCD)
    return true;
  return false;
}

bool GOMidiSender::isNote(GOMidiSenderMessageType type) {
  if (
    type == MIDI_S_NOTE || type == MIDI_S_NOTE_NO_VELOCITY
    || type == MIDI_S_NOTE_ON || type == MIDI_S_NOTE_OFF)
    return true;

  return false;
}

unsigned GOMidiSender::keyLimit(GOMidiSenderMessageType type) {
  if (type == MIDI_S_PGM_ON || type == MIDI_S_PGM_OFF)
    return 0x200000;

  if (
    type == MIDI_S_RPN || type == MIDI_S_NRPN || type == MIDI_S_RPN_ON
    || type == MIDI_S_RPN_OFF || type == MIDI_S_NRPN_ON
    || type == MIDI_S_NRPN_OFF || type == MIDI_S_HW_NAME_LCD
    || type == MIDI_S_HW_LCD)
    return 0x3fff;

  return 0x7f;
}

unsigned GOMidiSender::lowValueLimit(GOMidiSenderMessageType type) {
  if (type == MIDI_S_PGM_RANGE)
    return 0x200000;
  if (type == MIDI_S_RPN_RANGE || type == MIDI_S_NRPN_RANGE)
    return 0x3fff;
  if (type == MIDI_S_RODGERS_STOP_CHANGE)
    return 35 * 7;
  return 0x7f;
}

unsigned GOMidiSender::highValueLimit(GOMidiSenderMessageType type) {
  if (type == MIDI_S_PGM_RANGE)
    return 0x200000;
  if (type == MIDI_S_RPN_RANGE || type == MIDI_S_NRPN_RANGE)
    return 0x3fff;
  return 0x7f;
}

unsigned GOMidiSender::startLimit(GOMidiSenderMessageType type) {
  if (type == MIDI_S_HW_NAME_STRING || type == MIDI_S_HW_STRING)
    return 15;
  if (type == MIDI_S_HW_NAME_LCD || type == MIDI_S_HW_LCD)
    return 31;
  return 0x00;
}

unsigned GOMidiSender::lengthLimit(GOMidiSenderMessageType type) {
  if (type == MIDI_S_HW_NAME_STRING || type == MIDI_S_HW_STRING)
    return 16;
  if (type == MIDI_S_HW_NAME_LCD || type == MIDI_S_HW_LCD)
    return 32;
  return 0x00;
}

void GOMidiSender::SetDisplay(bool state) {
  if (m_ElementID != -1) {
    GOMidiEvent e;
    e.SetMidiType(GOMidiEvent::MIDI_NRPN);
    e.SetDevice(m_ElementID);
    e.SetValue(state ? 0x7F : 0x00);
    r_proxy.SendMidiRecorderMessage(e);
  }

  for (unsigned i = 0; i < m_events.size(); i++) {
    if (m_events[i].type == MIDI_S_NOTE) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_NOTE);
      e.SetChannel(m_events[i].channel);
      e.SetKey(m_events[i].key);
      e.SetValue(state ? m_events[i].high_value : m_events[i].low_value);
      e.SetUseNoteOff(m_events[i].useNoteOff);
      r_proxy.SendMidiMessage(e);
    }
    if (m_events[i].type == MIDI_S_CTRL) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_CTRL_CHANGE);
      e.SetChannel(m_events[i].channel);
      e.SetKey(m_events[i].key);
      e.SetValue(state ? m_events[i].high_value : m_events[i].low_value);
      r_proxy.SendMidiMessage(e);
    }
    if (m_events[i].type == MIDI_S_RPN) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_RPN);
      e.SetChannel(m_events[i].channel);
      e.SetKey(m_events[i].key);
      e.SetValue(state ? m_events[i].high_value : m_events[i].low_value);
      r_proxy.SendMidiMessage(e);
    }
    if (m_events[i].type == MIDI_S_NRPN) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_NRPN);
      e.SetChannel(m_events[i].channel);
      e.SetKey(m_events[i].key);
      e.SetValue(state ? m_events[i].high_value : m_events[i].low_value);
      r_proxy.SendMidiMessage(e);
    }
    if (m_events[i].type == MIDI_S_PGM_RANGE) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_PGM_CHANGE);
      e.SetChannel(m_events[i].channel);
      e.SetKey(state ? m_events[i].high_value : m_events[i].low_value);
      r_proxy.SendMidiMessage(e);
    }
    if (m_events[i].type == MIDI_S_RPN_RANGE) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_RPN);
      e.SetChannel(m_events[i].channel);
      e.SetValue(m_events[i].key);
      e.SetKey(state ? m_events[i].high_value : m_events[i].low_value);
      r_proxy.SendMidiMessage(e);
    }
    if (m_events[i].type == MIDI_S_NRPN_RANGE) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_NRPN);
      e.SetChannel(m_events[i].channel);
      e.SetValue(m_events[i].key);
      e.SetKey(state ? m_events[i].high_value : m_events[i].low_value);
      r_proxy.SendMidiMessage(e);
    }
    if (m_events[i].type == MIDI_S_PGM_ON && state) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_PGM_CHANGE);
      e.SetChannel(m_events[i].channel);
      e.SetKey(m_events[i].key);
      r_proxy.SendMidiMessage(e);
    }
    if (m_events[i].type == MIDI_S_PGM_OFF && !state) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_PGM_CHANGE);
      e.SetChannel(m_events[i].channel);
      e.SetKey(m_events[i].key);
      r_proxy.SendMidiMessage(e);
    }
    if (m_events[i].type == MIDI_S_NOTE_ON && state) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_NOTE);
      e.SetChannel(m_events[i].channel);
      e.SetKey(m_events[i].key);
      e.SetValue(m_events[i].high_value);
      e.SetUseNoteOff(m_events[i].useNoteOff);
      r_proxy.SendMidiMessage(e);
    }
    if (m_events[i].type == MIDI_S_NOTE_OFF && !state) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_NOTE);
      e.SetChannel(m_events[i].channel);
      e.SetKey(m_events[i].key);
      e.SetValue(m_events[i].low_value);
      r_proxy.SendMidiMessage(e);
    }
    if (m_events[i].type == MIDI_S_CTRL_ON && state) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_CTRL_CHANGE);
      e.SetChannel(m_events[i].channel);
      e.SetKey(m_events[i].key);
      e.SetValue(m_events[i].high_value);
      r_proxy.SendMidiMessage(e);
    }
    if (m_events[i].type == MIDI_S_CTRL_OFF && !state) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_CTRL_CHANGE);
      e.SetChannel(m_events[i].channel);
      e.SetKey(m_events[i].key);
      e.SetValue(m_events[i].low_value);
      r_proxy.SendMidiMessage(e);
    }
    if (m_events[i].type == MIDI_S_RPN_ON && state) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_RPN);
      e.SetChannel(m_events[i].channel);
      e.SetKey(m_events[i].key);
      e.SetValue(m_events[i].high_value);
      r_proxy.SendMidiMessage(e);
    }
    if (m_events[i].type == MIDI_S_RPN_OFF && !state) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_RPN);
      e.SetChannel(m_events[i].channel);
      e.SetKey(m_events[i].key);
      e.SetValue(m_events[i].low_value);
      r_proxy.SendMidiMessage(e);
    }
    if (m_events[i].type == MIDI_S_NRPN_ON && state) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_NRPN);
      e.SetChannel(m_events[i].channel);
      e.SetKey(m_events[i].key);
      e.SetValue(m_events[i].high_value);
      r_proxy.SendMidiMessage(e);
    }
    if (m_events[i].type == MIDI_S_NRPN_OFF && !state) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_NRPN);
      e.SetChannel(m_events[i].channel);
      e.SetKey(m_events[i].key);
      e.SetValue(m_events[i].low_value);
      r_proxy.SendMidiMessage(e);
    }
    if (m_events[i].type == MIDI_S_HW_LCD) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_SYSEX_HW_LCD);
      e.SetChannel(m_events[i].low_value);
      e.SetKey(m_events[i].key);
      e.SetValue(m_events[i].start);
      e.SetString(state ? _("ON") : _("OFF"), m_events[i].length);
      r_proxy.SendMidiMessage(e);
    }
    if (m_events[i].type == MIDI_S_HW_STRING) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_SYSEX_HW_STRING);
      e.SetKey(m_events[i].key);
      e.SetValue(m_events[i].start);
      e.SetString(state ? _("ON") : _("OFF"), m_events[i].length);
      r_proxy.SendMidiMessage(e);
    }
    if (m_events[i].type == MIDI_S_RODGERS_STOP_CHANGE) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_SYSEX_RODGERS_STOP_CHANGE);
      e.SetChannel(m_events[i].key);
      e.SetKey(m_events[i].low_value);
      e.SetValue(state);
      r_proxy.SendMidiMessage(e);
    }
  }
}

void GOMidiSender::ResetKey() {
  if (m_ElementID != -1) {
    GOMidiEvent e;
    e.SetMidiType(GOMidiEvent::MIDI_CTRL_CHANGE);
    e.SetDevice(m_ElementID);
    e.SetKey(MIDI_CTRL_NOTES_OFF);
    e.SetValue(0);
    r_proxy.SendMidiRecorderMessage(e);
  }

  for (unsigned i = 0; i < m_events.size(); i++) {
    if (
      m_events[i].type == MIDI_S_NOTE
      || m_events[i].type == MIDI_S_NOTE_NO_VELOCITY) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_CTRL_CHANGE);
      e.SetChannel(m_events[i].channel);
      e.SetKey(MIDI_CTRL_NOTES_OFF);
      e.SetValue(0);
      r_proxy.SendMidiMessage(e);
    }
  }
}

void GOMidiSender::SetKey(unsigned key, unsigned velocity) {
  if (m_ElementID != -1) {
    GOMidiEvent e;
    e.SetMidiType(GOMidiEvent::MIDI_NOTE);
    e.SetDevice(m_ElementID);
    e.SetKey(key & 0x7F);
    e.SetValue(velocity & 0x7F);
    e.SetUseNoteOff(true);
    r_proxy.SendMidiRecorderMessage(e);
  }

  for (unsigned i = 0; i < m_events.size(); i++) {
    if (m_events[i].type == MIDI_S_NOTE) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_NOTE);
      e.SetChannel(m_events[i].channel);
      e.SetKey(key);
      e.SetValue(m_events[i].ConvertIntValueToDst(velocity));
      e.SetUseNoteOff(m_events[i].useNoteOff);
      r_proxy.SendMidiMessage(e);
    }
    if (m_events[i].type == MIDI_S_NOTE_NO_VELOCITY) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_NOTE);
      e.SetChannel(m_events[i].channel);
      e.SetKey(key);
      e.SetValue(velocity ? m_events[i].high_value : m_events[i].low_value);
      r_proxy.SendMidiMessage(e);
    }
  }
}

void GOMidiSender::SetValue(unsigned value) {
  if (m_ElementID != -1) {
    GOMidiEvent e;
    e.SetMidiType(GOMidiEvent::MIDI_NRPN);
    e.SetDevice(m_ElementID);
    e.SetValue(value & 0x7F);
    r_proxy.SendMidiRecorderMessage(e);
  }

  for (unsigned i = 0; i < m_events.size(); i++) {
    if (m_events[i].type == MIDI_S_CTRL) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_CTRL_CHANGE);
      e.SetChannel(m_events[i].channel);
      e.SetKey(m_events[i].key);
      e.SetValue(m_events[i].ConvertIntValueToDst(value));
      r_proxy.SendMidiMessage(e);
    }
    if (m_events[i].type == MIDI_S_RPN) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_RPN);
      e.SetChannel(m_events[i].channel);
      e.SetKey(m_events[i].key);
      e.SetValue(m_events[i].ConvertIntValueToDst(value));
      r_proxy.SendMidiMessage(e);
    }
    if (m_events[i].type == MIDI_S_NRPN) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_NRPN);
      e.SetChannel(m_events[i].channel);
      e.SetKey(m_events[i].key);
      e.SetValue(m_events[i].ConvertIntValueToDst(value));
      r_proxy.SendMidiMessage(e);
    }
    if (m_events[i].type == MIDI_S_PGM_RANGE) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_PGM_CHANGE);
      e.SetChannel(m_events[i].channel);
      e.SetKey(m_events[i].ConvertIntValueToDst(value));
      r_proxy.SendMidiMessage(e);
    }
    if (m_events[i].type == MIDI_S_HW_LCD) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_SYSEX_HW_LCD);
      e.SetChannel(m_events[i].low_value);
      e.SetKey(m_events[i].key);
      e.SetValue(m_events[i].start);
      e.SetString(
        wxString::Format(_("%d %%"), value * 100 / 127), m_events[i].length);
      r_proxy.SendMidiMessage(e);
    }
    if (m_events[i].type == MIDI_S_HW_STRING) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_SYSEX_HW_STRING);
      e.SetKey(m_events[i].key);
      e.SetValue(m_events[i].start);
      e.SetString(
        wxString::Format(_("%d %%"), value * 100 / 127), m_events[i].length);
      r_proxy.SendMidiMessage(e);
    }
  }
}

void GOMidiSender::SetLabel(const wxString &text) {
  for (unsigned i = 0; i < m_events.size(); i++) {
    if (m_events[i].type == MIDI_S_HW_LCD) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_SYSEX_HW_LCD);
      e.SetChannel(m_events[i].low_value);
      e.SetKey(m_events[i].key);
      e.SetValue(m_events[i].start);
      e.SetString(text, m_events[i].length);
      r_proxy.SendMidiMessage(e);
    }
    if (m_events[i].type == MIDI_S_HW_STRING) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_SYSEX_HW_STRING);
      e.SetKey(m_events[i].key);
      e.SetValue(m_events[i].start);
      e.SetString(text, m_events[i].length);
      r_proxy.SendMidiMessage(e);
    }
  }
}

void GOMidiSender::SetName(const wxString &text) {
  for (unsigned i = 0; i < m_events.size(); i++) {
    if (m_events[i].type == MIDI_S_HW_NAME_LCD) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_SYSEX_HW_LCD);
      e.SetChannel(m_events[i].low_value);
      e.SetKey(m_events[i].key);
      e.SetValue(m_events[i].start);
      e.SetString(text, m_events[i].length);
      r_proxy.SendMidiMessage(e);
    }
    if (m_events[i].type == MIDI_S_HW_NAME_STRING) {
      GOMidiEvent e;
      e.SetDevice(m_events[i].deviceId);
      e.SetMidiType(GOMidiEvent::MIDI_SYSEX_HW_STRING);
      e.SetKey(m_events[i].key);
      e.SetValue(m_events[i].start);
      e.SetString(text, m_events[i].length);
      r_proxy.SendMidiMessage(e);
    }
  }
}
