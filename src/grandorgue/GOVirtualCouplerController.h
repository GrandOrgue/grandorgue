/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOVIRTUALCOUPLERCONTROLLER_H
#define GOVIRTUALCOUPLERCONTROLLER_H

#include <map>
#include <utility>
#include <vector>

class GOConfigReader;
class GOCoupler;
class GOOrganModel;

class GOVirtualCouplerController {
  /**
   * Represents virtual couplers that do not exist vin ODF and are added
   * virtually by GrandOrgue. They are exposed on the GOGUICouplerPanel
   */

public:
  enum CouplerType {
    COUPLER_16,
    COUPLER_8,
    COUPLER_4,
    COUPLER_BAS,
    COUPLER_MEL
  };

  // FromManual, toManual
  using CouplerSetKey = std::pair<unsigned, unsigned>;

  // indexed by CouplerType
  using ManualCouplerSet = std::vector<GOCoupler *>;

private:
  std::map<CouplerSetKey, ManualCouplerSet> m_CouplerPtrs;

public:
  // Create virtual couplers for each manual pairs with all CouplerType and add
  // them to the organ model
  void Init(GOOrganModel &organModel, GOConfigReader &cfg);

  // Clears the couplers
  void Cleanup() { m_CouplerPtrs.clear(); }

  // Returns the coupler pointer
  GOCoupler *GetCoupler(
    unsigned fromManual, unsigned toManual, CouplerType type) const;
};

#endif /* GOVIRTUALCOUPLERCONTROLLER_H */
