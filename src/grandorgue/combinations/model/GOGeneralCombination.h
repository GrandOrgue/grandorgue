/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGENERALCOMBINATION_H
#define GOGENERALCOMBINATION_H

#include <wx/string.h>

#include "GOCombination.h"
#include "GOSaveableObject.h"

class GOConfigReader;
class GOConfigWriter;
class GOOrganController;

class GOGeneralCombination : public GOCombination, private GOSaveableObject {
private:
  GOOrganController *m_OrganController;
  bool m_IsSetter;

  void Save(GOConfigWriter &cfg);
  void LoadCombination(GOConfigReader &cfg);

public:
  GOGeneralCombination(
    GOCombinationDefinition &general_template,
    GOOrganController *organController,
    bool is_setter);
  void Load(GOConfigReader &cfg, wxString group);

  /*
   * Activate this combination
   * If extraSet is passed then not to disable stops that are present in
   * extraSet
   * If isFromCrescendo and extraSet is passed then does not depress other
   * buttons
   */
  void Push(
    ExtraElementsSet const *extraSet = nullptr, bool isFromCrescendo = false);
};

#endif /* GOGENERALCOMBINATION_H */
