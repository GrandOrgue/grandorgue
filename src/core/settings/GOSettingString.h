/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
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
  const wxString &GetDefaultValue() const { return m_DefaultValue; }
  virtual wxString Validate(const wxString &value) const { return value; }

public:
  GOSettingString(
    GOSettingStore *store,
    const wxString &group,
    const wxString &name,
    const wxString &defaultValue);

  void SetDefaultValue(const wxString &defaultValue) {
    m_DefaultValue = defaultValue;
  }

  const wxString &operator()() const { return m_Value; }
  void operator()(const wxString &value) { m_Value = Validate(value); }
};

#endif
