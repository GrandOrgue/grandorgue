/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "settings/GOSettingFloat.h"

#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"

GOSettingFloat::GOSettingFloat(
  GOSettingStore *store,
  wxString group,
  wxString name,
  float min_value,
  float max_value,
  float default_value)
  : GOSetting(store, group, name),
    m_Value(default_value),
    m_MinValue(min_value),
    m_MaxValue(max_value),
    m_DefaultValue(default_value) {}

void GOSettingFloat::Load(GOConfigReader &cfg) {
  (*this)(cfg.ReadFloat(
    CMBSetting,
    m_Group,
    m_Name,
    m_MinValue,
    m_MaxValue,
    false,
    m_DefaultValue));
}

void GOSettingFloat::Save(GOConfigWriter &cfg) {
  cfg.WriteFloat(m_Group, m_Name, m_Value);
}

void GOSettingFloat::setDefaultValue(float default_value) {
  m_DefaultValue = default_value;
}

float GOSettingFloat::operator()() const { return m_Value; }

void GOSettingFloat::operator()(float value) { m_Value = validate(value); }

float GOSettingFloat::validate(float value) {
  if (value <= m_MinValue)
    value = m_MinValue;
  if (value >= m_MaxValue)
    value = m_MaxValue;
  return value;
}
