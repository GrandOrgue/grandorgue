/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUIDIVISIONALSPANEL_H
#define GOGUIDIVISIONALSPANEL_H

#include "gui/GOGUIPanelCreator.h"

class GOGUIPanel;
class GOOrganController;

class GOGUIDivisionalsPanel : public GOGUIPanelCreator {
private:
  GOOrganController *m_OrganController;

  GOGUIPanel *CreateDivisionalsPanel(GOConfigReader &cfg);

public:
  GOGUIDivisionalsPanel(GOOrganController *organController);
  virtual ~GOGUIDivisionalsPanel();

  void CreatePanels(GOConfigReader &cfg);
};

#endif
