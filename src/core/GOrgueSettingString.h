/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUESETTINGSTRING_H
#define GORGUESETTINGSTRING_H

#include "GOrgueSetting.h"

class GOrgueSettingString : private GOrgueSetting
{
private:
	wxString m_Value;
	wxString m_DefaultValue;

	void Load(GOrgueConfigReader& cfg);
	void Save(GOrgueConfigWriter& cfg);

protected:
	wxString getDefaultValue();
	virtual wxString validate(wxString value);

public:
	GOrgueSettingString(GOrgueSettingStore* store, wxString group, wxString name, wxString default_value);

	void setDefaultValue(wxString default_value);

	wxString operator() () const;
	void operator()(wxString value);
};

#endif
