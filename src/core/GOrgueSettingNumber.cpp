/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
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
