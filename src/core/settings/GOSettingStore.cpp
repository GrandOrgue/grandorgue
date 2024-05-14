/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "settings/GOSettingStore.h"

#include "settings/GOSetting.h"

GOSettingStore::GOSettingStore() : m_SettingList() {}

GOSettingStore::~GOSettingStore() {}

void GOSettingStore::AddSetting(GOSetting *setting) {
  m_SettingList.push_back(setting);
}

void GOSettingStore::Load(GOConfigReader &cfg) {
  for (unsigned i = 0; i < m_SettingList.size(); i++)
    m_SettingList[i]->Load(cfg);
}

void GOSettingStore::Save(GOConfigWriter &cfg) {
  for (unsigned i = 0; i < m_SettingList.size(); i++)
    m_SettingList[i]->Save(cfg);
}
