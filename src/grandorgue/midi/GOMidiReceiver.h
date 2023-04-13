/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIRECEIVER_H
#define GOMIDIRECEIVER_H

#include "midi/GOMidiReceiverBase.h"

class GOConfig;
class GOOrganModel;

class GOMidiReceiver : public GOMidiReceiverBase {
private:
  GOOrganModel &r_OrganModel;
  const GOConfig &r_config;
  int m_Index;

protected:
  void Preconfigure(GOConfigReader &cfg, wxString group) override;
  int GetTranspose() override;

public:
  GOMidiReceiver(GOOrganModel &organModel, GOMidiReceiverType type);

  void Load(GOConfigReader &cfg, const wxString &group, GOMidiMap &map);

  void SetIndex(int index) { m_Index = index; }
};

#endif
