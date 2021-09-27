/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOGUIRECORDERPANEL_H
#define GOGUIRECORDERPANEL_H

#include "GOGUIPanelCreator.h"

class GOGUIPanel;
class GrandOrgueFile;

class GOGUIRecorderPanel : public GOGUIPanelCreator
{
private:
	GrandOrgueFile* m_organfile;

	GOGUIPanel* CreateRecorderPanel(GOrgueConfigReader& cfg);

public:
	GOGUIRecorderPanel(GrandOrgueFile* organfile);
	virtual ~GOGUIRecorderPanel();

	void CreatePanels(GOrgueConfigReader& cfg);
};

#endif
