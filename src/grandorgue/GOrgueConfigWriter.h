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

#ifndef GORGUECONFIGWRITER_H
#define GORGUECONFIGWRITER_H

#include <wx/wx.h>

class GOrgueConfigFileWriter;

struct IniFileEnumEntry;

class GOrgueConfigWriter
{
private:
	GOrgueConfigFileWriter& m_ConfigFile;
	bool m_Prefix;

public:
	GOrgueConfigWriter(GOrgueConfigFileWriter& cfg, bool prefix);

	void Write(wxString group, wxString key, int value, bool sign = false, bool force = false);
	void Write(wxString group, wxString key, int value, const struct IniFileEnumEntry* entry, unsigned count);
	void Write(wxString group, wxString key, float value);
	void WriteString(wxString group, wxString key, wxString value);
	void WriteInteger(wxString group, wxString key, int value);
	void WriteEnum(wxString group, wxString key, int value, const struct IniFileEnumEntry* entry, unsigned count);
	void WriteFloat(wxString group, wxString key, float value);
	void WriteBoolean(wxString group, wxString key, bool value);
};

#endif
