/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOBITMAPCACHE_H
#define GOBITMAPCACHE_H

#include <wx/string.h>

#include "GOBitmap.h"
#include "ptrvector.h"

class GODefinitionFile;

class GOBitmapCache {
 private:
  GODefinitionFile *m_organfile;
  ptr_vector<wxImage> m_Bitmaps;
  std::vector<wxString> m_Filenames;
  std::vector<wxString> m_Masknames;

  bool loadFile(wxImage &img, wxString filename);

 public:
  GOBitmapCache(GODefinitionFile *organfile);
  virtual ~GOBitmapCache();

  void RegisterBitmap(
    wxImage *bitmap, wxString filename, wxString maskname = wxEmptyString);
  GOBitmap GetBitmap(wxString filename, wxString maskName = wxEmptyString);
};

#endif
