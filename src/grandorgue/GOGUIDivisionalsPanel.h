/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOGUIDIVISIONALSPANEL_H
#define GOGUIDIVISIONALSPANEL_H

#include "GOGUIPanelCreator.h"

class GOGUIPanel;
class GODefinitionFile;

class GOGUIDivisionalsPanel : public GOGUIPanelCreator
{
private:
	GODefinitionFile* m_organfile;

	GOGUIPanel* CreateDivisionalsPanel(GOConfigReader& cfg);

public:
	GOGUIDivisionalsPanel(GODefinitionFile* organfile);
	virtual ~GOGUIDivisionalsPanel();

	void CreatePanels(GOConfigReader& cfg);
};

#endif
