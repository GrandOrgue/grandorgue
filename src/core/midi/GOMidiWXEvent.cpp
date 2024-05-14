/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiWXEvent.h"

DEFINE_LOCAL_EVENT_TYPE(wxEVT_MIDI_ACTION)

IMPLEMENT_DYNAMIC_CLASS(wxMidiEvent, wxEvent)

wxMidiEvent::wxMidiEvent(int id, wxEventType type)
  : wxEvent(id, type), m_midi() {}

wxMidiEvent::wxMidiEvent(const GOMidiEvent &e, int id, wxEventType type)
  : wxEvent(id, type), m_midi(e) {}

wxMidiEvent::wxMidiEvent(const wxMidiEvent &e)
  : wxEvent(e), m_midi(e.GetMidiEvent()) {}

wxEvent *wxMidiEvent::Clone() const { return new wxMidiEvent(*this); }
