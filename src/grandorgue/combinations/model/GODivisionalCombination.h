/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
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
  unsigned m_odfManualNumber;
  int m_DivisionalNumber;
  bool m_IsSetter;

  // It is not registered as saveable object because
  // GOdivisionalSetter::LoadCombination creates the combinations dynamically
  void LoadCombinationInt(GOConfigReader &cfg, GOSettingType srcType) override;
  void SaveInt(GOConfigWriter &cfg);

  void PutElementToYamlMap(
    const GOCombinationDefinition::Element &e,
    const wxString &valueLabel,
    const unsigned objectIndex,
    YAML::Node &yamlMap) const override;

  /**
   * Loads the combination from the Yaml Node
   * @param yamlNode - a YAML node to load from
   */
  void FromYamlMap(const YAML::Node &yamlMap) override;

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

  bool Push(ExtraElementsSet const *extraSet = nullptr);

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
