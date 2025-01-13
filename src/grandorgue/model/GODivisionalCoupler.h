/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GODIVISIONALCOUPLER_H
#define GODIVISIONALCOUPLER_H

#include <set>
#include <vector>

#include "GODrawstop.h"

class GOOrganModel;

class GODivisionalCoupler : public GODrawstop {
private:
  bool m_BiDirectionalCoupling;
  std::vector<unsigned> m_manuals;

  void OnDrawstopStateChanged(bool on) override {}
  void SetupIsToStoreInCmb() override;

public:
  GODivisionalCoupler(GOOrganModel &organModel);

  using GODrawstop::Load; // for avoiding a compilation warning
  void Load(GOConfigReader &cfg, const wxString &group);

  /**
   * If the coupler is engaged and start manual participates in the coupler
   * then returns other manuals in the coupler. Otherwise returns an empty set
   * @param startManual The manual that is checked for coupling
   * @return the resulting set of other manuals
   */
  std::set<unsigned> GetCoupledManuals(unsigned startManual) const;
};

#endif /* GODIVISIONALCOUPLER_H */
