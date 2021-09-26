/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueSettingStore.h"

#include "GOrgueSetting.h"

GOrgueSettingStore::GOrgueSettingStore() :
	m_SettingList()
{
}

GOrgueSettingStore::~GOrgueSettingStore()
{
}

void GOrgueSettingStore::AddSetting(GOrgueSetting* setting)
{
	m_SettingList.push_back(setting);
}


void GOrgueSettingStore::Load(GOrgueConfigReader& cfg)
{
	for(unsigned i = 0; i < m_SettingList.size(); i++)
		m_SettingList[i]->Load(cfg);
}

void GOrgueSettingStore::Save(GOrgueConfigWriter& cfg)
{
	for(unsigned i = 0; i < m_SettingList.size(); i++)
		m_SettingList[i]->Save(cfg);
}
