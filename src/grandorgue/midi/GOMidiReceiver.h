/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIRECEIVER_H
#define GOMIDIRECEIVER_H

#include "midi/GOMidiReceiverBase.h"

class GOOrganController;

class GOMidiReceiver : public GOMidiReceiverBase {
private:
  GOOrganController *m_OrganController;
  int m_Index;

protected:
  void Preconfigure(GOConfigReader &cfg, wxString group);
  int GetTranspose();

public:
  GOMidiReceiver(GOOrganController *organController, GOMidiReceiverType type);

  void Load(GOConfigReader &cfg, const wxString &group, GOMidiMap &map);

  void SetIndex(int index);

  void Assign(const GOMidiReceiverEventPatternList &data);
};

#endif
