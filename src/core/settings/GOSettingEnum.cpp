/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "settings/GOSettingEnum.h"

#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"

template <class T>
GOSettingEnum<T>::GOSettingEnum(
  GOSettingStore *store,
  wxString group,
  wxString name,
  const struct IniFileEnumEntry *entries,
  unsigned count,
  T default_value)
  : GOSetting(store, group, name),
    m_Value(default_value),
    m_DefaultValue(default_value),
    m_Entries(entries),
    m_Count(count) {}

template <class T> void GOSettingEnum<T>::Load(GOConfigReader &cfg) {
  (*this)((T)cfg.ReadEnum(
    CMBSetting,
    m_Group,
    m_Name,
    m_Entries,
    m_Count,
    false,
    (int)m_DefaultValue));
}

template <class T> void GOSettingEnum<T>::Save(GOConfigWriter &cfg) {
  cfg.WriteEnum(m_Group, m_Name, (int)m_Value, m_Entries, m_Count);
}

template <class T> void GOSettingEnum<T>::setDefaultValue(T default_value) {
  m_DefaultValue = default_value;
}
