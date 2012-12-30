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

#include "GOrgueConfigReaderDB.h"
#include "GOrgueConfigFileReader.h"

GOrgueConfigReaderDB::GOrgueConfigReaderDB()
{
}

GOrgueConfigReaderDB::~GOrgueConfigReaderDB()
{
}

bool GOrgueConfigReaderDB::ReadData(GOrgueConfigFileReader& ODF, GOSettingType type, bool handle_prefix)
{
	const std::map<wxString, std::map<wxString, wxString> >& entries = ODF.GetContent();
	bool changed = false;

	for(std::map<wxString, std::map<wxString, wxString> >::const_iterator i = entries.begin(); i != entries.end(); i++)
	{
		const std::map<wxString, wxString>& g = i->second;
		wxString group = i->first;

		if (!handle_prefix || group.StartsWith(wxT("_")))
		{
			if (handle_prefix)
				group = group.Mid(1);

			for(std::map<wxString, wxString>::const_iterator j = g.begin(); j != g.end(); j++)
			{
				const wxString& key = j->first;
				const wxString& value = j->second;

				if (type == ODFSetting)
				{
					AddEntry(m_ODF, group + wxT('/') + key, value);
					AddEntry(m_ODF_LC, (group + wxT('/') + key).Lower(), value);
				}
				else
					AddEntry(m_CMB, group + wxT('/') + key, value);
				changed = true;
			}
		}
	}
	
	return changed;
}

void GOrgueConfigReaderDB::ClearCMB()
{
	m_CMB.clear();
}

void GOrgueConfigReaderDB::AddEntry(GOStringHashMap& hash, wxString key, wxString value)
{
	GOStringHashMap::iterator i = hash.find(key);
	if (i != hash.end())
	{
		wxLogWarning(_("Dupplicate entry: %s"), key.c_str());
	}
	hash[key] = value;
}


bool GOrgueConfigReaderDB::GetString(GOSettingType type, wxString group, wxString key, wxString& value)
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
