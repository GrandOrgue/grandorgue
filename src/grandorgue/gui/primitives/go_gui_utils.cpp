/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "go_gui_utils.h"

#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/image.h>

#include "Images.h"

static bool has_initialised = false;
static wxIcon go_icon;

static void assure_initialised() {
  if (!has_initialised) {
    wxBitmap iconBitmap(GetImage_GOIcon());

    go_icon.CopyFromBitmap(iconBitmap);
    has_initialised = true;
  }
}

const wxIcon &get_go_icon() {
  assure_initialised();
  return go_icon;
}