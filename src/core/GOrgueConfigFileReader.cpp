/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
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

#include "GOrgueConfigFileReader.h"

#include "GOrgueBuffer.h"
#include "GOrgueCompress.h"
#include "GOrgueHash.h"
#include "GOrgueStandardFile.h"
#include <wx/file.h>
#include <wx/intl.h>
#include <wx/log.h>

GOrgueConfigFileReader::GOrgueConfigFileReader() :
	m_Entries(),
	m_Hash()
{
}

GOrgueConfigFileReader::~GOrgueConfigFileReader()
{
}

wxString GOrgueConfigFileReader::GetHash()
{
	return m_Hash;
}

const std::map<wxString, std::map<wxString, wxString> >& GOrgueConfigFileReader::GetContent()
{
	return m_Entries;
}

wxString GOrgueConfigFileReader::GetNextLine(const wxString& buffer, unsigned &pos)
{
	int newpos = buffer.find(wxT("\n"), pos);
	if (newpos < (int)pos)
		newpos = buffer.Len();
	wxString line = buffer.Mid(pos, newpos - pos);
	pos = newpos + 1;
	if (line.Len() > 0 && line[line.Len() - 1] == wxT('\r'))
		return line.Mid(0, line.Len() - 1);
	return line;
}

wxString GOrgueConfigFileReader::getEntry(wxString group, wxString name)
{
	std::map<wxString, std::map<wxString, wxString> >::const_iterator i = m_Entries.find(group);
	if (i == m_Entries.end())
		return wxEmptyString;
	const std::map<wxString, wxString>& g = i->second;
	std::map<wxString, wxString>::const_iterator j = g.find(name);
	if (j == g.end())
		return wxEmptyString;
	else
		return j->second;
}

bool GOrgueConfigFileReader::Read(wxString filename)
{
	GOrgueStandardFile file(filename);
	return Read(&file);
}

bool GOrgueConfigFileReader::Read(GOrgueFile* file)
{
	m_Entries.clear();
	
	if (!file->Open())
	{
		wxLogError(_("Failed to open file '%s'"), file->GetName().c_str());
		return false;
	}
	GOrgueBuffer<uint8_t> data;
	try
	{
		data.resize(file->GetSize());
	}
	catch (GOrgueOutOfMemory e)
	{
		wxLogError(_("Failed to load file '%s' into the memory"), file->GetName().c_str());
		file->Close();
		return false;
	}
	if (!file->Read(data))
	{
		file->Close();
		wxLogError(_("Failed to read file '%s'"), file->GetName().c_str());
		return false;
	}
	file->Close();
	GOrgueHash hash;
	hash.Update(data.get(), data.GetSize());
	m_Hash = hash.getStringHash();

	if (isBufferCompressed(data))
	{
		if (!uncompressBuffer(data))
		{
			wxLogError(_("Failed to decompress file '%s'"), file->GetName().c_str());
			return false;
		}
	}

	wxMBConv* conv;
	wxCSConv isoConv(wxT("ISO-8859-1"));
	uint8_t* dataPtr = data.get();
	size_t length = data.GetCount();
	if (length >= 3 && data[0] == 0xEF && data[1] == 0xBB && data[2] == 0xBF)
	{
		conv = &wxConvUTF8;
		dataPtr = data.get() + 3;
		length -= 3;
	}
	else
		conv = &isoConv;
	wxString input((const char*)dataPtr, *conv, length);
	data.free();
	if (length && input.Len() == 0)
	{
		wxLogError(_("Failed to decode file '%s'"), file->GetName().c_str());
		return false;
	}
	
	unsigned pos = 0;
	wxString group;
	std::map<wxString, wxString>* grp = NULL;
	unsigned lineno = 0;
	
	while(pos < input.Len())
	{
		wxString line = GetNextLine(input, pos);
		lineno++;
		
		if (line == wxEmptyString)
			continue;
		if (line.Len() >= 1 && line[0] == wxT(';'))
			continue;
		if (line.Len() > 1 && line[0] == wxT('['))
		{
			if (line[line.Len() - 1] != wxT(']'))
			{
				line = line.Trim();
				if (line[line.Len() - 1] != wxT(']'))
				{
					wxLogError(_("Invalid Config entry at line %d: %s"), lineno, line.c_str());
					continue;
				}
				wxLogError(_("Invalid section start at line %d: %s"), lineno, line.c_str());
			}
			group = line.Mid(1, line.Len() - 2);
			if (m_Entries.find(group) != m_Entries.end())
			{
				wxLogWarning(_("Duplicate group at line %d: %s"), lineno, group.c_str());
			}
			grp = &m_Entries[group];
		}
		else
		{
			if (!grp)
			{
				wxLogError(_("Config entry without any group at line %d"), lineno);
				continue;
			}
			int datapos = line.find(wxT("="), 0);
			if (datapos <= 0)
			{
				wxLogError(_("Invalid Config entry at line %d: %s"), lineno, line.c_str());
				continue;
			}
			wxString name = line.Mid(0, datapos);
			if (grp->find(name) != grp->end())
			{
				wxLogWarning(_("Duplicate entry in section %s at line %d: %s"), group.c_str(), lineno, name.c_str());
			}
			(*grp)[name] = line.Mid(datapos + 1);
		}
	}

	return true;
}
