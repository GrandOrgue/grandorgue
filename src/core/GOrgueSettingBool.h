/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUESETTINGBOOL_H
#define GORGUESETTINGBOOL_H

#include "GOrgueSetting.h"

class GOrgueSettingBool : private GOrgueSetting
{
private:
	bool m_Value;
	bool m_DefaultValue;

	void Load(GOrgueConfigReader& cfg);
	void Save(GOrgueConfigWriter& cfg);

protected:
	virtual bool validate(bool value);

public:
	GOrgueSettingBool(GOrgueSettingStore* store, wxString group, wxString name, bool default_value);

	void setDefaultValue(bool default_value);

	bool operator() () const;
	void operator()(bool value);
};

#endif
