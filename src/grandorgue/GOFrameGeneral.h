/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOFRAMEGENERAL_H
#define GOFRAMEGENERAL_H

#include <wx/string.h>

#include "GOCombination.h"
#include "GOSaveableObject.h"

class GOConfigReader;
class GOConfigWriter;
class GODefinitionFile;

class GOFrameGeneral : public GOCombination, private GOSaveableObject {
private:
  GODefinitionFile *m_organfile;
  bool m_IsSetter;

  void Save(GOConfigWriter &cfg);
  void LoadCombination(GOConfigReader &cfg);

public:
  GOFrameGeneral(
    GOCombinationDefinition &general_template,
    GODefinitionFile *organfile,
    bool is_setter);
  void Load(GOConfigReader &cfg, wxString group);
  void Push();
};

#endif /* GOFRAMEGENERAL_H */
