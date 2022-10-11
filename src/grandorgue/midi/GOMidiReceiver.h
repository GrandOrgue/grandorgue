/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIRECEIVER_H
#define GOMIDIRECEIVER_H

#include "midi/GOMidiReceiverBase.h"

class GODefinitionFile;

class GOMidiReceiver : public GOMidiReceiverBase {
private:
  GODefinitionFile *m_organfile;
  int m_Index;

protected:
  void Preconfigure(GOConfigReader &cfg, wxString group);
  int GetTranspose();

public:
  GOMidiReceiver(GODefinitionFile *organfile, GOMidiReceiverType type);

  void Load(GOConfigReader &cfg, const wxString &group, GOMidiMap &map);

  void SetIndex(int index);

  void Assign(const GOMidiReceiverData &data);
};

#endif
