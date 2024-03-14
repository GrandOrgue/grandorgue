/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GODIRPICKERCTRL_H
#define GODIRPICKERCTRL_H

#include <wx/filepicker.h>

#include "GORightVisiblePicker.h"

class GODirPickerCtrl : public wxDirPickerCtrl, private GORightVisiblePicker {
public:
  GODirPickerCtrl(
    wxWindow *parent,
    wxWindowID id,
    const wxString &title,
    const wxPoint &pos = wxDefaultPosition,
    const wxSize &size = wxDefaultSize,
    long style = 0)
    : wxDirPickerCtrl(parent, id, wxEmptyString, title, pos, size, style),
      GORightVisiblePicker(this) {}

  OVERRIDE_UPDATE_TEXTCTRL(wxDirPickerCtrl)
};

#endif /* GODIRPICKERCTRL_H */
