/*
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOICONMANAGER_H
#define GOICONMANAGER_H

#include <wx/artprov.h>
// #include "wxMaterialDesignArtProvider.hpp"
// #include "wxMaterialTwoToneArt.hpp"
#include <string>
#include <wx/bitmap.h>
#include <wx/regex.h>
#include <wx/string.h>

#ifndef wxART_DATA_SAVER_ON
#define wxART_DATA_SAVER_ON wxART_MAKE_ART_ID(wxART_DATA_SAVER_ON)
#endif
static constexpr const char *SVG_MATERIAL_SHARP_DATA_SAVER_ON
  = R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24"><path d="M11 8v3H8v2h3v3h2v-3h3v-2h-3V8h-2zm2-5.95v3.03c3.39.49 6 3.39 6 6.92 0 .9-.18 1.75-.48 2.54l2.6 1.53c.56-1.24.88-2.62.88-4.07 0-5.18-3.95-9.45-9-9.95zM12 19c-3.87 0-7-3.13-7-7 0-3.53 2.61-6.43 6-6.92V2.05c-5.06.5-9 4.76-9 9.95 0 5.52 4.47 10 9.99 10 3.31 0 6.24-1.61 8.06-4.09l-2.6-1.53A6.95 6.95 0 0 1 12 19z"/></svg>)rawsvg";

#ifndef wxART_SETTINGS_ETHERNET
#define wxART_SETTINGS_ETHERNET wxART_MAKE_ART_ID(wxART_SETTINGS_ETHERNET)
#endif
static constexpr const char *SVG_MATERIAL_SHARP_SETTINGS_ETHERNET
  = R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24"><path d="M7.77 6.76 6.23 5.48.82 12l5.41 6.52 1.54-1.28L3.42 12l4.35-5.24zM7 13h2v-2H7v2zm10-2h-2v2h2v-2zm-6 2h2v-2h-2v2zm6.77-7.52-1.54 1.28L20.58 12l-4.35 5.24 1.54 1.28L23.18 12l-5.41-6.52z"/></svg>)rawsvg";

#ifndef wxART_VOLUME_UP
#define wxART_VOLUME_UP wxART_MAKE_ART_ID(wxART_VOLUME_UP)
#endif
static constexpr const char *SVG_MATERIAL_SHARP_VOLUME_UP
  = R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24"><path d="M3 9v6h4l5 5V4L7 9H3zm13.5 3A4.5 4.5 0 0 0 14 7.97v8.05c1.48-.73 2.5-2.25 2.5-4.02zM14 3.23v2.06c2.89.86 5 3.54 5 6.71s-2.11 5.85-5 6.71v2.06c4.01-.91 7-4.49 7-8.77s-2.99-7.86-7-8.77z"/></svg>)rawsvg";

#ifndef wxART_COMPARE_ARROWS
#define wxART_COMPARE_ARROWS wxART_MAKE_ART_ID(wxART_COMPARE_ARROWS)
#endif
static constexpr const char *SVG_MATERIAL_SHARP_COMPARE_ARROWS
  = R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24"><path d="M9.01 14H2v2h7.01v3L13 15l-3.99-4v3zm5.98-1v-3H22V8h-7.01V5L11 9l3.99 4z"/></svg>)rawsvg";

#ifndef wxART_LEAK_ADD
#define wxART_LEAK_ADD wxART_MAKE_ART_ID(wxART_LEAK_ADD)
#endif
static constexpr const char *SVG_MATERIAL_SHARP_LEAK_ADD
  = R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24"><path d="M6 3H3v3c1.66 0 3-1.34 3-3zm8 0h-2a9 9 0 0 1-9 9v2c6.08 0 11-4.93 11-11zm-4 0H8c0 2.76-2.24 5-5 5v2c3.87 0 7-3.13 7-7zm0 18h2a9 9 0 0 1 9-9v-2c-6.07 0-11 4.93-11 11zm8 0h3v-3c-1.66 0-3 1.34-3 3zm-4 0h2c0-2.76 2.24-5 5-5v-2c-3.87 0-7 3.13-7 7z"/></svg>)rawsvg";

#ifndef wxART_WARNING_AMBER
#define wxART_WARNING_AMBER wxART_MAKE_ART_ID(wxART_WARNING_AMBER)
#endif
static constexpr const char *SVG_MATERIAL_SHARP_WARNING_AMBER
  = R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24"><path d="M12 5.99 19.53 19H4.47L12 5.99M12 2 1 21h22L12 2zm1 14h-2v2h2v-2zm0-6h-2v4h2v-4z"/></svg>)rawsvg";

#ifndef wxART_PIANO
#define wxART_PIANO wxART_MAKE_ART_ID(wxART_PIANO)
#endif
static constexpr const char *SVG_MATERIAL_SHARP_PIANO
  = R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24"><path d="M21 3H3v18h18V3zm-8 11.5h1.25V19h-4.5v-4.5H11V5h2v9.5zM5 5h2v9.5h1.25V19H5V5zm14 14h-3.25v-4.5H17V5h2v14z"/></svg>)rawsvg";

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
    wxString svg;
    if (icon_name == "set") {
      svg = SVG_MATERIAL_SHARP_DATA_SAVER_ON;
    }
    if (icon_name == "level") {
      svg = SVG_MATERIAL_SHARP_SETTINGS_ETHERNET;
    }
    if (icon_name == "volume") {
      svg = SVG_MATERIAL_SHARP_VOLUME_UP;
    }
    if (icon_name == "transpose") {
      svg = SVG_MATERIAL_SHARP_COMPARE_ARROWS;
    }
    if (icon_name == "reverb") {
      svg = SVG_MATERIAL_SHARP_LEAK_ADD;
    }
    if (icon_name == "polyphony") {
      svg = SVG_MATERIAL_SHARP_PIANO;
    }
    if (icon_name == "panic") {
      svg = SVG_MATERIAL_SHARP_WARNING_AMBER;
    }
    const wxSize &size = wxSize(24, 24);
    if (svg.IsEmpty())
      return wxNullBitmap;

    // Create bundle and get bitmap from it
    wxSize useSize = size == wxDefaultSize ? wxSize(24, 24) : size;
    wxBitmapBundle bundle = wxBitmapBundle::FromSVG(svg.mb_str(), useSize);
    if (!bundle.IsOk())
      return wxNullBitmap;
    wxBitmap bmp = bundle.GetBitmap(useSize);
    if (!bmp.IsOk())
      return wxNullBitmap;
    return bmp;
  };
};

#endif /* GOMIDI_H */