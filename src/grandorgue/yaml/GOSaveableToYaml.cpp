/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSaveableToYaml.h"

#include <yaml-cpp/yaml.h>

YAML::Node GOSaveableToYaml::ToYamlNode() const {
  YAML::Node yamlNode(YAML::NodeType::Map);

  ToYaml(yamlNode);
  return yamlNode;
}

YAML::Node &operator<<(
  YAML::Node &yamlNode, const GOSaveableToYaml &saveableObj) {
  saveableObj.ToYaml(yamlNode);
  return yamlNode;
}

const YAML::Node &operator>>(
  const YAML::Node &yamlNode, GOSaveableToYaml &saveableObj) {
  saveableObj.FromYaml(yamlNode);
  return yamlNode;
}

YAML::Emitter &operator<<(
  YAML::Emitter &emitter, const GOSaveableToYaml &saveableObj) {
  YAML::Node yamlNode = saveableObj.ToYamlNode();

  if (!yamlNode.IsNull())
    emitter << yamlNode;
  return emitter;
}
