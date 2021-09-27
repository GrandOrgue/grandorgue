/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUESETTING_H
#define GORGUESETTING_H

#include <wx/string.h>

class GOrgueConfigReader;
class GOrgueConfigWriter;
class GOrgueSettingStore;

class GOrgueSetting
{
private:
	GOrgueSetting(const GOrgueSetting&);
	void operator=(const GOrgueSetting&);

protected:
	wxString m_Group;
	wxString m_Name;

public:
	GOrgueSetting(GOrgueSettingStore* store, wxString group, wxString name);
	virtual ~GOrgueSetting();

	virtual void Load(GOrgueConfigReader& cfg) = 0;
	virtual void Save(GOrgueConfigWriter& cfg) = 0;
};

#endif
