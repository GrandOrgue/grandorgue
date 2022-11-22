/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIRECEIVERDATA_H
#define GOMIDIRECEIVERDATA_H

#include <vector>

#include "GOMidiReceiverEventPattern.h"
#include "GOTime.h"

typedef enum {
  MIDI_RECV_DRAWSTOP,
  MIDI_RECV_BUTTON,
  MIDI_RECV_ENCLOSURE,
  MIDI_RECV_MANUAL,
  MIDI_RECV_SETTER,
  MIDI_RECV_ORGAN,
} GOMidiReceiverType;

typedef enum {
  MIDI_MATCH_NONE,
  MIDI_MATCH_ON,
  MIDI_MATCH_OFF,
  MIDI_MATCH_CHANGE,
  MIDI_MATCH_RESET,
} GOMidiMatchType;

class GOMidiReceiverData {
protected:
  GOMidiReceiverType m_type;
  std::vector<GOMidiReceiverEvent> m_events;

public:
  GOMidiReceiverData(GOMidiReceiverType type);
  virtual ~GOMidiReceiverData();

  GOMidiReceiverType GetType() const;

  unsigned GetEventCount() const;
  GOMidiReceiverEvent &GetEvent(unsigned index);
  unsigned AddNewEvent();
  void DeleteEvent(unsigned index);
};

#endif
