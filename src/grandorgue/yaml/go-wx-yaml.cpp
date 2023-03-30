/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "go-wx-yaml.h"

#include <wx/string.h>

namespace YAML {

Node convert<wxString>::encode(const wxString &rhs) {
  Node node;

  if (!rhs.IsEmpty())
    node = rhs.mbc_str().data();
  return node;
}

bool convert<wxString>::decode(const Node &node, wxString &rhs) {
  const bool isNull = !node.IsDefined() || node.IsNull();
  bool isValid = isNull || node.IsScalar();

  if (isValid) {
    if (isNull)
      rhs = wxEmptyString;
    else
      rhs = node.as<std::string>();
  }
  return isValid;
}

} // namespace YAML

YAML::Node get_from_map_or_null(const YAML::Node &container, const char *key) {
  YAML::Node resultNode;

  if (container.IsDefined() && container.IsMap()) {
    YAML::Node valueNode = container[key];

    if (valueNode.IsDefined())
      resultNode = valueNode;
  }
  return resultNode;
}

void put_to_map_if_not_null(
  YAML::Node &container, const char *key, const YAML::Node &value) {
  if (!value.IsNull())
    container[key] = value;
}

const char *const NAME = "name";

void put_to_map_with_name(
  YAML::Node &container,
  const char *key,
  const wxString &nameValue,
  const char *valueLabel,
  const YAML::Node &value) {
  if (!value.IsNull()) {
    YAML::Node childNode = container[key];

    childNode[NAME] = nameValue;
    childNode[valueLabel] = value;
  }
}
