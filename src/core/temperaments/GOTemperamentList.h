/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTEMPERAMENTLIST_H
#define GOTEMPERAMENTLIST_H

#include <wx/string.h>

#include "ptrvector.h"

class GOConfigReader;
class GOConfigWriter;
class GOTemperament;
class GOTemperamentUser;

class GOTemperamentList {
private:
  ptr_vector<GOTemperament> m_Temperaments;
  ptr_vector<GOTemperamentUser> m_UserTemperaments;

public:
  GOTemperamentList();
  ~GOTemperamentList();

  void Load(GOConfigReader &cfg);
  void Save(GOConfigWriter &cfg);
  ptr_vector<GOTemperamentUser> &GetUserTemperaments();

  void InitTemperaments();

  const GOTemperament &GetTemperament(wxString Name);
  const GOTemperament &GetTemperament(unsigned index);
  unsigned GetTemperamentIndex(wxString name);
  const wxString GetTemperamentName(unsigned index);
  unsigned GetTemperamentCount();
};

#endif
