/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2016 GrandOrgue contributors (see AUTHORS)
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
