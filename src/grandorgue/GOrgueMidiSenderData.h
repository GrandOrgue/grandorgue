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

#ifndef GORGUEMIDISENDERDATA_H
#define GORGUEMIDISENDERDATA_H

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
} midi_send_message_type;

typedef struct {
	unsigned device;
	midi_send_message_type type;
	unsigned channel;
	unsigned key;
	unsigned low_value;
	unsigned high_value;
} MIDI_SEND_EVENT;

class GOrgueMidiSenderData
{
protected:
	MIDI_SENDER_TYPE m_type;
	std::vector<MIDI_SEND_EVENT> m_events;

public:
	GOrgueMidiSenderData(MIDI_SENDER_TYPE type);
	virtual ~GOrgueMidiSenderData();

	MIDI_SENDER_TYPE GetType() const;

	unsigned GetEventCount() const;
	MIDI_SEND_EVENT& GetEvent(unsigned index);
	unsigned AddNewEvent();
	void DeleteEvent(unsigned index);
};

#endif
