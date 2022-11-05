/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GODIVISIONALCOMBINATION_H
#define GODIVISIONALCOMBINATION_H

#include <wx/string.h>

#include "GOCombination.h"

class GOConfigReader;
class GOConfigWriter;
class GOOrganController;

class GODivisionalCombination : public GOCombination {
protected:
  GOOrganController *m_OrganController;
  wxString m_group;
  unsigned m_odfManualNumber;
  int m_DivisionalNumber;
  bool m_IsSetter;

public:
  GODivisionalCombination(
    GOOrganController *organController,
    GOCombinationDefinition &divisionalTemplate,
    bool isSetter);

  unsigned GetManualNumber() const { return m_odfManualNumber; }
  int GetDivisionalNumber() const { return m_DivisionalNumber; }

  void Init(const wxString &group, int manualNumber, int divisionalNumber);
  void Load(
    GOConfigReader &cfg,
    wxString group,
    int manualNumber,
    int divisionalNumber);
  // It does not inherit GOSaveableOblect because
  // GOdivisionalSetter::LoadCombination creates the combinations dynamically
  void LoadCombination(GOConfigReader &cfg);
  void Save(GOConfigWriter &cfg);
  void Push(ExtraElementsSet const *extraSet = nullptr);

  wxString GetMidiType();

  // checks if the combination exists in the config file
  // returns the loaded combination if it exists else returns nullptr
  static GODivisionalCombination *LoadFrom(
    GOOrganController *organController,
    GOConfigReader &cfg,
    GOCombinationDefinition &divisional_template,
    const wxString &group,
    // for compatibility with the old preset: load the combination with the old
    // group name
    const wxString &readGroup,
    int manualNumber,
    int divisionalNumber);
};

#endif
