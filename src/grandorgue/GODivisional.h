/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GODIVISIONAL_H
#define GODIVISIONAL_H

#include "GOCombination.h"
#include "GOPushbutton.h"

class GOConfigReader;
class GOConfigWriter;

class GODivisional : public GOPushbutton, public GOCombination {
protected:
  int m_DivisionalNumber;
  unsigned m_ManualNumber;
  bool m_IsSetter;

  bool PushLocal();

public:
  GODivisional(
    GODefinitionFile *organfile,
    GOCombinationDefinition &divisional_template,
    bool is_setter);
  void Init(
    GOConfigReader &cfg,
    wxString group,
    int manualNumber,
    int divisionalNumber,
    wxString name);
  void Load(
    GOConfigReader &cfg,
    wxString group,
    int manualNumber,
    int divisionalNumber);
  void LoadCombination(GOConfigReader &cfg);
  void Save(GOConfigWriter &cfg);
  void Push();

  wxString GetMidiType();
};

#endif
