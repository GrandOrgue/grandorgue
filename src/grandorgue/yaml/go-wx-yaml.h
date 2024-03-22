/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOWXYAML_H
#define GOWXYAML_H

/**
 * Converter between YAML::Node and wxString
 */

#include <wx/string.h>
#include <yaml-cpp/yaml.h>

namespace YAML {

template <> struct convert<wxString> {
  static Node encode(const wxString &rhs);
  static bool decode(const Node &node, wxString &rhs);
};

} // namespace YAML

// utility functions for special processing empty YAML nodes and maps
/**
 * if the container is a map and contains the key, return it. Otherwise return
 * a null mode
 * @param container
 * @param key
 * @return
 */
extern YAML::Node get_from_map_or_null(
  const YAML::Node &container, const char *key);
inline YAML::Node get_from_map_or_null(
  const YAML::Node &container, const wxString &key) {
  return get_from_map_or_null(container, key.utf8_str().data());
}

/**
 * Put the key-value pair into the container map only if the value is not
 *   empty:
 * key: value
 *
 * @param container - the YAML node to put the pair into
 * @param key the key to put
 * @param value the value to put
 */
extern void put_to_map_if_not_null(
  YAML::Node &container, const char *key, const YAML::Node &value);
inline void putToMapIfNotNull(
  YAML::Node &container, const wxString &key, const YAML::Node &value) {
  put_to_map_if_not_null(container, key.utf8_str().data(), value);
}

/**
 * Put the complex structure to the map if the value is not empty
 * key:
 *   name: nameValue
 *   valueLabel: value
 *
 * @param container
 * @param key
 * @param nameValue
 * @param valueLabel
 * @param value
 */
extern void put_to_map_with_name(
  YAML::Node &container,
  const char *key,
  const wxString &nameValue,
  const char *valueLabel,
  const YAML::Node &value);
inline void put_to_map_with_name(
  YAML::Node &container,
  const wxString &key,
  const wxString &nameValue,
  const char *valueLabel,
  const YAML::Node &value) {
  put_to_map_with_name(
    container, key.utf8_str().data(), nameValue, valueLabel, value);
}
#endif /* GOWXYAML_H */
