/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOKEYRECEIVER_H
#define GOKEYRECEIVER_H

#include <wx/string.h>

#include "GOKeyReceiverData.h"

class GOConfigReader;
class GOConfigWriter;
class GODefinitionFile;

class GOKeyReceiver : public GOKeyReceiverData {
private:
  GODefinitionFile *m_organfile;

public:
  GOKeyReceiver(GODefinitionFile *organfile, KEY_RECEIVER_TYPE type);

  void Load(GOConfigReader &cfg, wxString group);
  void Save(GOConfigWriter &cfg, wxString group);

  KEY_MATCH_TYPE Match(unsigned key);

  void Assign(const GOKeyReceiverData &data);
};

#endif
