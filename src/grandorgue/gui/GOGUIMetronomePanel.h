/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUIMETRONOMEPANEL_H
#define GOGUIMETRONOMEPANEL_H

#include "gui/GOGUIPanelCreator.h"

class GOGUIPanel;
class GODefinitionFile;

class GOGUIMetronomePanel : public GOGUIPanelCreator {
 private:
  GODefinitionFile* m_organfile;

  GOGUIPanel* CreateMetronomePanel(GOConfigReader& cfg);

 public:
  GOGUIMetronomePanel(GODefinitionFile* organfile);
  virtual ~GOGUIMetronomePanel();

  void CreatePanels(GOConfigReader& cfg);
};

#endif
