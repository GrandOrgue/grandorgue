/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOGUISEQUENCERPANEL_H
#define GOGUISEQUENCERPANEL_H

#include "GOGUIPanelCreator.h"

class GOGUIPanel;
class GrandOrgueFile;

class GOGUISequencerPanel : public GOGUIPanelCreator
{
private:
	GrandOrgueFile* m_organfile;

	GOGUIPanel* CreateSequencerPanel(GOrgueConfigReader& cfg);

public:
	GOGUISequencerPanel(GrandOrgueFile* organfile);
	virtual ~GOGUISequencerPanel();

	void CreatePanels(GOrgueConfigReader& cfg);
};

#endif
