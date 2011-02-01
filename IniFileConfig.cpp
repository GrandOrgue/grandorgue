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

bool IniFileConfig::ReadKey(const char* group, const char* key, void* retval, ValueType type, bool required, int nmin, int nmax)
{
	wxString string;
	wxInt16 integer;
	static wxInt16 sizes[2][4] = {{800, 1007, 1263, 1583}, {500, 663, 855, 1095}};

	try
	{
		m_cfg->SetPath("/");
		if (!m_cfg->HasGroup(group))
		{//JB: strncasecmp was strnicmp
			if (strlen(group) >= 8 && !strncasecmp(group, "General", 7) && group[7] > '0')	// FrameGeneral groups aren't required.
			{
			    if (type == ORGAN_INTEGER)
                    *(wxInt16*)retval = 0;
				return false;
			}
			throw -1;
		}
		m_cfg->SetPath(wxString("/") + group);

		if (type >= ORGAN_INTEGER)
		{
			if (!m_cfg->Read(key, &string) || string.empty())
			{
				integer = nmin;
				if (required)
					throw -2;
			}
			else
			{
				if (!::wxIsdigit(string[0]) && string[0] != '+' && string[0] != '-' && string.CmpNoCase("none"))
					throw -3;
				integer = atoi(string.c_str() + (string[0] == '+' ? 1 : 0));
			}

			if (integer >= nmin && integer <= nmax)
			{
				if (type == ORGAN_LONG)
					*(wxInt32*)retval = integer;
				else
					*(wxInt16*)retval = integer;
			}
			else
				throw -4;
		}
		else
		{
			if (!m_cfg->Read(key, &string))
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
					if (string[0] == 'Y')
						return true;
					else if (string[0] == 'N')
						return false;
					else
						throw -3;
					break;
				case ORGAN_FONTSIZE:
					if (string == "SMALL")
						integer = 6;
					else if (string == "NORMAL")
						integer = 7;
					else if (string == "LARGE")
						integer = 10;
					else
						throw -3;
					*(wxInt16*)retval = integer;
					break;
				case ORGAN_SIZE:
					if (string == "SMALL")
						integer = 0;
					else if (string == "MEDIUM")
						integer = 1;
					else if (string == "MEDIUM LARGE")
						integer = 2;
					else if (string == "LARGE")
						integer = 3;
					else
						throw -3;
					*(wxInt16*)retval = sizes[nmin][integer];
					break;
				case ORGAN_COLOR:
					if (string == "BLACK")
						*(wxColour*)retval = wxColour(0x00, 0x00, 0x00);
					else if (string == "BLUE")
						*(wxColour*)retval = wxColour(0x00, 0x00, 0xFF);
					else if (string == "DARK BLUE")
						*(wxColour*)retval = wxColour(0x00, 0x00, 0x80);
					else if (string == "GREEN")
						*(wxColour*)retval = wxColour(0x00, 0xFF, 0x00);
					else if (string == "DARK GREEN")
						*(wxColour*)retval = wxColour(0x00, 0x80, 0x00);
					else if (string == "CYAN")
						*(wxColour*)retval = wxColour(0x00, 0xFF, 0xFF);
					else if (string == "DARK CYAN")
						*(wxColour*)retval = wxColour(0x00, 0x80, 0x80);
					else if (string == "RED")
						*(wxColour*)retval = wxColour(0xFF, 0x00, 0x00);
					else if (string == "DARK RED")
						*(wxColour*)retval = wxColour(0x80, 0x00, 0x00);
					else if (string == "MAGENTA")
						*(wxColour*)retval = wxColour(0xFF, 0x00, 0xFF);
					else if (string == "DARK MAGENTA")
						*(wxColour*)retval = wxColour(0x80, 0x00, 0x80);
					else if (string == "YELLOW")
						*(wxColour*)retval = wxColour(0xFF, 0xFF, 0x00);
					else if (string == "DARK YELLOW")
						*(wxColour*)retval = wxColour(0x80, 0x80, 0x00);
					else if (string == "LIGHT GREY")
						*(wxColour*)retval = wxColour(0xC0, 0xC0, 0xC0);
					else if (string == "DARK GREY")
						*(wxColour*)retval = wxColour(0x80, 0x80, 0x80);
					else if (string == "WHITE")
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
			error.Printf("Missing required group '/%s'", group);
			break;
		case -2:
			error.Printf("Missing required value '/%s/%s'", group, key);
			break;
		case -3:
			error.Printf("Invalid value '/%s/%s'", group, key);
			break;
		case -4:
			error.Printf("Out of range value '/%s/%s'", group, key);
			break;
		}
		throw error;
	}

	return false;
}

bool IniFileConfig::ReadBoolean(const char* group, const char* key, bool required)
{
	return ReadKey(group, key, 0, ORGAN_BOOLEAN, required);
}

wxColour IniFileConfig::ReadColor(const char* group, const char* key, bool required)
{
	wxColour retval;
	ReadKey(group, key, &retval, ORGAN_COLOR, required);
	return retval;
}

wxString IniFileConfig::ReadString(const char* group, const char* key, int nmax, bool required)
{
	wxString retval;
	ReadKey(group, key, &retval, ORGAN_STRING, required, 0, nmax);
	return retval;
}

wxInt16 IniFileConfig::ReadInteger(const char* group, const char* key, int nmin, int nmax, bool required)
{
	wxInt16 retval;
	ReadKey(group, key, &retval, ORGAN_INTEGER, required, nmin, nmax);
	return retval;
}

wxInt32 IniFileConfig::ReadLong(const char* group, const char* key, int nmin, int nmax, bool required)
{
	wxInt32 retval;
	ReadKey(group, key, &retval, ORGAN_LONG, required, nmin, nmax);
	return retval;
}

wxInt16 IniFileConfig::ReadSize(const char* group, const char* key, int nmin, bool required)
{
	wxInt16 retval;
	ReadKey(group, key, &retval, ORGAN_SIZE, required, nmin);
	return retval;
}

wxInt16 IniFileConfig::ReadFontSize(const char* group, const char* key, bool required)
{
	wxInt16 retval;
	ReadKey(group, key, &retval, ORGAN_FONTSIZE, required);
	return retval;
}

void IniFileConfig::SaveHelper(bool prefix, wxString group, wxString key, wxString value)
{
    wxString str = group + '/' + key;
    if (m_cfg->Read(str) != value)
    {
        if (prefix)
        {
            m_cfg->Write('_' + str, value);
        }
        else
            m_cfg->Write(str, value);
    }
}

void IniFileConfig::SaveHelper( bool prefix, wxString group, wxString key, int value, bool sign, bool force)
{
    wxString str;
    if (force)
        str.Printf("%03d", value);
    else if (sign)
    {
        if (value >= 0)
            str.Printf("+%03d", value);
        else
            str.Printf("-%03d", -value);
    }
    else
        str.Printf("%d", value);
    SaveHelper(prefix, group, key, str);
}
