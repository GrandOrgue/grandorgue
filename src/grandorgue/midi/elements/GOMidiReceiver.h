/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIRECEIVER_H
#define GOMIDIRECEIVER_H

#include "GOMidiReceiverBase.h"

class GOConfig;
class GOOrganModel;

class GOMidiReceiver : public GOMidiReceiverBase {
private:
  const GOConfig &r_config;

protected:
  int GetTranspose() const override;

public:
  GOMidiReceiver(GOOrganModel &organModel, GOMidiReceiverType type);

  using GOMidiReceiverBase::Load;
  void Load(GOConfigReader &cfg, const wxString &group, GOMidiMap &map);
};

#endif
