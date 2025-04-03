/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "settings/GOSettingEnum.h"

#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"

class GOConfigEnum;

template <class T>
GOSettingEnum<T>::GOSettingEnum(
  GOSettingStore *store,
  const wxString &group,
  const wxString &name,
  const GOConfigEnum &configEnum,
  T default_value)
  : GOSetting(store, group, name),
    r_enum(configEnum),
    m_Value(default_value),
    m_DefaultValue(default_value) {}

template <class T> void GOSettingEnum<T>::Load(GOConfigReader &cfg) {
  (*this)((T)cfg.ReadEnum(
    CMBSetting, m_Group, m_Name, r_enum, false, (int)m_DefaultValue));
}

template <class T> void GOSettingEnum<T>::Save(GOConfigWriter &cfg) {
  cfg.WriteEnum(m_Group, m_Name, r_enum, (int)m_Value);
}

template <class T> void GOSettingEnum<T>::setDefaultValue(T default_value) {
  m_DefaultValue = default_value;
}
