/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GONameMap.h"

static const std::string EMPTY_STRING;

GONameMap::IdType GONameMap::AddName(const std::string &name) {
  // Create a new string owned by unique_ptr and push into vector
  m_names.emplace_back(std::make_unique<std::string>(name));
  IdType newId = static_cast<IdType>(m_names.size()); // id = index + 1

  // Use string_view referencing the newly added string as key in the map
  // Note: string_view lifetime is tied to unique_ptr managed string in vector
  m_IdsByName.emplace(name, newId);
  return newId;
}

GONameMap::IdType GONameMap::GetIdByName(const std::string &name) const {
  // Lookup by string_view
  auto it = m_IdsByName.find(name);

  return it != m_IdsByName.end() ? it->second : ID_NOT_IN_FILE;
}

const std::string &GONameMap::GetNameById(GONameMap::IdType id) const {
  return (id && id <= m_names.size()) ? *m_names[id - 1] : EMPTY_STRING;
}
