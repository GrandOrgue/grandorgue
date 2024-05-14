/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GORightVisiblePicker.h"

#include <wx/pickerbase.h>
#include <wx/textctrl.h>

void GORightVisiblePicker::EnsureRigtIsVisible() {
  wxTextCtrl *pTxt = p_picker->GetTextCtrl();

  if (pTxt) {
    pTxt->ShowPosition(pTxt->GetValue().length());
  }
}
