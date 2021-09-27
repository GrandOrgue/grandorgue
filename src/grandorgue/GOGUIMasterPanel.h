/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOGUIMASTERPANEL_H
#define GOGUIMASTERPANEL_H

#include "GOGUIPanelCreator.h"

class GOGUIPanel;
class GrandOrgueFile;

class GOGUIMasterPanel : public GOGUIPanelCreator
{
private:
	GrandOrgueFile* m_organfile;

	GOGUIPanel* CreateMasterPanel(GOrgueConfigReader& cfg);

public:
	GOGUIMasterPanel(GrandOrgueFile* organfile);
	virtual ~GOGUIMasterPanel();

	void CreatePanels(GOrgueConfigReader& cfg);
};

#endif
