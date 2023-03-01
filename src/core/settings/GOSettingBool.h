/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTINGBOOL_H
#define GOSETTINGBOOL_H

#include "settings/GOSetting.h"

class GOSettingBool : private GOSetting {
private:
  bool m_Value;
  bool m_DefaultValue;
  // whether the value was read from the setting or was set by default
  bool m_IsPresent;

  void Load(GOConfigReader &cfg);
  void Save(GOConfigWriter &cfg);

protected:
  virtual bool validate(bool value);

public:
  GOSettingBool(
    GOSettingStore *store, wxString group, wxString name, bool default_value);

  bool IsPresent() const { return m_IsPresent; }
  void setDefaultValue(bool default_value);

  bool operator()() const;
  void operator()(bool value);
};

#endif
