/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiWxEvent.h"

DEFINE_LOCAL_EVENT_TYPE(wxEVT_MIDI_ACTION)

IMPLEMENT_DYNAMIC_CLASS(GOMidiWxEvent, wxEvent)

GOMidiWxEvent::GOMidiWxEvent(int id, wxEventType type)
  : wxEvent(id, type), m_midi() {}

GOMidiWxEvent::GOMidiWxEvent(const GOMidiEvent &e, int id, wxEventType type)
  : wxEvent(id, type), m_midi(e) {}

GOMidiWxEvent::GOMidiWxEvent(const GOMidiWxEvent &e)
  : wxEvent(e), m_midi(e.GetMidiEvent()) {}

wxEvent *GOMidiWxEvent::Clone() const { return new GOMidiWxEvent(*this); }
