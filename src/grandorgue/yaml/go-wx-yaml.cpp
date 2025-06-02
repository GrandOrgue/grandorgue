/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "go-wx-yaml.h"

#include <wx/intl.h>
#include <wx/log.h>
#include <wx/string.h>

#include "config/GOConfigEnum.h"

namespace YAML {

Node convert<wxString>::encode(const wxString &rhs) {
  Node node;

  if (!rhs.IsEmpty())
    node = rhs.utf8_str().data();
  return node;
}

bool convert<wxString>::decode(const Node &node, wxString &rhs) {
  const bool isNull = !node.IsDefined() || node.IsNull();
  bool isValid = isNull || node.IsScalar();

  if (isValid) {
    if (isNull)
      rhs = wxEmptyString;
    else
      rhs = wxString::FromUTF8(node.as<std::string>().c_str());
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

static void put_to_map_by_path(
  YAML::Node &container,
  std::vector<wxString>::const_iterator &current,
  const std::vector<wxString>::const_iterator &end,
  const wxString &lastKey,
  const YAML::Node &node) {
  if (current != end) {
    const char *pKey = current->utf8_str().data();
    YAML::Node child = container[pKey];
    const bool isNew = !child.IsDefined();

    if (isNew)
      container[pKey] = child;
    current++;
    put_to_map_by_path(child, current, end, lastKey, node);
  } else
    put_to_map_if_not_null(container, lastKey, node);
}

void put_to_map_by_path_if_not_null(
  YAML::Node &rootNode,
  const std::vector<wxString> &path,
  const wxString &lastKey,
  const YAML::Node &node) {
  if (!node.IsNull()) {
    std::vector<wxString>::const_iterator current = path.cbegin();

    put_to_map_by_path(rootNode, current, path.cend(), lastKey, node);
  }
}

static YAML::Node get_from_map_by_path(
  const YAML::Node &container,
  std::vector<wxString>::const_iterator &current,
  const std::vector<wxString>::const_iterator &end,
  const wxString &lastKey) {
  YAML::Node res;

  if (current != end) {
    YAML::Node child = get_from_map_or_null(container, *current);

    current++;
    res = get_from_map_by_path(child, current, end, lastKey);
  } else
    res = get_from_map_or_null(container, lastKey);
  return res;
}

YAML::Node get_from_map_by_path_or_null(
  const YAML::Node &rootNode,
  const std::vector<wxString> &path,
  const wxString &lastKey) {
  std::vector<wxString>::const_iterator current = path.cbegin();

  return get_from_map_by_path(rootNode, current, path.cend(), lastKey);
}

wxString get_child_path(const wxString &parentPath, const wxString &childKey) {
  return parentPath.IsEmpty() ? childKey
                              : wxString::Format("%s.%s", parentPath, childKey);
}

wxString get_child_path(const wxString &parentPath, const unsigned childIndex) {
  return wxString::Format("%s[%u]", parentPath, childIndex);
}

template <typename ScalarType>
static bool read_scalar(
  const YAML::Node &parentNode,
  const wxString &childPath,
  const wxString &key,
  const wxString &scalarTypeName,
  bool isRequired,
  ScalarType &value,
  GOStringSet &usedPaths) {
  bool isOk = false;
  YAML::Node childNode = get_from_map_or_null(parentNode, key);

  if (childNode.IsDefined() && !childNode.IsNull()) {
    usedPaths.insert(childPath);
    if (childNode.IsScalar())
      try {
        value = childNode.as<ScalarType>();
        isOk = true;
      } catch (const YAML::TypedBadConversion<int> &e) {
        wxLogError(
          _("The value of %s (%s) is not a valid %s"),
          childPath,
          childNode.as<wxString>(),
          scalarTypeName);
      }
    else
      wxLogError(_("The value %s is not scalar"), childPath);
  } else if (isRequired)
    wxLogError(_("The required key %s is missing"), childPath);
  return isOk;
}

wxString read_string(
  const YAML::Node &parentNode,
  const wxString &parentPath,
  const wxString &key,
  bool isRequired,
  GOStringSet &usedPaths) {
  wxString resValue;
  read_scalar(
    parentNode,
    get_child_path(parentPath, key),
    key,
    wxT("string"),
    isRequired,
    resValue,
    usedPaths);
  return resValue;
}

int read_int(
  const YAML::Node &parentNode,
  const wxString &parentPath,
  const wxString &key,
  int minValue,
  int maxValue,
  bool isRequired,
  int defaultValue,
  GOStringSet &usedPaths) {
  wxString childPath = get_child_path(parentPath, key);
  int resValue = defaultValue;
  int nodeValue;

  if (read_scalar(
        parentNode,
        childPath,
        key,
        wxT("integer"),
        isRequired,
        nodeValue,
        usedPaths)) {
    if (nodeValue >= minValue && nodeValue <= maxValue)
      resValue = nodeValue;
    else
      wxLogError(
        _("The value of %s (%d) is out of range from %d to %d"),
        childPath,
        nodeValue,
        minValue,
        maxValue);
  }
  return resValue;
}

bool read_bool(
  const YAML::Node &parentNode,
  const wxString &parentPath,
  const wxString &key,
  bool isRequired,
  bool defaultValue,
  GOStringSet &usedPaths) {
  bool resValue = defaultValue;

  read_scalar(
    parentNode,
    get_child_path(parentPath, key),
    key,
    wxT("bool"),
    isRequired,
    resValue,
    usedPaths);
  return resValue;
}

int read_enum(
  const YAML::Node &parentNode,
  const wxString &parentPath,
  const wxString &key,
  const GOConfigEnum &configEnum,
  bool isRequired,
  int defaultValue,
  GOStringSet &usedPaths) {
  wxString childPath = get_child_path(parentPath, key);
  int resValue = defaultValue;
  wxString strValue;

  if (read_scalar(
        parentNode,
        childPath,
        key,
        wxT("string"),
        isRequired,
        strValue,
        usedPaths)) {
    int nodeValue = configEnum.GetValue(strValue, -1);

    if (nodeValue != -1)
      resValue = nodeValue;
    else
      wxLogError(
        _("The value of %s (%s) is not among valid enum values"),
        childPath,
        nodeValue);
  }
  return resValue;
}

void check_all_used(
  const YAML::Node &node, const wxString &path, const GOStringSet &usedPaths) {
  if (node.IsDefined()) {
    if (node.IsScalar()) {
      if (usedPaths.find(path) == usedPaths.end())
        wxLogWarning(_("Unused yaml node %s (%s)"), path, node.as<wxString>());
    } else if (node.IsMap()) {
      for (const auto &e : node)
        check_all_used(
          e.second, get_child_path(path, e.first.as<wxString>()), usedPaths);
    } else if (node.IsSequence()) {
      for (unsigned l = node.size(), i = 0; i < l; i++)
        check_all_used(node[i], get_child_path(path, i), usedPaths);
    }
  }
}
