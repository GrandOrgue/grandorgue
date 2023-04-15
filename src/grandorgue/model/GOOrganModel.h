/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOORGANMODEL_H
#define GOORGANMODEL_H

#include "ptrvector.h"

#include "midi/GOMidiSendProxy.h"
#include "midi/dialog-creator/GOMidiDialogCreatorProxy.h"
#include "modification/GOModificationProxy.h"
#include "pipe-config/GOPipeConfigTreeNode.h"

#include "GOEventHandlerList.h"

class GOConfig;
class GOConfigReader;
class GODivisionalCoupler;
class GOEnclosure;
class GOGeneralButtonControl;
class GOManual;
class GOOrganController;
class GOPistonControl;
class GORank;
class GOSwitch;
class GOTremulant;
class GOWindchest;

class GOOrganModel : public GOEventHandlerList,
                     public GOMidiDialogCreatorProxy,
                     public GOMidiSendProxy {
private:
  GOModificationProxy m_ModificationProxy;

  const GOConfig &m_config;

  GOPipeConfigTreeNode m_RootPipeConfigNode;

  bool m_OrganModelModified;

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

  void Load(GOConfigReader &cfg, GOOrganController *organController);

public:
  GOOrganModel(const GOConfig &config);
  ~GOOrganModel();

  const GOConfig &GetConfig() const { return m_config; }

  GOPipeConfigNode &GetRootPipeConfigNode() { return m_RootPipeConfigNode; }

  bool IsOrganModelModified() const { return m_OrganModelModified; }
  void SetOrganModelModified(bool modified);
  void SetOrganModelModified() { SetOrganModelModified(true); }
  void ResetOrganModelModified() { SetOrganModelModified(false); }

  void SetModelModificationListener(GOModificationListener *listener) {
    m_ModificationProxy.SetModificationListener(listener);
  }

  void UpdateTremulant(GOTremulant *tremulant);
  void UpdateVolume();

  unsigned GetWindchestGroupCount();
  unsigned AddWindchest(GOWindchest *windchest);
  GOWindchest *GetWindchest(unsigned index);

  GOEnclosure *GetEnclosureElement(unsigned index);
  unsigned GetEnclosureCount();
  unsigned AddEnclosure(GOEnclosure *enclosure);

  unsigned GetSwitchCount() const { return m_switches.size(); }
  GOSwitch *GetSwitch(unsigned index) { return m_switches[index]; }
  /**
   * Find a switch by it's name
   * @param name - the name of switch to find
   * @return the switch index or -1 if the tremulant is not found
   */
  int FindSwitchByName(const wxString &name) const;

  unsigned GetTremulantCount() const { return m_tremulants.size(); }
  GOTremulant *GetTremulant(unsigned index) { return m_tremulants[index]; }
  /**
   * Find a tremulant by it's name
   * @param name - the name of tremulant to find
   * @return the tremulant index or -1 if the tremulant is not found
   */
  int FindTremulantByName(const wxString &name) const;

  unsigned GetManualAndPedalCount();
  unsigned GetODFManualCount();
  unsigned GetFirstManualIndex();
  GOManual *GetManual(unsigned index);

  GORank *GetRank(unsigned index);
  unsigned GetODFRankCount();
  void AddRank(GORank *rank);

  unsigned GetNumberOfReversiblePistons();
  GOPistonControl *GetPiston(unsigned index);

  unsigned GetDivisionalCouplerCount() const {
    return m_DivisionalCoupler.size();
  }
  GODivisionalCoupler *GetDivisionalCoupler(unsigned index) {
    return m_DivisionalCoupler[index];
  }

  /**
   * Find a divisional coupler by it's name
   * @param name - the name of the divisional coupler to find
   * @return the divisional coupler index or -1 if the coupler is not found
   */
  int FindDivisionalCouplerByName(const wxString &name) const;

  unsigned GetGeneralCount();
  GOGeneralButtonControl *GetGeneral(unsigned index);

  unsigned GetStopCount();
  unsigned GetCouplerCount();
  unsigned GetODFCouplerCount();
};

#endif
