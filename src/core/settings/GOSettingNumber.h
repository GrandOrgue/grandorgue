/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTINGNUMBER_H
#define GOSETTINGNUMBER_H

#include "settings/GOSetting.h"

template <class T> class GOSettingNumber : private GOSetting {
private:
  T m_Value;
  T m_MinValue;
  T m_MaxValue;
  T m_DefaultValue;

  void Load(GOConfigReader &cfg);
  void Save(GOConfigWriter &cfg);

protected:
  virtual T validate(T value) {
    if (value <= m_MinValue)
      value = m_MinValue;
    if (value >= m_MaxValue)
      value = m_MaxValue;
    return value;
  }

public:
  GOSettingNumber(
    GOSettingStore *store,
    wxString group,
    wxString name,
    T min_value,
    T max_value,
    T default_value);

  void setDefaultValue(T default_value);

  T operator()() const { return m_Value; }

  void operator()(T value) { m_Value = validate(value); }
};

typedef GOSettingNumber<unsigned> GOSettingUnsigned;
typedef GOSettingNumber<int> GOSettingInteger;

#endif
