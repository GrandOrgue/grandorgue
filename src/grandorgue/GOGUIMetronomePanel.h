/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOGUIMETRONOMEPANEL_H
#define GOGUIMETRONOMEPANEL_H

#include "GOGUIPanelCreator.h"

class GOGUIPanel;
class GrandOrgueFile;

class GOGUIMetronomePanel : public GOGUIPanelCreator
{
private:
	GrandOrgueFile* m_organfile;

	GOGUIPanel* CreateMetronomePanel(GOrgueConfigReader& cfg);

public:
	GOGUIMetronomePanel(GrandOrgueFile* organfile);
	virtual ~GOGUIMetronomePanel();

	void CreatePanels(GOrgueConfigReader& cfg);
};

#endif
