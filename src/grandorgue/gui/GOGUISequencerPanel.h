/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUISEQUENCERPANEL_H
#define GOGUISEQUENCERPANEL_H

#include "gui/GOGUIPanelCreator.h"

class GOGUIPanel;
class GOOrganController;

class GOGUISequencerPanel : public GOGUIPanelCreator {
private:
  GOOrganController *m_OrganController;

  GOGUIPanel *CreateSequencerPanel(GOConfigReader &cfg);

public:
  GOGUISequencerPanel(GOOrganController *organController);
  virtual ~GOGUISequencerPanel();

  void CreatePanels(GOConfigReader &cfg);
};

#endif
