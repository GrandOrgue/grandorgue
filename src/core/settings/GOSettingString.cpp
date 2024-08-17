/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "settings/GOSettingString.h"

#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"

GOSettingString::GOSettingString(
  GOSettingStore *store,
  const wxString &group,
  const wxString &name,
  const wxString &defaultValue)
  : GOSetting(store, group, name),
    m_Value(defaultValue),
    m_DefaultValue(defaultValue) {}

void GOSettingString::Load(GOConfigReader &cfg) {
  (*this)(cfg.ReadString(CMBSetting, m_Group, m_Name, false, m_DefaultValue));
}

void GOSettingString::Save(GOConfigWriter &cfg) {
  cfg.WriteString(m_Group, m_Name, m_Value);
}
