/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueSetting.h"

#include "GOrgueSettingStore.h"

GOrgueSetting::GOrgueSetting(GOrgueSettingStore* store, wxString group, wxString name) :
	m_Group(group),
	m_Name(name)
{
	if (store)
		store->AddSetting(this);
}

GOrgueSetting::~GOrgueSetting()
{
}
