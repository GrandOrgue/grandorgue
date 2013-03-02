/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2013 GrandOrgue contributors (see AUTHORS)
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

#ifndef GORGUEMIDIRECEIVER_H
#define GORGUEMIDIRECEIVER_H

#include <wx/wx.h>
#include <vector>
#include "GOrgueMidiEvent.h"

class GOrgueConfigReader;
class GOrgueConfigWriter;
class GOrgueSettings;
class GrandOrgueFile;
struct IniFileEnumEntry;

typedef enum {
	MIDI_RECV_DRAWSTOP,
	MIDI_RECV_BUTTON,
	MIDI_RECV_ENCLOSURE,
	MIDI_RECV_MANUAL,
	MIDI_RECV_SETTER,
} MIDI_RECEIVER_TYPE;

typedef enum {
	MIDI_MATCH_NONE,
	MIDI_MATCH_ON,
	MIDI_MATCH_OFF,
	MIDI_MATCH_CHANGE,
	MIDI_MATCH_RESET,
} MIDI_MATCH_TYPE;

typedef enum {
	MIDI_M_NONE = MIDI_NONE,
	MIDI_M_RESET = MIDI_RESET,
	MIDI_M_NOTE = MIDI_NOTE,
	MIDI_M_CTRL_CHANGE = MIDI_CTRL_CHANGE,
	MIDI_M_PGM_CHANGE = MIDI_PGM_CHANGE,
	MIDI_M_NOTE_ON,
	MIDI_M_NOTE_OFF,
	MIDI_M_CTRL_CHANGE_ON,
	MIDI_M_CTRL_CHANGE_OFF,
	MIDI_M_NOTE_NO_VELOCITY,
	MIDI_M_NOTE_SHORT_OCTAVE,
} midi_match_message_type;

typedef struct {
	wxString device;
	midi_match_message_type type;
	int channel;
	int key;
	int low_key;
	int high_key;
	int low_velocity;
	int high_velocity;
} MIDI_MATCH_EVENT;

class GOrgueMidiReceiver {
private:
	GrandOrgueFile* m_organfile;
	static const struct IniFileEnumEntry m_MidiTypes[];
	MIDI_RECEIVER_TYPE m_type;
	int m_Index;
	std::vector<MIDI_MATCH_EVENT> m_events;

public:
	GOrgueMidiReceiver(GrandOrgueFile* organfile, MIDI_RECEIVER_TYPE type);

	void Load(GOrgueConfigReader& cfg, wxString group);
	void Save(GOrgueConfigWriter& cfg, wxString group);

	MIDI_RECEIVER_TYPE GetType() const;

	void SetIndex(int index);

	MIDI_MATCH_TYPE Match(const GOrgueMidiEvent& e);
 	MIDI_MATCH_TYPE Match(const GOrgueMidiEvent& e, int& value);
 	MIDI_MATCH_TYPE Match(const GOrgueMidiEvent& e, const unsigned midi_map[128], int& key, int& value);

	unsigned GetEventCount() const;
	MIDI_MATCH_EVENT& GetEvent(unsigned index);
	unsigned AddNewEvent();
	void DeleteEvent(unsigned index);

	GOrgueSettings& GetSettings();
	GrandOrgueFile* GetOrganfile();
};

#endif
