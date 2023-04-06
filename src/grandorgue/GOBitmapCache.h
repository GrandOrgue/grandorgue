/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOBITMAPCACHE_H
#define GOBITMAPCACHE_H

#include <wx/string.h>

#include "ptrvector.h"

#include "gui/primitives/GOBitmap.h"

class GOOrganController;

class GOBitmapCache {
private:
  GOOrganController *m_OrganController;
  ptr_vector<wxImage> m_Bitmaps;
  std::vector<wxString> m_Filenames;
  std::vector<wxString> m_Masknames;

  bool loadFile(wxImage &img, const wxString &filename);

public:
  GOBitmapCache(GOOrganController *organController);
  virtual ~GOBitmapCache();

  void RegisterBitmap(
    wxImage *bitmap, wxString filename, wxString maskname = wxEmptyString);
  GOBitmap GetBitmap(wxString filename, wxString maskName = wxEmptyString);
};

#endif
