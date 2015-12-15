/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2015 GrandOrgue contributors (see AUTHORS)
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

#ifndef GORGUEMIDIEVENT_H
#define GORGUEMIDIEVENT_H

#include "GOrgueTime.h"
#include <vector>

class GOrgueMidiMap;

typedef enum {
	MIDI_NONE,
	MIDI_RESET,
	MIDI_NOTE,
	MIDI_AFTERTOUCH,
	MIDI_CTRL_CHANGE,
	MIDI_PGM_CHANGE,
	MIDI_RPN,
	MIDI_NRPN,
	MIDI_SYSEX_JOHANNUS,
	MIDI_SYSEX_VISCOUNT,
	MIDI_SYSEX_GO_CLEAR,
	MIDI_SYSEX_GO_SETUP,
	MIDI_SYSEX_HW_STRING,
	MIDI_SYSEX_HW_LCD,
} midi_message_type;

#define MIDI_CTRL_BANK_SELECT_MSB 0
#define MIDI_CTRL_BANK_SELECT_LSB 32
#define MIDI_CTRL_RPN_MSB 101
#define MIDI_CTRL_RPN_LSB 100
#define MIDI_CTRL_NRPN_MSB 99
#define MIDI_CTRL_NRPN_LSB 98
#define MIDI_CTRL_DATA_ENTRY 6
#define MIDI_CTRL_SOUNDS_OFF 120
#define MIDI_CTRL_NOTES_OFF 123

class GOrgueMidiEvent {
private:
	midi_message_type m_miditype;
	int m_channel, m_key, m_value;
	unsigned m_device;
	GOTime m_time;

public:
	GOrgueMidiEvent();
	GOrgueMidiEvent(const GOrgueMidiEvent& e);

	midi_message_type GetMidiType() const
	{
		return m_miditype;
	}
	void SetMidiType(midi_message_type t)
	{
		m_miditype = t;
	}

	void SetDevice(unsigned dev)
	{
		m_device = dev;
	}

	unsigned GetDevice() const
	{
		return m_device;
	}	

	int GetChannel() const
	{
		return m_channel;
	}
	void SetChannel(int ch)
	{
		m_channel = ch;
	}

	int GetKey() const
	{
		return m_key;
	}
	void SetKey(int k)
	{
		m_key = k;
	}

	int GetValue() const
	{
		return m_value;
	}
	void SetValue(int v)
	{
		m_value = v;
	}

	GOTime GetTime() const
	{
		return m_time;
	}
	void SetTime(GOTime t)
	{
		m_time = t;
	}

	void FromMidi(const std::vector<unsigned char>& msg, GOrgueMidiMap& map);
	void ToMidi(std::vector<std::vector<unsigned char>>& msg, GOrgueMidiMap& map) const;

	wxString ToString(GOrgueMidiMap& map) const;
};

#endif
