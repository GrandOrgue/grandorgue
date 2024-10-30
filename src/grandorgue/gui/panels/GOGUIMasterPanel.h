/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUIMASTERPANEL_H
#define GOGUIMASTERPANEL_H

#include "gui/GOGUIPanelCreator.h"

class GOGUIPanel;
class GOOrganController;

class GOGUIMasterPanel : public GOGUIPanelCreator {
private:
  GOOrganController *m_OrganController;

  GOGUIPanel *CreateMasterPanel(GOConfigReader &cfg);

public:
  GOGUIMasterPanel(GOOrganController *organController);
  virtual ~GOGUIMasterPanel();

  void CreatePanels(GOConfigReader &cfg);
};

#endif
