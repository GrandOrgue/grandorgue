/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTEMPERAMENTUSER_H
#define GOTEMPERAMENTUSER_H

#include "GOTemperamentCent.h"

class GOConfigReader;
class GOConfigWriter;

class GOTemperamentUser : public GOTemperamentCent {
public:
  GOTemperamentUser(
    wxString name, wxString title, wxString group, wxString groupTitle);
  GOTemperamentUser(GOConfigReader &cfg, wxString group);

  void Save(GOConfigWriter &cfg, wxString group);

  float GetNoteOffset(unsigned note);
  void SetNoteOffset(unsigned note, float offset);

  void SetTitle(wxString title);
  void SetGroup(wxString group);
};

#endif
