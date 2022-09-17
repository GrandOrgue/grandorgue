/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GODIVISIONALBUTTONCONTROL_H
#define GODIVISIONALBUTTONCONTROL_H

#include "combinations/model/GOCombination.h"
#include "control/GOPushbuttonControl.h"

class GOConfigReader;
class GOConfigWriter;

class GODivisionalButtonControl : public GOPushbuttonControl,
                                  public GOCombination {
protected:
  int m_DivisionalNumber;
  unsigned m_ManualNumber;
  bool m_IsSetter;

  bool PushLocal();

public:
  GODivisionalButtonControl(
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
