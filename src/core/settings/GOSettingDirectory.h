/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTINGDIRECTORY_H
#define GOSETTINGDIRECTORY_H

#include "settings/GOSettingString.h"

class GOSettingDirectory : public GOSettingString {
protected:
  wxString Validate(const wxString &value) const override;

public:
  GOSettingDirectory(
    GOSettingStore *store,
    const wxString &group,
    const wxString &name,
    const wxString &defaultValue);
};

#endif
