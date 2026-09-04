/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GGUICOUPLERPANEL_H
#define GGUICOUPLERPANEL_H

#include "GOGUIPanelCreator.h"

class GOGuiOrgan;
class GOGUIPanel;
class GOOrganController;
class GOVirtualCouplerController;

class GOGUICouplerPanel : public GOGUIPanelCreator {
private:
  GOGuiOrgan &r_GuiOrgan;
  GOOrganController *m_OrganController;
  const GOVirtualCouplerController &r_VirtualCouplers;

  GOGUIPanel *CreateCouplerPanel(GOConfigReader &cfg, unsigned manual_nr);

public:
  GOGUICouplerPanel(
    GOGuiOrgan &guiOrgan, const GOVirtualCouplerController &virtualCouplers);

  void CreatePanels(GOConfigReader &cfg);
};

#endif
