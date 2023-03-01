/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GGUICOUPLERPANEL_H
#define GGUICOUPLERPANEL_H

#include "gui/GOGUIPanelCreator.h"

class GOGUIPanel;
class GOOrganController;

class GOGUICouplerPanel : public GOGUIPanelCreator {
private:
  GOOrganController *m_OrganController;

  GOGUIPanel *CreateCouplerPanel(GOConfigReader &cfg, unsigned manual_nr);

public:
  GOGUICouplerPanel(GOOrganController *organController);
  ~GOGUICouplerPanel();

  void CreatePanels(GOConfigReader &cfg);
};

#endif
