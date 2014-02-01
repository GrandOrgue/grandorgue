/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2014 GrandOrgue contributors (see AUTHORS)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "GOrgueMidiWXEvent.h"

DEFINE_LOCAL_EVENT_TYPE( wxEVT_MIDI_ACTION )

IMPLEMENT_DYNAMIC_CLASS( wxMidiEvent, wxEvent )

wxMidiEvent::wxMidiEvent(int id, wxEventType type) :
	wxEvent(id, type),
	m_midi()
{
}

wxMidiEvent::wxMidiEvent(const GOrgueMidiEvent& e, int id, wxEventType type) :
	wxEvent(id, type),
	m_midi(e)
{
}

wxMidiEvent::wxMidiEvent(const wxMidiEvent& e) :
	wxEvent(e),
	m_midi(e.GetMidiEvent())
{
}

wxEvent* wxMidiEvent::Clone() const
{
	return new wxMidiEvent(*this);
}



