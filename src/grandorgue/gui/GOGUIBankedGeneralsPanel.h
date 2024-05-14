/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUIBANKEDGENERALSPANEL_H
#define GOGUIBANKEDGENERALSPANEL_H

#include "gui/GOGUIPanelCreator.h"

class GOGUIPanel;
class GOOrganController;

class GOGUIBankedGeneralsPanel : public GOGUIPanelCreator {
private:
  GOOrganController *m_OrganController;

  GOGUIPanel *CreateBankedGeneralsPanel(GOConfigReader &cfg);

public:
  GOGUIBankedGeneralsPanel(GOOrganController *organController);
  virtual ~GOGUIBankedGeneralsPanel();

  void CreatePanels(GOConfigReader &cfg);
};

#endif
