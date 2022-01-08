/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
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

  virtual void Save(GOConfigWriter &cfg) = 0;
  virtual void LoadCombination(GOConfigReader &cfg) {}
};

#endif
