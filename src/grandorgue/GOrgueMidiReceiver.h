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

#ifndef GORGUEMIDIRECEIVER_H
#define GORGUEMIDIRECEIVER_H

#include "GOrgueMidiReceiverData.h"

class GOrgueConfigReader;
class GOrgueConfigWriter;
class GOrgueMidiEvent;
class GOrgueMidiMap;
class GrandOrgueFile;
struct IniFileEnumEntry;

class GOrgueMidiReceiver : public GOrgueMidiReceiverData
{
private:
	GrandOrgueFile* m_organfile;
	static const struct IniFileEnumEntry m_MidiTypes[];
	int m_Index;
	std::vector<GOTime> m_last;

	MIDI_MATCH_TYPE debounce(const GOrgueMidiEvent& e, MIDI_MATCH_TYPE event, unsigned index);

public:
	GOrgueMidiReceiver(GrandOrgueFile* organfile, MIDI_RECEIVER_TYPE type);

	void Load(GOrgueConfigReader& cfg, wxString group, GOrgueMidiMap& map);
	void Save(GOrgueConfigWriter& cfg, wxString group, GOrgueMidiMap& map);

	void SetIndex(int index);

	MIDI_MATCH_TYPE Match(const GOrgueMidiEvent& e);
 	MIDI_MATCH_TYPE Match(const GOrgueMidiEvent& e, int& value);
 	MIDI_MATCH_TYPE Match(const GOrgueMidiEvent& e, const unsigned midi_map[128], int& key, int& value);

	bool HasDebounce(midi_match_message_type type);
	bool HasLowerLimit(midi_match_message_type type);
	bool HasUpperLimit(midi_match_message_type type);

	void Assign(const GOrgueMidiReceiverData& data);

	GrandOrgueFile* GetOrganfile();
};

#endif
