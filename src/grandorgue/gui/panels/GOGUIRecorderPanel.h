/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUIRECORDERPANEL_H
#define GOGUIRECORDERPANEL_H

#include "GOGUIPanelCreator.h"

class GOGuiOrgan;
class GOGUIPanel;
class GOOrganController;

class GOGUIRecorderPanel : public GOGUIPanelCreator {
private:
  GOGuiOrgan &r_GuiOrgan;
  GOOrganController *m_OrganController;

  GOGUIPanel *CreateRecorderPanel(GOConfigReader &cfg);

public:
  GOGUIRecorderPanel(GOGuiOrgan &guiOrgan);
  virtual ~GOGUIRecorderPanel();

  void CreatePanels(GOConfigReader &cfg);
};

#endif
