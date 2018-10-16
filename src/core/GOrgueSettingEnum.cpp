/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2018 GrandOrgue contributors (see AUTHORS)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
