/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOFILEPICKERCTRL_H
#define GOFILEPICKERCTRL_H

#include <wx/filepicker.h>

#include "GORightVisiblePicker.h"

class GOFilePickerCtrl : public wxFilePickerCtrl, private GORightVisiblePicker {
public:
  GOFilePickerCtrl(
    wxWindow *parent,
    wxWindowID id,
    const wxString &title,
    const wxString &wildcard,
    const wxPoint &pos = wxDefaultPosition,
    const wxSize &size = wxDefaultSize,
    long style = 0)
    : wxFilePickerCtrl(
      parent, id, wxEmptyString, title, wildcard, pos, size, style),
      GORightVisiblePicker(this) {}

  OVERRIDE_UPDATE_TEXTCTRL(wxFilePickerCtrl)
};

#endif /* GOFILEPICKERCTRL_H */
