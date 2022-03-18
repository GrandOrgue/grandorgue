/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GGUICOUPLERPANEL_H
#define GGUICOUPLERPANEL_H

#include "gui/GOGUIPanelCreator.h"

class GOGUIPanel;
class GODefinitionFile;

class GOGUICouplerPanel : public GOGUIPanelCreator {
private:
  GODefinitionFile *m_organfile;

  GOGUIPanel *CreateCouplerPanel(GOConfigReader &cfg, unsigned manual_nr);

public:
  GOGUICouplerPanel(GODefinitionFile *organfile);
  ~GOGUICouplerPanel();

  void CreatePanels(GOConfigReader &cfg);
};

#endif
