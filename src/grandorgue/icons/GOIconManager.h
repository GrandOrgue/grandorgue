/*
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOICONMANAGER_H
#define GOICONMANAGER_H

#include "wx/artprov.h"
#include <string>
#include <wx/bitmap.h>
#include <wxMaterialFilledArt.hpp>
/*
 * This is a class to manage icons through the application
 * It is agnostic from any particular library
 */
class GOIconManager {

public:
  GOIconManager();
  ~GOIconManager();

  /*
   * This returns the icon in char
   */
  wxBitmap GetIcon(std::string icon_name) {
    wxArtProvider::Push(new wxMaterialDesignArtProvider);
    wxBitmap image = wxArtProvider::GetBitmap(
      wxART_CLIENT_MATERIAL_TWOTONE, wxART_VOLUME_UP, wxSize(10, 10));
    return image;
  };
};

#endif /* GOMIDI_H */