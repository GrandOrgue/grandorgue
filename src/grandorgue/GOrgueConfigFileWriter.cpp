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

#include "GOrgueConfigFileWriter.h"

#include <wx/file.h>

GOrgueConfigFileWriter::GOrgueConfigFileWriter() :
	m_Entries()
{
}
	
void GOrgueConfigFileWriter::AddEntry(wxString group, wxString name, wxString value)
{
	std::map<wxString, wxString>& g = m_Entries[group];
	assert(g.find(name) == g.end());
	g[name] = value;
}

bool GOrgueConfigFileWriter::Save(wxString filename)
{
	wxFile out;
	wxCSConv conv(wxT("ISO-8859-1"));

	if (!out.Create(filename, true))
		return false;

	for(std::map<wxString, std::map<wxString, wxString> >::const_iterator i = m_Entries.begin(); i != m_Entries.end(); i++)
	{
		const std::map<wxString, wxString>& g = i->second;
		const wxString group = i->first;
		
		if (!out.Write(wxT("[") + group + wxT("]\n"), conv))
			return false;

		for(std::map<wxString, wxString>::const_iterator j = g.begin(); j != g.end(); j++)
		{
			const wxString& name = j->first;
			const wxString& value = j->second;
			if (!out.Write(name + wxT("=") + value + wxT("\n"), conv))
				return false;
		}
	}
	out.Flush();
	out.Close();
	return true;
}
