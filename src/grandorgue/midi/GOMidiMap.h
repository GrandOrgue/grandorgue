/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIMAP_H
#define GOMIDIMAP_H

#include <wx/string.h>

#include <vector>

class GOMidiMap {
private:
  std::vector<wxString> m_DeviceMap;
  std::vector<wxString> m_ElementMap;

public:
  GOMidiMap();
  ~GOMidiMap();

  unsigned GetDeviceIdByLogicalName(const wxString &str);
  const wxString &GetDeviceLogicalNameById(unsigned id);

  unsigned GetElementByString(const wxString &str);
  const wxString &GetElementByID(unsigned id);
};

#endif
