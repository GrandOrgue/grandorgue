/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMAINWINDOWDATA_H
#define GOMAINWINDOWDATA_H

#include <wx/gdicmn.h>

#include "GOLogicalRect.h"
#include "GOSaveableObject.h"

class GODefinitionFile;

class GOMainWindowData : private GOSaveableObject {
protected:
  GODefinitionFile *p_organFile;
  GOLogicalRect m_rect;

public:
  GOMainWindowData(GODefinitionFile *pOrganFile, const wxString &group)
    : p_organFile(pOrganFile) {
    m_group = group;
  }
  virtual ~GOMainWindowData() {}

  void Load(GOConfigReader &cfg);
  const GOLogicalRect &GetWindowRect() const { return m_rect; }
  void SetWindowRect(const GOLogicalRect &rect) { m_rect = rect; }
  void Save(GOConfigWriter &cfg);
};

#endif
