/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUICOUPLERMANUALSANDVOLUMEPANEL_H
#define GOGUICOUPLERMANUALSANDVOLUMEPANEL_H

#include "GOGUIPanelCreator.h"

class GOGUIPanel;
class GOOrganController;

class GOGUICouplerManualsAndVolumePanel : public GOGUIPanelCreator {
private:
  GOOrganController *m_OrganController;

  GOGUIPanel *CreatePanel(GOConfigReader &cfg);

public:
  GOGUICouplerManualsAndVolumePanel(GOOrganController *organController);
  ~GOGUICouplerManualsAndVolumePanel();

  void CreatePanels(GOConfigReader &cfg);
};

#endif
