/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOORGANMODEL_H
#define GOORGANMODEL_H

#include <set>

#include "ptrvector.h"

#include "combinations/control/GOCombinationButtonSet.h"
#include "combinations/control/GOCombinationControllerProxy.h"
#include "combinations/model/GOCombinationDefinition.h"
#include "midi/GOMidiSendProxy.h"
#include "midi/dialog-creator/GOMidiDialogCreatorProxy.h"
#include "modification/GOModificationProxy.h"
#include "pipe-config/GOPipeConfigListener.h"
#include "pipe-config/GOPipeConfigTreeNode.h"

#include "GOEventHandlerList.h"

class GOConfig;
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

class GOOrganModel : private GOCombinationButtonSet,
                     public GOCombinationControllerProxy,
                     public GOEventHandlerList,
                     public GOMidiDialogCreatorProxy,
                     public GOMidiSendProxy,
                     public GOPipeConfigListener {
private:
  GOConfig &m_config;

  GOModificationProxy m_ModificationProxy;
  GOCombinationDefinition m_GeneralTemplate;

  bool m_DivisionalsStoreIntermanualCouplers;
  bool m_DivisionalsStoreIntramanualCouplers;
  bool m_DivisionalsStoreTremulants;
  bool m_GeneralsStoreDivisionalCouplers;
  bool m_CombinationsStoreNonDisplayedDrawstops;

  GOPipeConfigTreeNode m_RootPipeConfigNode;

  bool m_OrganModelModified;

  /**
   * Walks across all manuals with divisional coupler engaged and returns the
   *   set of manuals where the divisional with the same number should be pushed
   * @param startManual the first manual index where a divisional is pushed
   * @param manualSet the resulting set of manuals. It always contains
   *   startManual
   */
  void FillCoupledManualsForDivisional(
    unsigned startManual, std::set<unsigned> &manualSet) const;

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

  void Load(GOConfigReader &cfg);

  /**
   * Called after Load() and InitCmbTemplates();
   * Init general and divisional templates
   * Load generals and divisionals from ODF/cmb
   */
  void LoadCmbButtons(GOConfigReader &cfg);

  /**
   * Update all generals buttons light.
   * @param buttonToLight - the button that should be lighted on. All other
   *   divisionals are lighted off
   * @param manualIndexOnlyFor - if >= 0 change lighting of this manual only
   */
  void UpdateAllButtonsLight(
    GOButtonControl *buttonToLight, int manualIndexOnlyFor) override;

public:
  GOOrganModel(GOConfig &config);
  ~GOOrganModel();

  const GOConfig &GetConfig() const { return m_config; }
  GOConfig &GetConfig() { return m_config; }

  unsigned GetRecorderElementID(const wxString &name);

  const GOCombinationDefinition &GetGeneralTemplate() const {
    return m_GeneralTemplate;
  }

  /* combinations properties */
  bool DivisionalsStoreIntermanualCouplers() const {
    return m_DivisionalsStoreIntermanualCouplers;
  }

  bool DivisionalsStoreIntramanualCouplers() const {
    return m_DivisionalsStoreIntramanualCouplers;
  }

  bool DivisionalsStoreTremulants() const {
    return m_DivisionalsStoreTremulants;
  }

  bool CombinationsStoreNonDisplayedDrawstops() const {
    return m_CombinationsStoreNonDisplayedDrawstops;
  }

  bool GeneralsStoreDivisionalCouplers() const {
    return m_GeneralsStoreDivisionalCouplers;
  }

  GOPipeConfigNode &GetRootPipeConfigNode() { return m_RootPipeConfigNode; }

  bool IsOrganModelModified() const { return m_OrganModelModified; }
  void SetOrganModelModified(bool modified);
  void NotifyPipeConfigModified() override { SetOrganModelModified(true); }
  void ResetOrganModelModified() { SetOrganModelModified(false); }

  void SetModelModificationListener(GOModificationListener *listener) {
    m_ModificationProxy.SetModificationListener(listener);
  }

  void UpdateTremulant(GOTremulant *tremulant);
  void UpdateVolume();

  unsigned GetWindchestCount() const { return m_windchests.size(); }
  // Returns the windchest number starting with 1
  unsigned AddWindchest(GOWindchest *windchest);
  // Returns the windchest by it's index starting with 0
  GOWindchest *GetWindchest(unsigned index) { return m_windchests[index]; }

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
   * Walks recursively across all manuals with divisional coupler engaged and
   *   returns a set of manuals where the divisional with the same number should
   *   be pushed
   * @param startManual the first manual index where a divisional is pushed
   * @return the resulting set of manuals. It always contains startManual
   */
  std::set<unsigned> GetCoupledManualsForDivisional(unsigned startManual) const;

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
