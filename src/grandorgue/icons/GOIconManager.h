/*
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOICONMANAGER_H
#define GOICONMANAGER_H

#include "wx/artprov.h"
#include "wxMaterialDesignArtProvider.hpp"
#include "wxMaterialTwoToneArt.hpp"
#include <string>
#include <wx/bitmap.h>
#include <wx/string.h>

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
    wxString icon_id;
    if (icon_name == "set") {
      icon_id = wxART_DATA_SAVER_ON;
    }
    if (icon_name == "level") {
      icon_id = wxART_SETTINGS_ETHERNET;
    }
    if (icon_name == "volume") {
      icon_id = wxART_VOLUME_UP;
    }
    if (icon_name == "transpose") {
      icon_id = wxART_COMPARE_ARROWS;
    }
    if (icon_name == "reverb") {
      icon_id = wxART_LEAK_ADD;
    }
    if (icon_name == "polyphony") {
      icon_id = wxART_PIANO;
    }
    if (icon_name == "panic") {
      icon_id = wxART_WARNING_AMBER;
    }

    // wxArtProvider::Push(new wxMaterialDesignArtProvider);
    wxBitmap image = wxMaterialDesignArtProvider::GetBitmap(
      icon_id, wxART_CLIENT_MATERIAL_SHARP, wxSize(24, 24));
    return image;
  };
};

#endif /* GOMIDI_H */