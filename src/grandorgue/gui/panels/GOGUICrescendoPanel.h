/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUICRESCENDOPANEL_H
#define GOGUICRESCENDOPANEL_H

#include "GOGUIPanelCreator.h"

class GOGuiOrgan;
class GOGUIPanel;
class GOOrganController;

class GOGUICrescendoPanel : public GOGUIPanelCreator {
private:
  GOGuiOrgan &r_GuiOrgan;
  GOOrganController *m_OrganController;

  GOGUIPanel *CreateCrescendoPanel(GOConfigReader &cfg);

public:
  GOGUICrescendoPanel(GOGuiOrgan &guiOrgan);
  virtual ~GOGUICrescendoPanel();

  void CreatePanels(GOConfigReader &cfg);
};

#endif
