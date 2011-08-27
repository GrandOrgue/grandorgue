/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */


#include "IniFileConfig.h"
#include <wx/fileconf.h>

IniFileConfig::IniFileConfig(wxFileConfig& odf_ini_file) :
	m_ODFIni (odf_ini_file)
{

}

bool IniFileConfig::ReadKey(wxString group, wxString key, void* retval, ValueType type, bool required, int nmin, int nmax)
{
	wxString string;
	int integer;
	static wxInt16 sizes[2][4] = {{800, 1007, 1263, 1583}, {500, 663, 855, 1095}};

	try
	{
		m_ODFIni.SetPath(wxT("/"));
		if (!m_ODFIni.HasGroup(group))
		{//JB: strncasecmp was strnicmp
			if (group.length() >= 12 && !group.Mid(0, 12).CmpNoCase(wxT("FrameGeneral")))	// FrameGeneral groups aren't required.
			{
				if (type == ORGAN_INTEGER)
					*(int*)retval = 0;
				return false;
			}
			throw -1;
		}
		m_ODFIni.SetPath(wxT("/") + group);

		if (type >= ORGAN_INTEGER)
		{
			if (!m_ODFIni.Read(key, &string) || string.empty())
			{
				integer = nmin;
				if (required)
					throw -2;
			}
			else
			{
				if (!::wxIsdigit(string[0]) && string[0] != wxT('+') && string[0] != wxT('-') && string.CmpNoCase(wxT("none")))
					throw -3;
				integer = atoi(string.mb_str() + (string[0] == wxT('+') ? 1 : 0));
			}

			if (integer >= nmin && integer <= nmax)
			{
				if (type == ORGAN_LONG)
					*(int*)retval = integer;
				else
					*(int*)retval = integer;
			}
			else
				throw -4;
		}
		else
		{
			if (!m_ODFIni.Read(key, &string))
			{
				if (required)
					throw -2;
			}
			else
			{
				string.Trim();
				if (type < ORGAN_STRING)
					string.MakeUpper();

				integer = 0;
				switch(type)
				{
				case ORGAN_BOOLEAN:
					// we cannot assign bitfield bool's the normal way
					// pointers to bitfields are not allowed!
					if (string[0] == wxT('Y'))
						return true;
					else if (string[0] == wxT('N'))
						return false;
					else
						throw -3;
					break;
				case ORGAN_FONTSIZE:
					if (string == wxT("SMALL"))
						integer = 6;
					else if (string == wxT("NORMAL"))
						integer = 7;
					else if (string == wxT("LARGE"))
						integer = 10;
					else
						throw -3;
					*(wxInt16*)retval = integer;
					break;
				case ORGAN_SIZE:
					if (string == wxT("SMALL"))
						integer = 0;
					else if (string == wxT("MEDIUM"))
						integer = 1;
					else if (string == wxT("MEDIUM LARGE"))
						integer = 2;
					else if (string == wxT("LARGE"))
						integer = 3;
					else
						throw -3;
					*(wxInt16*)retval = sizes[nmin][integer];
					break;
				case ORGAN_COLOR:
					if (string == wxT("BLACK"))
						*(wxColour*)retval = wxColour(0x00, 0x00, 0x00);
					else if (string == wxT("BLUE"))
						*(wxColour*)retval = wxColour(0x00, 0x00, 0xFF);
					else if (string == wxT("DARK BLUE"))
						*(wxColour*)retval = wxColour(0x00, 0x00, 0x80);
					else if (string == wxT("GREEN"))
						*(wxColour*)retval = wxColour(0x00, 0xFF, 0x00);
					else if (string == wxT("DARK GREEN"))
						*(wxColour*)retval = wxColour(0x00, 0x80, 0x00);
					else if (string == wxT("CYAN"))
						*(wxColour*)retval = wxColour(0x00, 0xFF, 0xFF);
					else if (string == wxT("DARK CYAN"))
						*(wxColour*)retval = wxColour(0x00, 0x80, 0x80);
					else if (string == wxT("RED"))
						*(wxColour*)retval = wxColour(0xFF, 0x00, 0x00);
					else if (string == wxT("DARK RED"))
						*(wxColour*)retval = wxColour(0x80, 0x00, 0x00);
					else if (string == wxT("MAGENTA"))
						*(wxColour*)retval = wxColour(0xFF, 0x00, 0xFF);
					else if (string == wxT("DARK MAGENTA"))
						*(wxColour*)retval = wxColour(0x80, 0x00, 0x80);
					else if (string == wxT("YELLOW"))
						*(wxColour*)retval = wxColour(0xFF, 0xFF, 0x00);
					else if (string == wxT("DARK YELLOW"))
						*(wxColour*)retval = wxColour(0x80, 0x80, 0x00);
					else if (string == wxT("LIGHT GREY"))
						*(wxColour*)retval = wxColour(0xC0, 0xC0, 0xC0);
					else if (string == wxT("DARK GREY"))
						*(wxColour*)retval = wxColour(0x80, 0x80, 0x80);
					else if (string == wxT("WHITE"))
						*(wxColour*)retval = wxColour(0xFF, 0xFF, 0xFF);
					else
						throw -3;
					break;
				case ORGAN_STRING:
					if ((int)string.length() > nmax)
						throw -4;
					*(wxString*)retval = string;
                default:
					break;
				}
			}
		}
	}
	catch(int exception)
	{
		wxString error;
		switch(exception)
		{
		case -1:
			error.Printf(_("Missing required group '/%s'"), group.c_str());
			break;
		case -2:
			error.Printf(_("Missing required value '/%s/%s'"), group.c_str(), key.c_str());
			break;
		case -3:
			error.Printf(_("Invalid value '/%s/%s'"), group.c_str(), key.c_str());
			break;
		case -4:
			error.Printf(_("Out of range value '/%s/%s'"), group.c_str(), key.c_str());
			break;
		}
		throw error;
	}

	return false;
}

bool IniFileConfig::ReadBoolean(wxString group, wxString key, bool required)
{
	return ReadKey(group, key, 0, ORGAN_BOOLEAN, required);
}

wxColour IniFileConfig::ReadColor(wxString group, wxString key, bool required)
{
	wxColour retval;
	ReadKey(group, key, &retval, ORGAN_COLOR, required);
	return retval;
}

wxString IniFileConfig::ReadString(wxString group, wxString key, int nmax, bool required)
{
	wxString retval;
	ReadKey(group, key, &retval, ORGAN_STRING, required, 0, nmax);
	return retval;
}

int IniFileConfig::ReadInteger(wxString group, wxString key, int nmin, int nmax, bool required)
{
	int retval;
	ReadKey(group, key, &retval, ORGAN_INTEGER, required, nmin, nmax);
	return retval;
}

int IniFileConfig::ReadLong(wxString group, wxString key, int nmin, int nmax, bool required)
{
	int retval;
	ReadKey(group, key, &retval, ORGAN_LONG, required, nmin, nmax);
	return retval;
}

wxInt16 IniFileConfig::ReadSize(wxString group, wxString key, int nmin, bool required)
{
	wxInt16 retval;
	ReadKey(group, key, &retval, ORGAN_SIZE, required, nmin);
	return retval;
}

wxInt16 IniFileConfig::ReadFontSize(wxString group, wxString key, bool required)
{
	wxInt16 retval;
	ReadKey(group, key, &retval, ORGAN_FONTSIZE, required);
	return retval;
}

int IniFileConfig::ReadEnum(wxString group, wxString key, const struct IniFileEnumEntry* entry, unsigned count, bool required)
{
	wxString value = ReadString(group, key, 255, required);
	for (unsigned i = 0; i < count; i++)
		if (entry[i].name == value)
			return entry[i].value;
	if (required || !value.IsEmpty())
	{
		wxString error;
		error.Printf(_("Invalid value '/%s/%s': %s"), group.c_str(), key.c_str(), value.c_str());
		throw error;
	}
	return entry[0].value;
}


void IniFileConfig::SaveHelper(bool prefix, wxString group, wxString key, wxString value)
{
	wxString str = group + wxT('/') + key;
	if (m_ODFIni.Read(str) != value)
	{
		if (prefix)
		{
			m_ODFIni.Write(wxT('_') + str, value);
		}
		else
			m_ODFIni.Write(str, value);
	}
}

void IniFileConfig::SaveHelper( bool prefix, wxString group, wxString key, int value, bool sign, bool force)
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
	SaveHelper(prefix, group, key, str);
}

void IniFileConfig::SaveHelper(bool prefix, wxString group, wxString key, int value, const struct IniFileEnumEntry* entry, unsigned count)
{
	for (unsigned i = 0; i < count; i++)
		if (entry[i].value == value)
		{
			SaveHelper(prefix, group, key, entry[i].name);
			return;
		}
	wxLogError(_("Invalid enum value for /%s/%s: %d"), group.c_str(), key.c_str(), value);
}
