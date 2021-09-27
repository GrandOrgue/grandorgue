/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUESETTINGFLOAT_H
#define GORGUESETTINGFLOAT_H

#include "GOrgueSetting.h"

class GOrgueSettingFloat : private GOrgueSetting
{
private:
	float m_Value;
	float m_MinValue;
	float m_MaxValue;
	float m_DefaultValue;

	void Load(GOrgueConfigReader& cfg);
	void Save(GOrgueConfigWriter& cfg);

protected:
	virtual float validate(float value);

public:
	GOrgueSettingFloat(GOrgueSettingStore* store, wxString group, wxString name, float min_value, float max_value, float default_value);

	void setDefaultValue(float default_value);

	float operator() () const;
	void operator()(float value);
};

#endif
