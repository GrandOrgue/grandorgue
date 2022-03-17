/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUISEQUENCERPANEL_H
#define GOGUISEQUENCERPANEL_H

#include "gui/GOGUIPanelCreator.h"

class GOGUIPanel;
class GODefinitionFile;

class GOGUISequencerPanel : public GOGUIPanelCreator {
private:
  GODefinitionFile *m_organfile;

  GOGUIPanel *CreateSequencerPanel(GOConfigReader &cfg);

public:
  GOGUISequencerPanel(GODefinitionFile *organfile);
  virtual ~GOGUISequencerPanel();

  void CreatePanels(GOConfigReader &cfg);
};

#endif
