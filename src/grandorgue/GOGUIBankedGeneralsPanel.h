/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOGUIBANKEDGENERALSPANEL_H
#define GOGUIBANKEDGENERALSPANEL_H

#include "GOGUIPanelCreator.h"

class GOGUIPanel;
class GrandOrgueFile;

class GOGUIBankedGeneralsPanel : public GOGUIPanelCreator
{
private:
	GrandOrgueFile* m_organfile;

	GOGUIPanel* CreateBankedGeneralsPanel(GOrgueConfigReader& cfg);

public:
	GOGUIBankedGeneralsPanel(GrandOrgueFile* organfile);
	virtual ~GOGUIBankedGeneralsPanel();

	void CreatePanels(GOrgueConfigReader& cfg);
};

#endif
