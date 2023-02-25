/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTINGFILE_H
#define GOSETTINGFILE_H

#include "settings/GOSettingString.h"

class GOSettingFile : public GOSettingString {
protected:
  wxString validate(wxString value);

public:
  GOSettingFile(
    GOSettingStore *store,
    wxString group,
    wxString name,
    wxString default_value);
};

#endif
