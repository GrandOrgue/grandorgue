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
class GOVirtualCouplerController;

class GOGUICouplerPanel : public GOGUIPanelCreator {
private:
  GOOrganController *m_OrganController;
  const GOVirtualCouplerController &r_VirtualCouplers;

  GOGUIPanel *CreateCouplerPanel(GOConfigReader &cfg, unsigned manual_nr);

public:
  GOGUICouplerPanel(
    GOOrganController *organController,
    const GOVirtualCouplerController &virtualCouplers)
    : m_OrganController(organController), r_VirtualCouplers(virtualCouplers) {}

  void CreatePanels(GOConfigReader &cfg);
};

#endif
