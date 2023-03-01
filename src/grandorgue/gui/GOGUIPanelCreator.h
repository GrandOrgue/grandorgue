/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUIPANELCREATOR_H
#define GOGUIPANELCREATOR_H

class GOConfigReader;

class GOGUIPanelCreator {
public:
  virtual ~GOGUIPanelCreator() {}

  virtual void CreatePanels(GOConfigReader &cfg) = 0;
};

#endif
