/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDISENDERDATA_H
#define GOMIDISENDERDATA_H

#include <vector>

#include "GOMidiSenderEventPattern.h"

typedef enum {
  MIDI_SEND_BUTTON,
  MIDI_SEND_LABEL,
  MIDI_SEND_ENCLOSURE,
  MIDI_SEND_MANUAL,
} GOMidiSenderType;

class GOMidiSenderData {
protected:
  GOMidiSenderType m_type;
  std::vector<GOMidiSenderEvent> m_events;

public:
  GOMidiSenderData(GOMidiSenderType type);
  virtual ~GOMidiSenderData();

  GOMidiSenderType GetType() const;

  unsigned GetEventCount() const;
  GOMidiSenderEvent &GetEvent(unsigned index);
  unsigned AddNewEvent();
  void DeleteEvent(unsigned index);
};

#endif
