/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2018 GrandOrgue contributors (see AUTHORS)
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

#include "GOrgueBuffer.h"
#include <wx/file.h>
#include <wx/intl.h>
#include <wx/log.h>

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

bool GOrgueConfigFileWriter::GetFileContent(GOrgueBuffer<uint8_t>& buf)
{
	uint8_t bom[] = { 0xEF, 0xBB, 0xBF };
	wxString content = wxEmptyString;

	for(std::map<wxString, std::map<wxString, wxString> >::const_iterator i = m_Entries.begin(); i != m_Entries.end(); i++)
	{
		const std::map<wxString, wxString>& g = i->second;
		const wxString group = i->first;
		wxString groupContent = wxT("[") + group + wxT("]\n");
		
		for(std::map<wxString, wxString>::const_iterator j = g.begin(); j != g.end(); j++)
		{
			const wxString& name = j->first;
			const wxString& value = j->second;
			groupContent += name + wxT("=") + value + wxT("\n");
		}
		content += groupContent;
	}
	wxCharBuffer b = content.utf8_str();
	buf.free();
	buf.Append(bom, sizeof(bom));
	buf.Append((const uint8_t*)b.data(), b.length());

	return true;
}

bool GOrgueConfigFileWriter::Save(wxString filename)
{
	wxFile out;
	GOrgueBuffer<uint8_t> buf;
	if (!GetFileContent(buf))
		return false;

	if (!out.Create(filename, true))
		return false;

	if (!out.Write(buf.get(), buf.GetSize()))
		return false;
	out.Flush();
	out.Close();
	return true;
}
