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

#include "GOrgueConfigReader.h"

#include "GOrgueConfigReaderDB.h"
#include "GOrgueUtil.h"
#include <wx/intl.h>
#include <wx/log.h>

GOrgueConfigReader::GOrgueConfigReader(GOrgueConfigReaderDB& cfg, bool strict) :
	m_Strict(strict),
	m_Config(cfg)
{
}

bool GOrgueConfigReader::Read(GOSettingType type, wxString group, wxString key, bool required, wxString& value)
{
	bool found = false;

	if (required)
	{
		if (group.length() >= 6 && group.Mid(0, 6) == wxT("Setter"))	// Setter groups aren't required.
			required = false;
		else if (group.length() >= 5 && group.Mid(0, 5) == wxT("Panel"))
		{
			if (group.length() >= 14 && group.Mid(8, 6) == wxT("Setter"))	// Setter groups aren't required.
				required = false;
		}
		else if (group.length() >= 12 && group.Mid(0, 12) == wxT("FrameGeneral"))	// FrameGeneral groups aren't required.
			required = false;
	}

	found = m_Config.GetString(type, group, key, value);

	if (!found)
	{
		if (required)
		{
			wxString error;
			error.Printf(_("Missing required value section '%s' entry '%s'"), group.c_str(), key.c_str());
			throw error;
		}
		else
			return false;
	}
	return true;
}

wxString GOrgueConfigReader::ReadString(GOSettingType type, wxString group, wxString key, bool required, wxString defaultValue)
{
	wxString value;
	bool found = Read(type, group, key, required, value);

	if (!found)
			value = defaultValue;
	return value;
}

wxString GOrgueConfigReader::ReadString(GOSettingType type, wxString group, wxString key, bool required)
{
	return ReadString(type, group, key, required, wxT(""));
}

wxString GOrgueConfigReader::ReadStringTrim(GOSettingType type, wxString group, wxString key, bool required, wxString defaultValue)
{
	wxString value = ReadString(type, group, key, required, defaultValue);

	if (value.length() > 0 && value[value.length() - 1] == ' ')
	{
		if (m_Strict)
			wxLogWarning(_("Trailing whitespace at section '%s' entry '%s': %s"), group.c_str(), key.c_str(), value.c_str());
		value.Trim();
	}
	return value;
}

wxString GOrgueConfigReader::ReadStringTrim(GOSettingType type, wxString group, wxString key, bool required)
{
	return ReadStringTrim(type, group, key, required, wxT(""));
}

wxString GOrgueConfigReader::ReadStringNotEmpty(GOSettingType type, wxString group, wxString key, bool required, wxString defaultValue)
{
	wxString value = ReadString(type, group, key, required, defaultValue);

	if (value.Trim().length() == 0 && m_Strict)
	{
		wxLogWarning(_("Empty at section '%s' entry '%s'"), group.c_str(), key.c_str());
	}
	return value;
}

wxString GOrgueConfigReader::ReadStringNotEmpty(GOSettingType type, wxString group, wxString key, bool required)
{
	return ReadStringNotEmpty(type, group, key, required, wxT(""));
}

bool GOrgueConfigReader::ReadBoolean(GOSettingType type, wxString group, wxString key, bool required)
{
	return ReadBoolean(type, group, key, required, false);
}

bool GOrgueConfigReader::ReadBoolean(GOSettingType type, wxString group, wxString key, bool required, bool defaultValue)
{
	wxString value;
	if (!Read(type, group, key, required, value))
		return defaultValue;

	if (value.length() > 0 && value[value.length() - 1] == ' ')
	{
		if (m_Strict)
			wxLogWarning(_("Trailing whitespace at section '%s' entry '%s': %s"), group.c_str(), key.c_str(), value.c_str());
		value.Trim();
	}
	if (value == wxT("Y") || value == wxT("y"))
		return true;
	if (value == wxT("N") || value == wxT("n"))
		return false;
	value.MakeUpper();
	wxLogWarning(_("Strange boolean value for section '%s' entry '%s': %s"), group.c_str(), key.c_str(), value.c_str());
	if (value.Length() && value[0] == wxT('Y'))
		return true;
	else if (value.Length() && value[0] == wxT('N'))
		return false;

	wxString error;
	error.Printf(_("Invalid boolean value at section '%s' entry '%s': %s"), group.c_str(), key.c_str(), value.c_str());
	throw error;
}

wxColour GOrgueConfigReader::ReadColor(GOSettingType type, wxString group, wxString key, bool required)
{
	return ReadColor(type, group, key, required, wxT("BLACK"));
}

wxColour GOrgueConfigReader::ReadColor(GOSettingType type, wxString group, wxString key, bool required, wxString defaultValue)
{
	wxString value;
	if (!Read(type, group, key, required, value))
		value = defaultValue;

	if (value.length() > 0 && value[value.length() - 1] == ' ')
	{
		if (m_Strict)
			wxLogWarning(_("Trailing whitespace at section '%s' entry '%s': %s"), group.c_str(), key.c_str(), value.c_str());
		value.Trim();
	}
	value.MakeUpper();

	if (value == wxT("BLACK"))
		return wxColour(0x00, 0x00, 0x00);
	else if (value == wxT("BLUE"))
		return wxColour(0x00, 0x00, 0xFF);
	else if (value == wxT("DARK BLUE"))
		return wxColour(0x00, 0x00, 0x80);
	else if (value == wxT("GREEN"))
		return wxColour(0x00, 0xFF, 0x00);
	else if (value == wxT("DARK GREEN"))
		return wxColour(0x00, 0x80, 0x00);
	else if (value == wxT("CYAN"))
		return wxColour(0x00, 0xFF, 0xFF);
	else if (value == wxT("DARK CYAN"))
		return wxColour(0x00, 0x80, 0x80);
	else if (value == wxT("RED"))
		return wxColour(0xFF, 0x00, 0x00);
	else if (value == wxT("DARK RED"))
		return wxColour(0x80, 0x00, 0x00);
	else if (value == wxT("MAGENTA"))
		return wxColour(0xFF, 0x00, 0xFF);
	else if (value == wxT("DARK MAGENTA"))
		return wxColour(0x80, 0x00, 0x80);
	else if (value == wxT("YELLOW"))
		return wxColour(0xFF, 0xFF, 0x00);
	else if (value == wxT("DARK YELLOW"))
		return wxColour(0x80, 0x80, 0x00);
	else if (value == wxT("LIGHT GREY"))
		return wxColour(0xC0, 0xC0, 0xC0);
	else if (value == wxT("DARK GREY"))
		return wxColour(0x80, 0x80, 0x80);
	else if (value == wxT("WHITE"))
		return wxColour(0xFF, 0xFF, 0xFF);
	else if (value == wxT("BROWN"))
		return wxColour(0xA5, 0x2A, 0x2A);

	wxColour colour;
	if (parseColor(colour, value))
		return colour;
	
	wxString error;
	error.Printf(_("Invalid color at section '%s' entry '%s': %s"), group.c_str(), key.c_str(), value.c_str());
	throw error;
}

int GOrgueConfigReader::ReadInteger(GOSettingType type, wxString group, wxString key, int nmin, int nmax, bool required)
{
	return ReadInteger(type, group, key, nmin, nmax, required, nmin);
}

int GOrgueConfigReader::ReadInteger(GOSettingType type, wxString group, wxString key, int nmin, int nmax, bool required, int defaultValue)
{
	wxString value;
	if (!Read(type, group, key, required, value))
		return defaultValue;

	long retval;
	if (value.length() > 0 && value[value.length() - 1] == ' ')
	{
		if (m_Strict)
			wxLogWarning(_("Trailing whitespace at section '%s' entry '%s': %s"), group.c_str(), key.c_str(), value.c_str());
		value.Trim();
	}
	if (!parseLong(retval, value))
	{
		if (value.Length() && !::wxIsdigit(value[0]) && value[0] != wxT('+') && value[0] != wxT('-') && value.CmpNoCase(wxT("none")) && !value.IsEmpty())
		{
			wxString error;
			error.Printf(_("Invalid integer value at section '%s' entry '%s': %s"), group.c_str(), key.c_str(), value.c_str());
			throw error;
		}

		retval = wxAtoi(value);
		wxLogWarning(_("Invalid integer value at section '%s' entry '%s': %s"), group.c_str(), key.c_str(), value.c_str());
	}

	if (nmin <= retval && retval <= nmax)
		return retval;

	wxString error;
	error.Printf(_("Out of range value at section '%s' entry '%s': %d"), group.c_str(), key.c_str(), retval);
	throw error;
}

int GOrgueConfigReader::ReadLong(GOSettingType type, wxString group, wxString key, int nmin, int nmax, bool required)
{
	return ReadInteger(type, group, key, nmin, nmax, required);
}

int GOrgueConfigReader::ReadLong(GOSettingType type, wxString group, wxString key, int nmin, int nmax, bool required, int defaultValue)
{
	return ReadInteger(type, group, key, nmin, nmax, required, defaultValue);
}

double GOrgueConfigReader::ReadFloat(GOSettingType type, wxString group, wxString key, double nmin, double nmax, bool required)
{
	return ReadFloat(type, group, key, nmin, nmax, required, nmin);
}

double GOrgueConfigReader::ReadFloat(GOSettingType type, wxString group, wxString key, double nmin, double nmax, bool required, double defaultValue)
{
	wxString value;
	if (!Read(type, group, key, required, value))
		return defaultValue;

	if (value.length() > 0 && value[value.length() - 1] == ' ')
	{
		if (m_Strict)
			wxLogWarning(_("Trailing whitespace at section '%s' entry '%s': %s"), group.c_str(), key.c_str(), value.c_str());
		value.Trim();
	}
	double retval;
	int pos = value.find(wxT(","), 0);
	if (pos >= 0)
	{
		wxLogWarning(_("Number %s contains locale dependent floating point"), value.c_str());
		value[pos] = wxT('.');
	}
	if (!parseCDouble(retval, value))
	{
		wxString error;
		error.Printf(_("Invalid float value at section '%s' entry '%s': %s"), group.c_str(), key.c_str(), value.c_str());
		throw error;
	}

	if (nmin <= retval && retval <= nmax)
		return retval;

	wxString error;
	error.Printf(_("Out of range value at section '%s' entry '%s': %f"), group.c_str(), key.c_str(), retval);
	throw error;
}

unsigned GOrgueConfigReader::ReadSize(GOSettingType type, wxString group, wxString key, unsigned size_type, bool required)
{
	return ReadSize(type, group, key, size_type, required, wxT("SMALL"));
}

unsigned GOrgueConfigReader::ReadSize(GOSettingType type, wxString group, wxString key, unsigned size_type, bool required, wxString defaultValue)
{
	static const int sizes[2][4] = {{800, 1007, 1263, 1583}, {500, 663, 855, 1095}};
	wxString value;

	if (!Read(type, group, key, required, value))
		value = defaultValue;

	if (value.length() > 0 && value[value.length() - 1] == ' ')
	{
		if (m_Strict)
			wxLogWarning(_("Trailing whitespace at section '%s' entry '%s': %s"), group.c_str(), key.c_str(), value.c_str());
		value.Trim();
	}
	value.MakeUpper();

	if (value == wxT("SMALL"))
		return sizes[size_type][0];
	else if (value == wxT("MEDIUM"))
		return sizes[size_type][1];
	else if (value == wxT("MEDIUM LARGE"))
		return sizes[size_type][2];
	else if (value == wxT("LARGE"))
		return sizes[size_type][3];

	long size;
	if (parseLong(size, value))
		if (100 <= size && size <= 4000)
			return size;
	
	wxString error;
	error.Printf(_("Invalid size at section '%s' entry '%s': %s"), group.c_str(), key.c_str(), value.c_str());
	throw error;
}

unsigned GOrgueConfigReader::ReadFontSize(GOSettingType type, wxString group, wxString key, bool required)
{
	return ReadFontSize(type, group, key, required, wxT("NORMAL"));
}

unsigned GOrgueConfigReader::ReadFontSize(GOSettingType type, wxString group, wxString key, bool required, wxString defaultValue)
{
	wxString value;
	if (!Read(type, group, key, required, value))
		value = defaultValue;

	if (value.length() > 0 && value[value.length() - 1] == ' ')
	{
		if (m_Strict)
			wxLogWarning(_("Trailing whitespace at section '%s' entry '%s': %s"), group.c_str(), key.c_str(), value.c_str());
		value.Trim();
	}
	value.MakeUpper();

	if (value == wxT("SMALL"))
		return 6;
	else if (value == wxT("NORMAL"))
		return 7;
	else if (value == wxT("LARGE"))
		return 10;

	long size;
	if (parseLong(size, value))
		if (1 <= size && size <= 50)
			return size;

	wxString error;
	error.Printf(_("Invalid font size at section '%s' entry '%s': %s"), group.c_str(), key.c_str(), value.c_str());
	throw error;
}

int GOrgueConfigReader::ReadEnum(GOSettingType type, wxString group, wxString key, const struct IniFileEnumEntry* entry, unsigned count, bool required, int defaultValue)
{
	int defaultEntry = -1;
	wxString value;
	for (unsigned i = 0; i < count; i++)
		if (entry[i].value == defaultValue)
			defaultEntry = i;
	if (defaultEntry == -1)
	{
		wxLogError(_("Invalid enum default value"));
		defaultEntry = 0;
	}

	if (!Read(type, group, key, required, value))
	    return entry[defaultEntry].value;

	if (value.length() > 0 && value[value.length() - 1] == ' ')
	{
		if (m_Strict)
			wxLogWarning(_("Trailing whitespace at section '%s' entry '%s': %s"), group.c_str(), key.c_str(), value.c_str());
		value.Trim();
	}
	for (unsigned i = 0; i < count; i++)
		if (entry[i].name == value)
			return entry[i].value;

	wxString error;
	error.Printf(_("Invalid enum value at section '%s' entry '%s': %s"), group.c_str(), key.c_str(), value.c_str());
	throw error;
}

int GOrgueConfigReader::ReadEnum(GOSettingType type, wxString group, wxString key, const struct IniFileEnumEntry* entry, unsigned count, bool required)
{
	return ReadEnum(type, group, key, entry, count, required, entry[0].value);
}
