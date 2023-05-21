/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGENERALCOMBINATION_H
#define GOGENERALCOMBINATION_H

#include <wx/string.h>

#include "GOCombination.h"

class GOConfigReader;
class GOConfigWriter;
class GOOrganController;
class GOSetter;

class GOGeneralCombination : public GOCombination {
private:
  GOOrganController *m_OrganController;
  bool m_IsSetter;

  void LoadCombinationInt(GOConfigReader &cfg, GOSettingType srcType) override;
  void SaveInt(GOConfigWriter &cfg) override;

  void PutElementToYamlMap(
    const GOCombinationDefinition::Element &e,
    const wxString &valueLabel,
    const unsigned objectIndex,
    YAML::Node &yamlMap) const override;

  /**
   * Loads the combination from the Yaml Node. Called from FromYaml
   * @param node
   */
  void FromYamlMap(const YAML::Node &yamlMap) override;

public:
  GOGeneralCombination(
    GOCombinationDefinition &general_template,
    GOOrganController *organController,
    bool is_setter);
  void Load(GOConfigReader &cfg, wxString group);
};

#endif /* GOGENERALCOMBINATION_H */
