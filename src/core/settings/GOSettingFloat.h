/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTINGFLOAT_H
#define GOSETTINGFLOAT_H

#include "settings/GOSetting.h"

class GOSettingFloat : private GOSetting {
private:
  float m_Value;
  float m_MinValue;
  float m_MaxValue;
  float m_DefaultValue;

  void Load(GOConfigReader &cfg);
  void Save(GOConfigWriter &cfg);

protected:
  virtual float validate(float value);

public:
  GOSettingFloat(
    GOSettingStore *store,
    wxString group,
    wxString name,
    float min_value,
    float max_value,
    float default_value);

  void setDefaultValue(float default_value);

  float operator()() const;
  void operator()(float value);
};

#endif
