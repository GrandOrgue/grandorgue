/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOIMAGECACHE_H
#define GOIMAGECACHE_H

#include <wx/image.h>
#include <wx/string.h>

#include "ptrvector.h"

class GOOrganController;

class GOImageCache {
private:
  static const wxString WX_EMPTY_STRING;

  GOOrganController *m_OrganController;
  ptr_vector<const wxImage> m_images;
  std::vector<wxString> m_filenames;
  std::vector<wxString> m_masknames;

  bool LoadImageFromFile(const wxString &filename, wxImage &image);
  void RegisterImage(
    const wxString &filename, const wxString &maskname, wxImage *pImage);

public:
  GOImageCache(GOOrganController *organController);

  void Cleanup() {
    m_images.clear();
    m_filenames.clear();
    m_masknames.clear();
  };

  const wxImage *LoadImage(
    const wxString &filename, const wxString &maskName = WX_EMPTY_STRING);
};

#endif
