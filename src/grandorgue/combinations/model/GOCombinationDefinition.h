/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCOMBINATIONDEFINITION_H
#define GOCOMBINATIONDEFINITION_H

#include <wx/string.h>

#include <vector>

class GOCombinationElement;
class GODrawstop;
class GOManual;
class GOOrganController;

class GOCombinationDefinition {
public:
  typedef enum {
    COMBINATION_STOP = 0,
    COMBINATION_COUPLER = 1,
    COMBINATION_TREMULANT = 2,
    COMBINATION_DIVISIONALCOUPLER = 3,
    COMBINATION_SWITCH = 4
  } CombinationType;
  typedef struct {
    CombinationType type;
    int manual;
    unsigned index;
    bool store_unconditional;
    wxString group;
    GOCombinationElement *control;
  } CombinationSlot;

private:
  GOOrganController *m_OrganController;
  std::vector<CombinationSlot> m_Content;

  void AddGeneral(
    GODrawstop *control, CombinationType type, int manual, unsigned index);
  void AddDivisional(
    GODrawstop *control, CombinationType type, int manual, unsigned index);
  void Add(
    GODrawstop *control,
    CombinationType type,
    int manual,
    unsigned index,
    bool store_unconditional);

public:
  GOCombinationDefinition(GOOrganController *organController);
  ~GOCombinationDefinition();

  void InitGeneral();
  void InitDivisional(unsigned manual_number);

  int findEntry(CombinationType type, int manual, unsigned index);

  const std::vector<CombinationSlot> &GetCombinationElements();
};

#endif
