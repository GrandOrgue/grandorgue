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

#include "GOrgueCacheCleaner.h"

#include "GOrgueArchiveFile.h"
#include "GOrgueOrgan.h"
#include "GOrgueSettings.h"
#include "ptrvector.h"
#include <wx/arrstr.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/intl.h>
#include <wx/log.h>

GOrgueCacheCleaner::GOrgueCacheCleaner(GOrgueSettings& settings) :
	m_settings(settings)
{
}
	    
GOrgueCacheCleaner::~GOrgueCacheCleaner()
{
}

wxArrayString GOrgueCacheCleaner::GetOrganIDList()
{
	wxArrayString list;
	ptr_vector<GOrgueOrgan>& organs = m_settings.GetOrganList();
	for(unsigned i = 0; i < organs.size(); i++)
		list.Add(organs[i]->GetOrganHash());
	return list;
}

wxArrayString GOrgueCacheCleaner::GetArchiveIDList()
{
	wxArrayString list;
	ptr_vector<GOrgueArchiveFile>& archives = m_settings.GetArchiveList();
	for(unsigned i = 0; i < archives.size(); i++)
		list.Add(archives[i]->GetArchiveHash());
	return list;
}

void GOrgueCacheCleaner::Cleanup()
{
	if (!m_settings.ManageCache())
		return;

	wxDir dir(m_settings.UserCachePath());
	if(!dir.IsOpened())
	{
		wxLogError(_("Failed to read cache directory"));
		return;
	}

	wxArrayString organs = GetOrganIDList();
	wxArrayString archives = GetArchiveIDList();

	wxString name;
	if (!dir.GetFirst(&name))
		return;
	do
	{
		wxFileName fn(name);
		if (fn.GetExt() == wxT("idx"))
		{
			if (archives.Index(fn.GetName()) == wxNOT_FOUND)
				wxRemoveFile(dir.GetNameWithSep() + name);
		}
		else if (fn.GetExt() == wxT("cache"))
		{
			if (organs.Index(fn.GetName().Mid(0, 40)) == wxNOT_FOUND)
				wxRemoveFile(dir.GetNameWithSep() + name);
		}
		else
			wxLogError(_("Unexpected file in the cache directory: %s"), name.c_str());
	}
	while(dir.GetNext(&name));
}
