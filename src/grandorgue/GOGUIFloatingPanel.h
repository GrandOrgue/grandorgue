/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOGUIFLOATINGPANEL_H
#define GOGUIFLOATINGPANEL_H

#include "GOGUIPanelCreator.h"

class GOGUIPanel;
class GrandOrgueFile;

class GOGUIFloatingPanel : public GOGUIPanelCreator
{
private:
	GrandOrgueFile* m_organfile;

	GOGUIPanel* CreateFloatingPanel(GOrgueConfigReader& cfg);

public:
	GOGUIFloatingPanel(GrandOrgueFile* organfile);
	~GOGUIFloatingPanel();

	void CreatePanels(GOrgueConfigReader& cfg);
};

#endif
