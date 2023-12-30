/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOCacheCleaner.h"

#include <wx/arrstr.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/intl.h>
#include <wx/log.h>

#include "GOOrgan.h"
#include "archive/GOArchiveFile.h"
#include "config/GOConfig.h"
#include "ptrvector.h"

GOCacheCleaner::GOCacheCleaner(GOConfig &settings) : m_config(settings) {}

GOCacheCleaner::~GOCacheCleaner() {}

wxArrayString GOCacheCleaner::GetOrganIDList() {
  wxArrayString list;
  const ptr_vector<GOOrgan> &organs = m_config.GetOrganList();
  for (unsigned i = 0; i < organs.size(); i++)
    list.Add(organs[i]->GetOrganHash());
  return list;
}

wxArrayString GOCacheCleaner::GetArchiveIDList() {
  wxArrayString list;
  const ptr_vector<GOArchiveFile> &archives = m_config.GetArchiveList();
  for (unsigned i = 0; i < archives.size(); i++)
    list.Add(archives[i]->GetArchiveHash());
  return list;
}

void GOCacheCleaner::Cleanup() {
  if (!m_config.ManageCache())
    return;

  wxDir dir(m_config.OrganCachePath());
  if (!dir.IsOpened()) {
    wxLogError(_("Failed to read cache directory"));
    return;
  }

  wxArrayString organs = GetOrganIDList();
  wxArrayString archives = GetArchiveIDList();

  wxString name;
  if (!dir.GetFirst(&name))
    return;
  do {
    wxFileName fn(name);
    if (fn.GetExt() == wxT("idx")) {
      if (archives.Index(fn.GetName()) == wxNOT_FOUND)
        wxRemoveFile(dir.GetNameWithSep() + name);
    } else if (fn.GetExt() == wxT("cache")) {
      if (organs.Index(fn.GetName().Mid(0, 40)) == wxNOT_FOUND)
        wxRemoveFile(dir.GetNameWithSep() + name);
    } else
      wxLogError(_("Unexpected file in the cache directory: %s"), name.c_str());
  } while (dir.GetNext(&name));
}
