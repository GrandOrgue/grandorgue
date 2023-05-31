/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GODIVISIONALBUTTONCONTROL_H
#define GODIVISIONALBUTTONCONTROL_H

#include "combinations/model/GODivisionalCombination.h"
#include "control/GOPushbuttonControl.h"

class GOConfigReader;
class GOSetter;

class GODivisionalButtonControl : public GOPushbuttonControl {
private:
  GOSetter &r_setter;
  GODivisionalCombination m_combination;

public:
  GODivisionalButtonControl(
    GOOrganController *organController,
    GOCombinationDefinition &divisionalTemplate,
    bool isSetter);

  GODivisionalCombination &GetCombination() { return m_combination; }
  wxString GetMidiType() override;

  void Init(
    GOConfigReader &cfg,
    const wxString &group,
    int manualNumber,
    int divisionalNumber,
    const wxString &name);

  void Load(
    GOConfigReader &cfg,
    const wxString &group,
    int manualNumber,
    int divisionalNumber);

  void LoadCombination(GOConfigReader &cfg);
  void Save(GOConfigWriter &cfg);

  void Push() override;
};

#endif /* GODIVISIONALBUTTONCONTROL_H */
