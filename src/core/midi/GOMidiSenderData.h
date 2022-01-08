/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDISENDERDATA_H
#define GOMIDISENDERDATA_H

#include <vector>

typedef enum {
  MIDI_SEND_BUTTON,
  MIDI_SEND_LABEL,
  MIDI_SEND_ENCLOSURE,
  MIDI_SEND_MANUAL,
} MIDI_SENDER_TYPE;

typedef enum {
  MIDI_S_NONE,
  MIDI_S_NOTE,
  MIDI_S_NOTE_NO_VELOCITY,
  MIDI_S_CTRL,
  MIDI_S_RPN,
  MIDI_S_NRPN,
  MIDI_S_RPN_RANGE,
  MIDI_S_NRPN_RANGE,
  MIDI_S_PGM_RANGE,
  MIDI_S_PGM_ON,
  MIDI_S_PGM_OFF,
  MIDI_S_NOTE_ON,
  MIDI_S_NOTE_OFF,
  MIDI_S_CTRL_ON,
  MIDI_S_CTRL_OFF,
  MIDI_S_RPN_ON,
  MIDI_S_RPN_OFF,
  MIDI_S_NRPN_ON,
  MIDI_S_NRPN_OFF,
  MIDI_S_HW_NAME_STRING,
  MIDI_S_HW_NAME_LCD,
  MIDI_S_HW_STRING,
  MIDI_S_HW_LCD,
  MIDI_S_RODGERS_STOP_CHANGE,
} midi_send_message_type;

typedef struct {
  unsigned device;
  midi_send_message_type type;
  unsigned channel;
  unsigned key;
  unsigned low_value;
  unsigned high_value;
  unsigned start;
  unsigned length;
} MIDI_SEND_EVENT;

class GOMidiSenderData {
 protected:
  MIDI_SENDER_TYPE m_type;
  std::vector<MIDI_SEND_EVENT> m_events;

 public:
  GOMidiSenderData(MIDI_SENDER_TYPE type);
  virtual ~GOMidiSenderData();

  MIDI_SENDER_TYPE GetType() const;

  unsigned GetEventCount() const;
  MIDI_SEND_EVENT& GetEvent(unsigned index);
  unsigned AddNewEvent();
  void DeleteEvent(unsigned index);
};

#endif
