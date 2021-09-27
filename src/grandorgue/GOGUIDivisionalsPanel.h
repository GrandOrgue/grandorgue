/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOGUIDIVISIONALSPANEL_H
#define GOGUIDIVISIONALSPANEL_H

#include "GOGUIPanelCreator.h"

class GOGUIPanel;
class GrandOrgueFile;

class GOGUIDivisionalsPanel : public GOGUIPanelCreator
{
private:
	GrandOrgueFile* m_organfile;

	GOGUIPanel* CreateDivisionalsPanel(GOrgueConfigReader& cfg);

public:
	GOGUIDivisionalsPanel(GrandOrgueFile* organfile);
	virtual ~GOGUIDivisionalsPanel();

	void CreatePanels(GOrgueConfigReader& cfg);
};

#endif
