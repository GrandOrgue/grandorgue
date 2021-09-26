/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueSettingBool.h"

#include "GOrgueConfigReader.h"
#include "GOrgueConfigWriter.h"

GOrgueSettingBool::GOrgueSettingBool(GOrgueSettingStore* store, wxString group, wxString name, bool default_value) :
	GOrgueSetting(store, group, name),
	m_Value(default_value),
	m_DefaultValue(default_value)
{
}

void GOrgueSettingBool::Load(GOrgueConfigReader& cfg)
{
	(*this)(cfg.ReadBoolean(CMBSetting, m_Group, m_Name, false, m_DefaultValue));
}

void GOrgueSettingBool::Save(GOrgueConfigWriter& cfg)
{
	cfg.WriteBoolean(m_Group, m_Name, m_Value);
}

void GOrgueSettingBool::setDefaultValue(bool default_value)
{
	m_DefaultValue = default_value;
}

bool GOrgueSettingBool::operator() () const
{
	return m_Value;
}

void GOrgueSettingBool::operator()(bool value)
{
	m_Value = validate(value);
}

bool GOrgueSettingBool::validate(bool value)
{
	return value;
}
