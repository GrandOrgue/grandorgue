/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOGUIFLOATINGPANEL_H
#define GOGUIFLOATINGPANEL_H

#include "GOGUIPanelCreator.h"

class GOGUIPanel;
class GODefinitionFile;

class GOGUIFloatingPanel : public GOGUIPanelCreator
{
private:
	GODefinitionFile* m_organfile;

	GOGUIPanel* CreateFloatingPanel(GOConfigReader& cfg);

public:
	GOGUIFloatingPanel(GODefinitionFile* organfile);
	~GOGUIFloatingPanel();

	void CreatePanels(GOConfigReader& cfg);
};

#endif
