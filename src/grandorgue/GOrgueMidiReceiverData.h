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

#ifndef GORGUEMIDIRECEIVERDATA_H
#define GORGUEMIDIRECEIVERDATA_H

#include "GOrgueTime.h"
#include <vector>

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
	MIDI_M_SYSEX_JOHANNUS,
	MIDI_M_CTRL_BIT,
	MIDI_M_RPN,
	MIDI_M_NRPN,
	MIDI_M_NOTE_ON,
	MIDI_M_NOTE_OFF,
	MIDI_M_CTRL_CHANGE_ON,
	MIDI_M_CTRL_CHANGE_OFF,
	MIDI_M_RPN_ON,
	MIDI_M_RPN_OFF,
	MIDI_M_NRPN_ON,
	MIDI_M_NRPN_OFF,
	MIDI_M_NOTE_NO_VELOCITY,
	MIDI_M_NOTE_SHORT_OCTAVE,
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

class GOrgueMidiReceiverData
{
protected:
	MIDI_RECEIVER_TYPE m_type;
	std::vector<MIDI_MATCH_EVENT> m_events;

public:
	GOrgueMidiReceiverData(MIDI_RECEIVER_TYPE type);
	virtual ~GOrgueMidiReceiverData();

	MIDI_RECEIVER_TYPE GetType() const;

	unsigned GetEventCount() const;
	MIDI_MATCH_EVENT& GetEvent(unsigned index);
	unsigned AddNewEvent();
	void DeleteEvent(unsigned index);
};

#endif
