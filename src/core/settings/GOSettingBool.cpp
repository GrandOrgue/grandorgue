/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "settings/GOSettingBool.h"

#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"

GOSettingBool::GOSettingBool(
  GOSettingStore *store, wxString group, wxString name, bool default_value)
  : GOSetting(store, group, name),
    m_Value(default_value),
    m_DefaultValue(default_value),
    m_IsPresent(false) {}

void GOSettingBool::Load(GOConfigReader &cfg) {
  int tripleValue = cfg.ReadBooleanTriple(CMBSetting, m_Group, m_Name, false);

  m_IsPresent = tripleValue >= 0;
  (*this)(m_IsPresent ? bool(tripleValue) : m_DefaultValue);
}

void GOSettingBool::Save(GOConfigWriter &cfg) {
  cfg.WriteBoolean(m_Group, m_Name, m_Value);
}

void GOSettingBool::setDefaultValue(bool default_value) {
  m_DefaultValue = default_value;
}

bool GOSettingBool::operator()() const { return m_Value; }

void GOSettingBool::operator()(bool value) { m_Value = validate(value); }

bool GOSettingBool::validate(bool value) { return value; }
