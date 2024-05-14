/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTEMPERAMENTCENT_H
#define GOTEMPERAMENTCENT_H

#include "GOTemperament.h"

class GOTemperamentCent : public GOTemperament {
protected:
  float m_Tuning[12];

public:
  GOTemperamentCent(
    wxString name,
    float i1,
    float i2,
    float i3,
    float i4,
    float i5,
    float i6,
    float i7,
    float i8,
    float i9,
    float i10,
    float i11,
    float i12);
  GOTemperamentCent(
    wxString name,
    wxString group,
    float i1,
    float i2,
    float i3,
    float i4,
    float i5,
    float i6,
    float i7,
    float i8,
    float i9,
    float i10,
    float i11,
    float i12);
  GOTemperamentCent(
    wxString name, wxString title, wxString group, wxString groupTitle);

  virtual float GetOffset(unsigned index) const override {
    return m_Tuning[index];
  }
};

#endif
