/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOCacheCleaner.h"

#include "GOArchiveFile.h"
#include "GOOrgan.h"
#include "GOSettings.h"
#include "ptrvector.h"
#include <wx/arrstr.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/intl.h>
#include <wx/log.h>

GOCacheCleaner::GOCacheCleaner(GOSettings& settings) :
	m_settings(settings)
{
}
	    
GOCacheCleaner::~GOCacheCleaner()
{
}

wxArrayString GOCacheCleaner::GetOrganIDList()
{
	wxArrayString list;
	const ptr_vector<GOOrgan>& organs = m_settings.GetOrganList();
	for(unsigned i = 0; i < organs.size(); i++)
		list.Add(organs[i]->GetOrganHash());
	return list;
}

wxArrayString GOCacheCleaner::GetArchiveIDList()
{
	wxArrayString list;
	const ptr_vector<GOArchiveFile>& archives = m_settings.GetArchiveList();
	for(unsigned i = 0; i < archives.size(); i++)
		list.Add(archives[i]->GetArchiveHash());
	return list;
}

void GOCacheCleaner::Cleanup()
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
