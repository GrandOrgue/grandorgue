/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUESETTINGENUM_H
#define GORGUESETTINGENUM_H

#include "GOrgueSetting.h"

template<class T>
class GOrgueSettingEnum : private GOrgueSetting
{
private:
	T m_Value;
	T m_DefaultValue;
	const struct IniFileEnumEntry* m_Entries;
	unsigned m_Count;

	void Load(GOrgueConfigReader& cfg);
	void Save(GOrgueConfigWriter& cfg);

public:
	GOrgueSettingEnum(GOrgueSettingStore* store, wxString group, wxString name, const struct IniFileEnumEntry* entries, unsigned count, T default_value);

	void setDefaultValue(T default_value);

	T operator() () const
	{
		return m_Value;
	}

	void operator()(T value)
	{
		m_Value = value;
	}
};

#endif
