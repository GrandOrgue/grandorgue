/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTINGENUM_H
#define GOSETTINGENUM_H

#include "settings/GOSetting.h"

template <class T> class GOSettingEnum : private GOSetting {
private:
  T m_Value;
  T m_DefaultValue;
  const struct IniFileEnumEntry *m_Entries;
  unsigned m_Count;

  void Load(GOConfigReader &cfg);
  void Save(GOConfigWriter &cfg);

public:
  GOSettingEnum(
    GOSettingStore *store,
    wxString group,
    wxString name,
    const struct IniFileEnumEntry *entries,
    unsigned count,
    T default_value);

  void setDefaultValue(T default_value);

  T operator()() const { return m_Value; }

  void operator()(T value) { m_Value = value; }
};

#endif
