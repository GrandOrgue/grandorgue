/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSimpleDialog.h"

#include <wx/sizer.h>
#include <wx/statline.h>

void GOSimpleDialog::LayoutWithInnerSizer(wxSizer *pInnerSizer) {
  wxBoxSizer *const pTopSizer = new wxBoxSizer(wxVERTICAL);

  pTopSizer->Add(pInnerSizer, 1, wxGROW | wxEXPAND | wxALL, 5);
  pTopSizer->Add(new wxStaticLine(this), 0, wxEXPAND | wxALL, 5);
  pTopSizer->Add(GetButtonSizer(), 0, wxEXPAND | wxALL, 5);
  SetSizerAndFit(pTopSizer);
}
