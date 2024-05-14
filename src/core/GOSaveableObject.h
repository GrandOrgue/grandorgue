/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSAVEABLE_H
#define GOSAVEABLE_H

#include <wx/string.h>

class GOConfigReader;
class GOConfigWriter;

class GOSaveableObject {
protected:
  wxString m_group;

public:
  GOSaveableObject() : m_group() {}

  virtual ~GOSaveableObject() {}

  const wxString &GetGroup() const { return m_group; }
  void SetGroup(const wxString &group) { m_group = group; }

  virtual void Save(GOConfigWriter &cfg) = 0;
  virtual void LoadCombination(GOConfigReader &cfg) {}
};

#endif
