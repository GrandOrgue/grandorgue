/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GONAMEMAP_H
#define GONAMEMAP_H

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * This class represents an id-name dictionary with search capability by id and
 * by name.
 */
class GONameMap {
public:
  using IdType = uint16_t;

  // a special id that is not in the dictionary
  static constexpr IdType ID_NOT_IN_FILE = 0;

private:
  // All names in the dictionary. The index is id - 1
  std::vector<std::unique_ptr<std::string>> m_names;

  // A map from names to ids
  std::unordered_map<std::string, IdType> m_IdsByName;

  // Adds a new name to the map. Returns the id of the new name
  IdType AddName(const std::string &name);

public:
  // search the name in the map and returns it's id or ID_NOT_FOUND if the name
  // is not in the map
  IdType GetIdByName(const std::string &name) const;

  // returns the id of the name or an empty string if the name does not exist
  const std::string &GetNameById(IdType id) const;

  /* Ensure that the name exists in the map.
   * If the name is already here, return it's id
   * If the name is not yet here, add in to the map with the new id, call
   * addingFun and return the new id
   */
  template <typename AddingFun>
  inline IdType EnsureNameExists(const std::string &name, AddingFun addingFun) {
    IdType id = GetIdByName(name);

    if (id == ID_NOT_IN_FILE) {
      id = AddName(name);
      addingFun(id);
    }
    return id;
  }

  inline IdType EnsureNameExists(const std::string &name) {
    return EnsureNameExists(name, [](IdType id) {});
  }
};

#endif /* GONAMEMAP_H */
