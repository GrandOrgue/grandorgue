/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOGUIMASTERPANEL_H
#define GOGUIMASTERPANEL_H

#include "GOGUIPanelCreator.h"

class GOGUIPanel;
class GODefinitionFile;

class GOGUIMasterPanel : public GOGUIPanelCreator
{
private:
	GODefinitionFile* m_organfile;

	GOGUIPanel* CreateMasterPanel(GOConfigReader& cfg);

public:
	GOGUIMasterPanel(GODefinitionFile* organfile);
	virtual ~GOGUIMasterPanel();

	void CreatePanels(GOConfigReader& cfg);
};

#endif
