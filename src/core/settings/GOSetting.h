/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTING_H
#define GOSETTING_H

#include <wx/string.h>

class GOConfigReader;
class GOConfigWriter;
class GOSettingStore;

class GOSetting {
private:
  GOSetting(const GOSetting &);
  void operator=(const GOSetting &);

protected:
  wxString m_Group;
  wxString m_Name;

public:
  GOSetting(GOSettingStore *store, wxString group, wxString name);
  virtual ~GOSetting();

  virtual void Load(GOConfigReader &cfg) = 0;
  virtual void Save(GOConfigWriter &cfg) = 0;
};

#endif
