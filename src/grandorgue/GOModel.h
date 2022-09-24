/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMODEL_H
#define GOMODEL_H

#include "ptrvector.h"

class GOConfigReader;
class GODivisionalCoupler;
class GOEnclosure;
class GOGeneralButtonControl;
class GOManual;
class GOPistonControl;
class GORank;
class GOSwitch;
class GOTremulant;
class GOWindchest;
class GODefinitionFile;

class GOModel {
protected:
  ptr_vector<GOWindchest> m_windchests;
  ptr_vector<GOManual> m_manuals;
  ptr_vector<GOEnclosure> m_enclosures;
  ptr_vector<GOSwitch> m_switches;
  ptr_vector<GOTremulant> m_tremulants;
  ptr_vector<GORank> m_ranks;
  ptr_vector<GOPistonControl> m_pistons;
  ptr_vector<GODivisionalCoupler> m_DivisionalCoupler;
  ptr_vector<GOGeneralButtonControl> m_generals;
  unsigned m_FirstManual;
  unsigned m_ODFManualCount;
  unsigned m_ODFRankCount;

  void Load(GOConfigReader &cfg, GODefinitionFile *organfile);

public:
  GOModel();
  ~GOModel();

  void UpdateTremulant(GOTremulant *tremulant);
  void UpdateVolume();

  unsigned GetWindchestGroupCount();
  unsigned AddWindchest(GOWindchest *windchest);
  GOWindchest *GetWindchest(unsigned index);

  GOEnclosure *GetEnclosureElement(unsigned index);
  unsigned GetEnclosureCount();
  unsigned AddEnclosure(GOEnclosure *enclosure);

  unsigned GetSwitchCount();
  GOSwitch *GetSwitch(unsigned index);

  unsigned GetTremulantCount();
  GOTremulant *GetTremulant(unsigned index);

  unsigned GetManualAndPedalCount();
  unsigned GetODFManualCount();
  unsigned GetFirstManualIndex();
  GOManual *GetManual(unsigned index);

  GORank *GetRank(unsigned index);
  unsigned GetODFRankCount();
  void AddRank(GORank *rank);

  unsigned GetNumberOfReversiblePistons();
  GOPistonControl *GetPiston(unsigned index);

  unsigned GetDivisionalCouplerCount();
  GODivisionalCoupler *GetDivisionalCoupler(unsigned index);

  unsigned GetGeneralCount();
  GOGeneralButtonControl *GetGeneral(unsigned index);

  unsigned GetStopCount();
  unsigned GetCouplerCount();
  unsigned GetODFCouplerCount();
};

#endif
