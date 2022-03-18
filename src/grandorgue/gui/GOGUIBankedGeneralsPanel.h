/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUIBANKEDGENERALSPANEL_H
#define GOGUIBANKEDGENERALSPANEL_H

#include "gui/GOGUIPanelCreator.h"

class GOGUIPanel;
class GODefinitionFile;

class GOGUIBankedGeneralsPanel : public GOGUIPanelCreator {
private:
  GODefinitionFile *m_organfile;

  GOGUIPanel *CreateBankedGeneralsPanel(GOConfigReader &cfg);

public:
  GOGUIBankedGeneralsPanel(GODefinitionFile *organfile);
  virtual ~GOGUIBankedGeneralsPanel();

  void CreatePanels(GOConfigReader &cfg);
};

#endif
