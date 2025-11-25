/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIMAP_H
#define GOMIDIMAP_H

#include <wx/string.h>

#include "GONameMap.h"

class GOMidiMap {
private:
  GONameMap m_DeviceMap;
  GONameMap m_ElementMap;

  static unsigned getIdByName(GONameMap &map, const wxString &name) {
    return map.EnsureNameExists(name.utf8_str().data());
  }

  template <typename AddingFun>
  static unsigned ensureName(
    GONameMap &map, const wxString &name, AddingFun addingFun) {
    return map.EnsureNameExists(name.utf8_str().data(), addingFun);
  }

  static wxString getNameById(const GONameMap &map, unsigned id) {
    return wxString::FromUTF8(
      map.GetNameById(static_cast<GONameMap::IdType>(id)).c_str());
  }

public:
  unsigned GetDeviceIdByLogicalName(const wxString &name) {
    return getIdByName(m_DeviceMap, name);
  }

  template <typename AddingFun>
  unsigned EnsureLogicalName(const wxString &name, AddingFun addingFun) {
    return ensureName(m_DeviceMap, name, addingFun);
  }

  wxString GetDeviceLogicalNameById(unsigned id) const {
    return getNameById(m_DeviceMap, id);
  }

  unsigned GetElementByString(const wxString &str) {
    return getIdByName(m_ElementMap, str);
  }

  wxString GetElementByID(unsigned id) const {
    return getNameById(m_ElementMap, id);
  }
};

#endif
