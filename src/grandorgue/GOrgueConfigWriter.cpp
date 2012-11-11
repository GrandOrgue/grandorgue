/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2012 GrandOrgue contributors (see AUTHORS)
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

#include <wx/fileconf.h>
#include "GOrgueConfigReader.h"
#include "GOrgueConfigWriter.h"

GOrgueConfigWriter::GOrgueConfigWriter(wxFileConfig& cfg, bool prefix) :
	m_ConfigFile(cfg),
	m_Prefix(prefix)
{
}

void GOrgueConfigWriter::Write(wxString group, wxString key, wxString value)
{
	wxString str = group + wxT('/') + key;

	if (m_Prefix)
		m_ConfigFile.Write(wxT('_') + str, value);
	else
		m_ConfigFile.Write(str, value);
}

void GOrgueConfigWriter::Write(wxString group, wxString key, int value, bool sign, bool force)
{
	wxString str;
	if (force)
		str.Printf(wxT("%03d"), value);
	else if (sign)
	{
		if (value >= 0)
			str.Printf(wxT("+%03d"), value);
		else
			str.Printf(wxT("-%03d"), -value);
	}
	else
		str.Printf(wxT("%d"), value);
	Write(group, key, str);
}

void GOrgueConfigWriter::Write(wxString group, wxString key, float value)
{
	wxString str = wxString::Format(wxT("%f"), value);
	Write(group, key, str);
}

void GOrgueConfigWriter::Write(wxString group, wxString key, int value, const struct IniFileEnumEntry* entry, unsigned count)
{
	for (unsigned i = 0; i < count; i++)
		if (entry[i].value == value)
		{
			Write(group, key, entry[i].name);
			return;
		}
	wxLogError(_("Invalid enum value for /%s/%s: %d"), group.c_str(), key.c_str(), value);
}

void GOrgueConfigWriter::Write(wxString group, wxString key, bool value)
{
	wxString str = value ? wxT("Y") : wxT("N");
	Write(group, key, str);
}
