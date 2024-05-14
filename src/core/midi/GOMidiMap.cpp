/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiMap.h"

#include <assert.h>

GOMidiMap::GOMidiMap() : m_DeviceMap(), m_ElementMap() {
  m_DeviceMap.push_back(wxEmptyString);
  m_ElementMap.push_back(wxEmptyString);
  assert(m_DeviceMap[0] == wxEmptyString);
}

GOMidiMap::~GOMidiMap() {}

const wxString &GOMidiMap::GetDeviceLogicalNameById(unsigned id) {
  assert(id <= m_DeviceMap.size());
  return m_DeviceMap[id];
}

unsigned GOMidiMap::GetDeviceIdByLogicalName(const wxString &str) {
  for (unsigned i = 0; i < m_DeviceMap.size(); i++)
    if (m_DeviceMap[i] == str)
      return i;
  m_DeviceMap.push_back(str);
  return m_DeviceMap.size() - 1;
}

unsigned GOMidiMap::GetElementByString(const wxString &str) {
  for (unsigned i = 0; i < m_ElementMap.size(); i++)
    if (m_ElementMap[i] == str)
      return i;
  m_ElementMap.push_back(str);
  return m_ElementMap.size() - 1;
}

const wxString &GOMidiMap::GetElementByID(unsigned id) {
  assert(id <= m_ElementMap.size());
  return m_ElementMap[id];
}
