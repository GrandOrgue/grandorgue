/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOGUIPANELCREATOR_H
#define GOGUIPANELCREATOR_H

class GOrgueConfigReader;

class GOGUIPanelCreator
{
public:
	virtual ~GOGUIPanelCreator()
	{
	}

	virtual void CreatePanels(GOrgueConfigReader& cfg) = 0;
};

#endif
