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
 * MA 02111-1307, USA.
 */


#include "IniFileConfig.h"
#include <wx/fileconf.h>

IniFileConfig::IniFileConfig(wxFileConfig& odf_ini_file) :
	m_ODFIni (odf_ini_file),
	m_LastGroup(),
	m_LastGroupExists(true)
{

}

wxString IniFileConfig::ReadString(GOSettingType type, wxString group, wxString key, unsigned nmax, bool required, wxString defaultValue)
{
	wxString value;

	if (group != m_LastGroup)
	{
		m_ODFIni.SetPath(wxT("/"));
		m_LastGroupExists = m_ODFIni.HasGroup(group);
		if (m_LastGroupExists)
			m_ODFIni.SetPath(wxT("/") + group);
		m_LastGroup = group;
	}
	if (!m_LastGroupExists)
	{
		if (group.length() >= 6 && !group.Mid(0, 6).CmpNoCase(wxT("Setter")))	// Setter groups aren't required.
			return defaultValue;
		if (group.length() >= 5 && !group.Mid(0, 5).CmpNoCase(wxT("Panel")))
			if (group.length() >= 14 && !group.Mid(8, 6).CmpNoCase(wxT("Setter")))	// Setter groups aren't required.
				return defaultValue;

		if (group.length() >= 12 && !group.Mid(0, 12).CmpNoCase(wxT("FrameGeneral")))	// FrameGeneral groups aren't required.
			return defaultValue;

		if (required)
		{
			wxString error;
			error.Printf(_("Missing required group '/%s'"), group.c_str());
			throw error;
		}
	}

	if (!m_ODFIni.Read(key, &value))
	{
		if (required)
		{
			if (group.length() >= 6 && !group.Mid(0, 6).CmpNoCase(wxT("Setter")))	// Setter groups aren't required.
				return defaultValue;
			if (group.length() >= 5 && !group.Mid(0, 5).CmpNoCase(wxT("Panel")))
				if (group.length() >= 14 && !group.Mid(8, 6).CmpNoCase(wxT("Setter")))	// Setter groups aren't required.
					return defaultValue;

			wxString error;
			error.Printf(_("Missing required value '/%s/%s'"), group.c_str(), key.c_str());
			throw error;
		}
		else
			return defaultValue;
	}

	value.Trim();
	if (value.length() > nmax)
	{
		wxString error;
		error.Printf(_("Value too long: '/%s/%s': %s"), group.c_str(), key.c_str(), value.c_str());
		throw error;
	}

	return value;
}
