/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUIRECORDERPANEL_H
#define GOGUIRECORDERPANEL_H

#include "gui/GOGUIPanelCreator.h"

class GOGUIPanel;
class GODefinitionFile;

class GOGUIRecorderPanel : public GOGUIPanelCreator {
private:
  GODefinitionFile *m_organfile;

  GOGUIPanel *CreateRecorderPanel(GOConfigReader &cfg);

public:
  GOGUIRecorderPanel(GODefinitionFile *organfile);
  virtual ~GOGUIRecorderPanel();

  void CreatePanels(GOConfigReader &cfg);
};

#endif
