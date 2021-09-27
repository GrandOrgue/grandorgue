/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueSettingEnum.h"

#include "GOrgueConfigReader.h"
#include "GOrgueConfigWriter.h"

template<class T>
GOrgueSettingEnum<T>::GOrgueSettingEnum(GOrgueSettingStore* store, wxString group, wxString name, const struct IniFileEnumEntry* entries, unsigned count, T default_value) :
	GOrgueSetting(store, group, name),
	m_Value(default_value),
	m_DefaultValue(default_value),
	m_Entries(entries),
	m_Count(count)
{
}

template<class T>
void GOrgueSettingEnum<T>::Load(GOrgueConfigReader& cfg)
{
	(*this)((T)cfg.ReadEnum(CMBSetting, m_Group, m_Name, m_Entries, m_Count, false, (int)m_DefaultValue));
}

template<class T>
void GOrgueSettingEnum<T>::Save(GOrgueConfigWriter& cfg)
{
	cfg.WriteEnum(m_Group, m_Name, (int)m_Value, m_Entries, m_Count);
}

template<class T>
void GOrgueSettingEnum<T>::setDefaultValue(T default_value)
{
	m_DefaultValue = default_value;
}
