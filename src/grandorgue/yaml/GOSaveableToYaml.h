/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSAVEABLETOYAML_H
#define GOSAVEABLETOYAML_H

#include <wx/string.h>
#include <yaml-cpp/yaml.h>

class GOSaveableToYaml {
public:
  /**
   * Save the combination to the YAML object
   */
  virtual void ToYaml(YAML::Node &yamlNode) const = 0;

  /**
   * Loads the object from the saved object in Yaml
   * @param yamlNode - a node with the saved object
   */
  virtual void FromYaml(const YAML::Node &yamlNode) = 0;

  /**
   * Converts the object to YAML
   * @return a node with the saved object
   */
  YAML::Node ToYamlNode() const;

  operator YAML::Node() const { return ToYamlNode(); }
};

/**
 * Save the object into the YAML node without cleaning it, so several objects
 *   may be saved to the same YAML node
 * @param yamlNode to save the object to
 * @param saveableObj to save
 * @return yamlNode
 */
YAML::Node &operator<<(
  YAML::Node &yamlNode, const GOSaveableToYaml &saveableObj);

/**
 * Load the object from the yaml
 * @param yamlNode a YAML node with the saved object
 * @param saveableObj the object to load
 * @return yamlNode
 */
const YAML::Node &operator>>(
  const YAML::Node &yamlNode, GOSaveableToYaml &saveableObj);

/**
 * Save the object into the Emitter stream only if the object is not empty
 * @param emitter to save the object to
 * @param saveableObj to save
 * @return emitter
 */
YAML::Emitter &operator<<(
  YAML::Emitter &emitter, const GOSaveableToYaml &saveableObj);

#endif /* GOSAVABLETOYAML_H */
