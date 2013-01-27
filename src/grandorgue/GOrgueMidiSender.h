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

#ifndef GORGUEMIDISENDER_H
#define GORGUEMIDISENDER_H

#include <wx/wx.h>
#include <vector>
#include "GOrgueMidiEvent.h"

class GOrgueConfigReader;
class GOrgueConfigWriter;
class GOrgueSettings;
class GrandOrgueFile;
struct IniFileEnumEntry;

typedef enum {
	MIDI_SEND_BUTTON,
	MIDI_SEND_LABEL,
	MIDI_SEND_ENCLOSURE,
	MIDI_SEND_MANUAL,
} MIDI_SENDER_TYPE;

typedef enum {
	MIDI_S_NONE,
	MIDI_S_NOTE,
	MIDI_S_CTRL,
	MIDI_S_PGM_ON,
	MIDI_S_PGM_OFF,
	MIDI_S_NOTE_ON,
	MIDI_S_NOTE_OFF,
	MIDI_S_CTRL_ON,
	MIDI_S_CTRL_OFF,
} midi_send_message_type;

typedef struct {
	wxString device;
	midi_send_message_type type;
	unsigned channel;
	unsigned key;
	unsigned low_value;
	unsigned high_value;
} MIDI_SEND_EVENT;

class GOrgueMidiSender
{
private:
	static const struct IniFileEnumEntry m_MidiTypes[];
	GrandOrgueFile* m_organfile;
	MIDI_SENDER_TYPE m_type;
	std::vector<MIDI_SEND_EVENT> m_events;

public:
	GOrgueMidiSender(GrandOrgueFile* organfile, MIDI_SENDER_TYPE type);
	~GOrgueMidiSender();

	MIDI_SENDER_TYPE GetType() const;

	void Load(GOrgueConfigReader& cfg, wxString group);
	void Save(GOrgueConfigWriter& cfg, wxString group);

	void SetDisplay(bool state);
	void SetKey(unsigned key, bool state);
	void SetValue(unsigned value);
	void SetLabel(wxString text);

	unsigned GetEventCount() const;
	MIDI_SEND_EVENT& GetEvent(unsigned index);
	unsigned AddNewEvent();
	void DeleteEvent(unsigned index);

	GOrgueSettings& GetSettings();
};

#endif
