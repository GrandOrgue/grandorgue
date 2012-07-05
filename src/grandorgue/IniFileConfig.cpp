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

IniFileConfig::IniFileConfig()
{
}

bool IniFileConfig::ReadData(wxFileConfig& ODF, GOSettingType type, bool handle_prefix)
{
	wxString group;
	long group_it;
	bool group_cont = ODF.GetFirstGroup(group, group_it);
	bool changed = false;
	while (group_cont)
	{
		if (!handle_prefix || group.StartsWith(wxT("_")))
		{
			ODF.SetPath(wxT('/') + group);
			if (handle_prefix)
				group = group.Mid(1);
			wxString key;
			long key_it;
			bool key_cont = ODF.GetFirstEntry(key, key_it);
			while (key_cont)
			{
				if (type == ODFSetting)
				{
					AddEntry(m_ODF, group + wxT('/') + key, ODF.Read(key));
					AddEntry(m_ODF_LC, (group + wxT('/') + key).Lower(), ODF.Read(key));
				}
				else
					AddEntry(m_CMB, group + wxT('/') + key, ODF.Read(key));
				changed = true;
				key_cont = ODF.GetNextEntry(key, key_it);
			}
			ODF.SetPath(wxT("/"));
		}
		group_cont = ODF.GetNextGroup(group, group_it);
	}
	ODF.SetPath(wxT('/'));
	return changed;
}

void IniFileConfig::AddEntry(GOStringHashMap& hash, wxString key, wxString value)
{
	GOStringHashMap::iterator i = hash.find(key);
	if (i != hash.end())
		wxLogWarning(_("Dupplicate entry: %s"), key.c_str());
	hash[key] = value;
}


bool IniFileConfig::GetString(GOSettingType type, wxString group, wxString key, wxString& value)
{
	if (type == UserSetting || type == CMBSetting)
	{
		GOStringHashMap::iterator i = m_CMB.find(group + wxT("/") + key);
		if (i != m_CMB.end())
		{
			value = i->second;
			return true;
		}
	}

	if (type == UserSetting || type == ODFSetting)
	{
		GOStringHashMap::iterator i = m_ODF.find(group + wxT("/") + key);
		if (i != m_ODF.end())
		{
			value = i->second;
			return true;
		}
	}
	if (type == UserSetting || type == ODFSetting)
	{
		GOStringHashMap::iterator i = m_ODF_LC.find((group + wxT("/") + key).Lower());
		if (i != m_ODF.end())
		{
			wxLogWarning(_("Incorrect case for '/%s/%s'"), group.c_str(), key.c_str());
			value = i->second;
			return true;
		}
	}
	return false;
}
