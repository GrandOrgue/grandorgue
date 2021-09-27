/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOGUICRESCENDOPANEL_H
#define GOGUICRESCENDOPANEL_H

#include "GOGUIPanelCreator.h"

class GOGUIPanel;
class GrandOrgueFile;

class GOGUICrescendoPanel : public GOGUIPanelCreator
{
private:
	GrandOrgueFile* m_organfile;

	GOGUIPanel* CreateCrescendoPanel(GOrgueConfigReader& cfg);

public:
	GOGUICrescendoPanel(GrandOrgueFile* organfile);
	virtual ~GOGUICrescendoPanel();

	void CreatePanels(GOrgueConfigReader& cfg);
};

#endif
