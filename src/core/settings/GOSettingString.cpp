/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "settings/GOSettingString.h"

#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"

GOSettingString::GOSettingString(
  GOSettingStore *store, wxString group, wxString name, wxString default_value)
  : GOSetting(store, group, name),
    m_Value(default_value),
    m_DefaultValue(default_value) {}

void GOSettingString::Load(GOConfigReader &cfg) {
  (*this)(cfg.ReadString(CMBSetting, m_Group, m_Name, false, m_DefaultValue));
}

void GOSettingString::Save(GOConfigWriter &cfg) {
  cfg.WriteString(m_Group, m_Name, m_Value);
}

void GOSettingString::setDefaultValue(wxString default_value) {
  m_DefaultValue = default_value;
}

wxString GOSettingString::getDefaultValue() { return m_DefaultValue; }

wxString GOSettingString::operator()() const { return m_Value; }

void GOSettingString::operator()(wxString value) { m_Value = validate(value); }

wxString GOSettingString::validate(wxString value) { return value; }
