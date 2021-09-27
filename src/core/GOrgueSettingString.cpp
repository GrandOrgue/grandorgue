/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueSettingString.h"

#include "GOrgueConfigReader.h"
#include "GOrgueConfigWriter.h"

GOrgueSettingString::GOrgueSettingString(GOrgueSettingStore* store, wxString group, wxString name, wxString default_value) :
	GOrgueSetting(store, group, name),
	m_Value(default_value),
	m_DefaultValue(default_value)
{
}

void GOrgueSettingString::Load(GOrgueConfigReader& cfg)
{
	(*this)(cfg.ReadString(CMBSetting, m_Group, m_Name, false, m_DefaultValue));
}

void GOrgueSettingString::Save(GOrgueConfigWriter& cfg)
{
	cfg.WriteString(m_Group, m_Name, m_Value);
}

void GOrgueSettingString::setDefaultValue(wxString default_value)
{
	m_DefaultValue = default_value;
}

wxString GOrgueSettingString::getDefaultValue()
{
	return m_DefaultValue;
}

wxString GOrgueSettingString::operator() () const
{
	return m_Value;
}

void GOrgueSettingString::operator()(wxString value)
{
	m_Value = validate(value);
}

wxString GOrgueSettingString::validate(wxString value)
{
	return value;
}
