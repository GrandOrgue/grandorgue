/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2013 GrandOrgue contributors (see AUTHORS)
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

void GOrgueConfigReaderDB::ReportUnused()
{
	for(GOBoolHashMap::iterator i = m_CMBUsed.begin(); i != m_CMBUsed.end(); i++)
	{
		if (!i->second)
		{
			wxLogWarning(_("Unused CMB entry '%s'"), i->first.c_str());
		}
	}
	bool warn_old = false;
	for(GOBoolHashMap::iterator i = m_ODFUsed.begin(); i != m_ODFUsed.end(); i++)
	{
		if (!i->second)
		{
			if (i->first.StartsWith(wxT("_")))
			{
				if (!warn_old)
				{
					wxLogWarning(_("Old GO 0.2 styled setting in ODF"));
				}
				warn_old = true;
			}
			else
			{
				wxLogWarning(_("Unused ODF entry '%s'"), i->first.c_str());
			}
		}
	}
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
					m_ODFUsed[group + wxT('/') + key] = false;
				}
				else
				{
					AddEntry(m_CMB, group + wxT('/') + key, value);
					m_CMBUsed[group + wxT('/') + key] = false;
				}
				changed = true;
			}
		}
	}
	
	return changed;
}

void GOrgueConfigReaderDB::ClearCMB()
{
	m_CMB.clear();
	m_CMBUsed.clear();
}

void GOrgueConfigReaderDB::AddEntry(GOStringHashMap& hash, wxString key, wxString value)
{
	GOStringHashMap::iterator i = hash.find(key);
	if (i != hash.end())
	{
		wxLogWarning(_("Duplicate entry: %s"), key.c_str());
	}
	hash[key] = value;
}


bool GOrgueConfigReaderDB::GetString(GOSettingType type, wxString group, wxString key, wxString& value)
{
	wxString index = group + wxT("/") + key;
	if (type == UserSetting || type == CMBSetting)
	{
		m_CMBUsed[index] = true;
		GOStringHashMap::iterator i = m_CMB.find(index);
		if (i != m_CMB.end())
		{
			if (type == UserSetting)
				m_ODFUsed[index] = true;
			value = i->second;
			return true;
		}
	}

	if (type == UserSetting || type == ODFSetting)
	{
		m_ODFUsed[index] = true;
		GOStringHashMap::iterator i = m_ODF.find(index);
		if (i != m_ODF.end())
		{
			value = i->second;
			return true;
		}
	}
	if (type == UserSetting || type == ODFSetting)
	{
		GOStringHashMap::iterator i = m_ODF_LC.find(index.Lower());
		if (i != m_ODF.end())
		{
			wxLogWarning(_("Incorrect case for section '%s' entry '%s'"), group.c_str(), key.c_str());
			value = i->second;
			return true;
		}
	}
	return false;
}
