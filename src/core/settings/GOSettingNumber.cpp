/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "settings/GOSettingNumber.h"

#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"

template <class T>
GOSettingNumber<T>::GOSettingNumber(
  GOSettingStore *store,
  wxString group,
  wxString name,
  T min_value,
  T max_value,
  T default_value)
  : GOSetting(store, group, name),
    m_Value(default_value),
    m_MinValue(min_value),
    m_MaxValue(max_value),
    m_DefaultValue(default_value) {}

template <class T> void GOSettingNumber<T>::Load(GOConfigReader &cfg) {
  (*this)(cfg.ReadInteger(
    CMBSetting,
    m_Group,
    m_Name,
    m_MinValue,
    m_MaxValue,
    false,
    m_DefaultValue));
}

template <class T> void GOSettingNumber<T>::Save(GOConfigWriter &cfg) {
  cfg.WriteInteger(m_Group, m_Name, m_Value);
}

template <class T> void GOSettingNumber<T>::setDefaultValue(T default_value) {
  m_DefaultValue = default_value;
}
