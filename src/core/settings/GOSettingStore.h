/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTINGSTORE_H
#define GOSETTINGSTORE_H

#include <vector>

class GOConfigReader;
class GOConfigWriter;
class GOSetting;

class GOSettingStore {
private:
  std::vector<GOSetting *> m_SettingList;

protected:
  void Load(GOConfigReader &cfg);
  void Save(GOConfigWriter &cfg);

public:
  GOSettingStore();
  virtual ~GOSettingStore();

  void AddSetting(GOSetting *setting);
};

#endif
