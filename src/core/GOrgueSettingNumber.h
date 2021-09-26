/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
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
