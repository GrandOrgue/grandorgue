/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIRECEIVERDATA_H
#define GOMIDIRECEIVERDATA_H

#include <vector>

#include "GOTime.h"

typedef enum {
  MIDI_RECV_DRAWSTOP,
  MIDI_RECV_BUTTON,
  MIDI_RECV_ENCLOSURE,
  MIDI_RECV_MANUAL,
  MIDI_RECV_SETTER,
  MIDI_RECV_ORGAN,
} MIDI_RECEIVER_TYPE;

typedef enum {
  MIDI_MATCH_NONE,
  MIDI_MATCH_ON,
  MIDI_MATCH_OFF,
  MIDI_MATCH_CHANGE,
  MIDI_MATCH_RESET,
} MIDI_MATCH_TYPE;

typedef enum {
  MIDI_M_NONE,
  MIDI_M_NOTE,
  MIDI_M_CTRL_CHANGE,
  MIDI_M_PGM_CHANGE,
  MIDI_M_PGM_RANGE,
  MIDI_M_SYSEX_JOHANNUS_9,
  MIDI_M_SYSEX_JOHANNUS_11,
  MIDI_M_SYSEX_VISCOUNT,
  MIDI_M_SYSEX_VISCOUNT_TOGGLE,
  MIDI_M_SYSEX_RODGERS_STOP_CHANGE,
  MIDI_M_SYSEX_AHLBORN_GALANTI,
  MIDI_M_SYSEX_AHLBORN_GALANTI_TOGGLE,
  MIDI_M_CTRL_BIT,
  MIDI_M_CTRL_CHANGE_FIXED,
  MIDI_M_RPN,
  MIDI_M_NRPN,
  MIDI_M_RPN_RANGE,
  MIDI_M_NRPN_RANGE,
  MIDI_M_NOTE_ON,
  MIDI_M_NOTE_OFF,
  MIDI_M_NOTE_ON_OFF,
  MIDI_M_CTRL_CHANGE_ON,
  MIDI_M_CTRL_CHANGE_OFF,
  MIDI_M_CTRL_CHANGE_ON_OFF,
  MIDI_M_CTRL_CHANGE_FIXED_ON,
  MIDI_M_CTRL_CHANGE_FIXED_OFF,
  MIDI_M_CTRL_CHANGE_FIXED_ON_OFF,
  MIDI_M_RPN_ON,
  MIDI_M_RPN_OFF,
  MIDI_M_RPN_ON_OFF,
  MIDI_M_NRPN_ON,
  MIDI_M_NRPN_OFF,
  MIDI_M_NRPN_ON_OFF,
  MIDI_M_NOTE_NO_VELOCITY,
  MIDI_M_NOTE_SHORT_OCTAVE,
  MIDI_M_NOTE_NORMAL,
} midi_match_message_type;

typedef struct {
  unsigned device;
  midi_match_message_type type;
  int channel;
  int key;
  int low_key;
  int high_key;
  int low_value;
  int high_value;
  unsigned debounce_time;
} MIDI_MATCH_EVENT;

class GOMidiReceiverData {
 protected:
  MIDI_RECEIVER_TYPE m_type;
  std::vector<MIDI_MATCH_EVENT> m_events;

 public:
  GOMidiReceiverData(MIDI_RECEIVER_TYPE type);
  virtual ~GOMidiReceiverData();

  MIDI_RECEIVER_TYPE GetType() const;

  unsigned GetEventCount() const;
  MIDI_MATCH_EVENT &GetEvent(unsigned index);
  unsigned AddNewEvent();
  void DeleteEvent(unsigned index);
};

#endif
