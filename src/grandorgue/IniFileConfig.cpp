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


#include "IniFileConfig.h"
#include <wx/fileconf.h>

IniFileConfig::IniFileConfig(wxFileConfig& odf_ini_file) :
	m_ODFIni (odf_ini_file),
	m_LastGroup(),
	m_LastGroupExists(true)
{

}

bool IniFileConfig::GetString(GOSettingType type, wxString group, wxString key, wxString& value)
{
	if (group != m_LastGroup)
	{
		m_ODFIni.SetPath(wxT("/"));
		m_LastGroupExists = m_ODFIni.HasGroup(group);
		if (m_LastGroupExists)
			m_ODFIni.SetPath(wxT("/") + group);
		m_LastGroup = group;
	}
	if (!m_LastGroupExists)
		return false;

	if (!m_ODFIni.Read(key, &value))
	{
		return false;
	}
	return true;
}
