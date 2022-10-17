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
class GODefinitionFile;

class GODivisionalCombination : public GOCombination {
protected:
  GODefinitionFile *m_organfile;
  wxString m_group;
  unsigned m_ManualNumber;
  int m_DivisionalNumber;
  bool m_IsSetter;

public:
  GODivisionalCombination(
    GODefinitionFile *organfile,
    GOCombinationDefinition &divisionalTemplate,
    bool isSetter);

  unsigned GetManualNumber() const { return m_ManualNumber; }
  int GetDivisionalNumber() const { return m_DivisionalNumber; }

  void Init(const wxString &group, int manualNumber, int divisionalNumber);
  void Load(
    GOConfigReader &cfg,
    wxString group,
    int manualNumber,
    int divisionalNumber);
  void LoadCombination(GOConfigReader &cfg);
  void Save(GOConfigWriter &cfg);
  void Push(ExtraElementsSet const *extraSet = nullptr);

  wxString GetMidiType();

  static GODivisionalCombination *LoadFrom(
    GODefinitionFile *organfile,
    GOConfigReader &cfg,
    GOCombinationDefinition &divisional_template,
    const wxString &group,
    const wxString &readGroup,
    int manualNumber,
    int divisionalNumber);
};

#endif
