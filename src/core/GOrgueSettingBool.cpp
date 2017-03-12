/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2017 GrandOrgue contributors (see AUTHORS)
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
