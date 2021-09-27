/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUESAVEABLE_H
#define GORGUESAVEABLE_H

#include <wx/string.h>

class GOrgueConfigReader;
class GOrgueConfigWriter;

class GOrgueSaveableObject
{
protected:
	wxString m_group;

public:
	GOrgueSaveableObject() :
		m_group()
	{
	}

	virtual ~GOrgueSaveableObject()
	{
	}

	virtual void Save(GOrgueConfigWriter& cfg) = 0;
	virtual void LoadCombination(GOrgueConfigReader& cfg)
	{
	}
};

#endif
