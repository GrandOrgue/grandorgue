/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOSETTINGBOOL_H
#define GOSETTINGBOOL_H

#include "GOSetting.h"

class GOSettingBool : private GOSetting
{
private:
	bool m_Value;
	bool m_DefaultValue;

	void Load(GOConfigReader& cfg);
	void Save(GOConfigWriter& cfg);

protected:
	virtual bool validate(bool value);

public:
	GOSettingBool(GOSettingStore* store, wxString group, wxString name, bool default_value);

	void setDefaultValue(bool default_value);

	bool operator() () const;
	void operator()(bool value);
};

#endif
