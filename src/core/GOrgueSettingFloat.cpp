/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueSettingFloat.h"

#include "GOrgueConfigReader.h"
#include "GOrgueConfigWriter.h"

GOrgueSettingFloat::GOrgueSettingFloat(GOrgueSettingStore* store, wxString group, wxString name, float min_value, float max_value, float default_value) :
	GOrgueSetting(store, group, name),
	m_Value(default_value),
	m_MinValue(min_value),
	m_MaxValue(max_value),
	m_DefaultValue(default_value)
{
}

void GOrgueSettingFloat::Load(GOrgueConfigReader& cfg)
{
	(*this)(cfg.ReadFloat(CMBSetting, m_Group, m_Name, m_MinValue, m_MaxValue, false, m_DefaultValue));
}

void GOrgueSettingFloat::Save(GOrgueConfigWriter& cfg)
{
	cfg.WriteFloat(m_Group, m_Name, m_Value);
}

void GOrgueSettingFloat::setDefaultValue(float default_value)
{
	m_DefaultValue = default_value;
}

float GOrgueSettingFloat::operator() () const
{
	return m_Value;
}

void GOrgueSettingFloat::operator()(float value)
{
	m_Value = validate(value);
}

float GOrgueSettingFloat::validate(float value)
{
	if (value <= m_MinValue)
		value = m_MinValue;
	if (value >= m_MaxValue)
		value = m_MaxValue;
	return value;
}
