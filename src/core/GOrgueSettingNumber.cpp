/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueSettingNumber.h"

#include "GOrgueConfigReader.h"
#include "GOrgueConfigWriter.h"

template<class T>
GOrgueSettingNumber<T>::GOrgueSettingNumber(GOrgueSettingStore* store, wxString group, wxString name, T min_value, T max_value, T default_value) :
	GOrgueSetting(store, group, name),
	m_Value(default_value),
	m_MinValue(min_value),
	m_MaxValue(max_value),
	m_DefaultValue(default_value)
{
}

template<class T>
void GOrgueSettingNumber<T>::Load(GOrgueConfigReader& cfg)
{
	(*this)(cfg.ReadInteger(CMBSetting, m_Group, m_Name, m_MinValue, m_MaxValue, false, m_DefaultValue));
}

template<class T>
void GOrgueSettingNumber<T>::Save(GOrgueConfigWriter& cfg)
{
	cfg.WriteInteger(m_Group, m_Name, m_Value);
}

template<class T>
void GOrgueSettingNumber<T>::setDefaultValue(T default_value)
{
	m_DefaultValue = default_value;
}
