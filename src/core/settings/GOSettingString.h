/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTINGSTRING_H
#define GOSETTINGSTRING_H

#include "settings/GOSetting.h"

class GOSettingString : private GOSetting {
private:
  wxString m_Value;
  wxString m_DefaultValue;

  void Load(GOConfigReader &cfg);
  void Save(GOConfigWriter &cfg);

protected:
  wxString getDefaultValue();
  virtual wxString validate(wxString value);

public:
  GOSettingString(
    GOSettingStore *store,
    wxString group,
    wxString name,
    wxString default_value);

  void setDefaultValue(wxString default_value);

  wxString operator()() const;
  void operator()(wxString value);
};

#endif
