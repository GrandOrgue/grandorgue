/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIWXEVENT_H
#define GOMIDIWXEVENT_H

#include <wx/event.h>

#include "GOMidiEvent.h"

DECLARE_LOCAL_EVENT_TYPE(wxEVT_MIDI_ACTION, -1)

class wxMidiEvent : public wxEvent {
private:
  GOMidiEvent m_midi;

public:
  wxMidiEvent(int id = 0, wxEventType type = wxEVT_MIDI_ACTION);
  wxMidiEvent(
    const GOMidiEvent &e, int id = 0, wxEventType type = wxEVT_MIDI_ACTION);
  wxMidiEvent(const wxMidiEvent &e);

  void SetMidiEvent(const GOMidiEvent &e) { m_midi = e; }

  const GOMidiEvent &GetMidiEvent() const { return m_midi; }

  wxEvent *Clone() const;

  DECLARE_DYNAMIC_CLASS(wxMidiEvent)
};

typedef void (wxEvtHandler::*wxMidiEventFunction)(wxMidiEvent &);

#define EVT_MIDI(fn)                                                           \
  DECLARE_EVENT_TABLE_ENTRY(                                                   \
    wxEVT_MIDI_ACTION,                                                         \
    wxID_ANY,                                                                  \
    wxID_ANY,                                                                  \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(                 \
      wxMidiEventFunction, &fn),                                               \
    (wxObject *)NULL),

#endif
