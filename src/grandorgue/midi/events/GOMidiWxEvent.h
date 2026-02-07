/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIWXEVENT_H
#define GOMIDIWXEVENT_H

#include <wx/event.h>

#include "GOMidiEvent.h"

DECLARE_LOCAL_EVENT_TYPE(wxEVT_MIDI_ACTION, -1)

/**
 * This class is a wrapper around GOMidiEvent. The main purpose of this class
 * is queing message event to the application event queue among other GUI
 * events. So this class inherits wxEvent
 */

class GOMidiWxEvent : public wxEvent {
private:
  GOMidiEvent m_midi;

public:
  GOMidiWxEvent(int id = 0, wxEventType type = wxEVT_MIDI_ACTION);
  GOMidiWxEvent(
    const GOMidiEvent &e, int id = 0, wxEventType type = wxEVT_MIDI_ACTION);
  GOMidiWxEvent(const GOMidiWxEvent &e);

  void SetMidiEvent(const GOMidiEvent &e) { m_midi = e; }

  const GOMidiEvent &GetMidiEvent() const { return m_midi; }

  wxEvent *Clone() const;

  DECLARE_DYNAMIC_CLASS(GOMidiWxEvent)
};

typedef void (wxEvtHandler::*wxMidiEventFunction)(GOMidiWxEvent &);

#define EVT_MIDI(fn)                                                           \
  DECLARE_EVENT_TABLE_ENTRY(                                                   \
    wxEVT_MIDI_ACTION,                                                         \
    wxID_ANY,                                                                  \
    wxID_ANY,                                                                  \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(                 \
      wxMidiEventFunction, &fn),                                               \
    (wxObject *)NULL),

#endif
