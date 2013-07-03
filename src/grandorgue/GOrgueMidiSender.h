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

#include "GOrgueMidiSenderData.h"

class GOrgueConfigReader;
class GOrgueConfigWriter;
class GOrgueSettings;
class GrandOrgueFile;
struct IniFileEnumEntry;

class GOrgueMidiSender : public GOrgueMidiSenderData
{
private:
	static const struct IniFileEnumEntry m_MidiTypes[];
	GrandOrgueFile* m_organfile;

public:
	GOrgueMidiSender(GrandOrgueFile* organfile, MIDI_SENDER_TYPE type);
	~GOrgueMidiSender();

	void Load(GOrgueConfigReader& cfg, wxString group);
	void Save(GOrgueConfigWriter& cfg, wxString group);

	void SetDisplay(bool state);
	void SetKey(unsigned key, bool state);
	void SetValue(unsigned value);
	void SetLabel(wxString text);

	void Assign(const GOrgueMidiSenderData& data);

	GOrgueSettings& GetSettings();
};

#endif
