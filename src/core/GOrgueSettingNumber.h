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

#ifndef GORGUESETTINGNUMBER_H
#define GORGUESETTINGNUMBER_H

#include "GOrgueSetting.h"

template<class T>
class GOrgueSettingNumber : private GOrgueSetting
{
private:
	T m_Value;
	T m_MinValue;
	T m_MaxValue;
	T m_DefaultValue;

	void Load(GOrgueConfigReader& cfg);
	void Save(GOrgueConfigWriter& cfg);

protected:
	virtual T validate(T value)
	{
		if (value <= m_MinValue)
			value = m_MinValue;
		if (value >= m_MaxValue)
			value = m_MaxValue;
		return value;
	}

public:
	GOrgueSettingNumber(GOrgueSettingStore* store, wxString group, wxString name, T min_value, T max_value, T default_value);

	void setDefaultValue(T default_value);

	T operator() () const
	{
		return m_Value;
	}

	void operator()(T value)
	{
		m_Value = validate(value);
	}
};

typedef GOrgueSettingNumber<unsigned> GOrgueSettingUnsigned;
typedef GOrgueSettingNumber<int> GOrgueSettingInteger;

#endif
