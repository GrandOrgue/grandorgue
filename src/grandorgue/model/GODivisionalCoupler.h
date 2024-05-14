/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GODIVISIONALCOUPLER_H
#define GODIVISIONALCOUPLER_H

#include <set>
#include <vector>

#include "GODrawStop.h"

class GOOrganModel;

class GODivisionalCoupler : public GODrawstop {
private:
  static const wxString WX_MIDI_TYPE_CODE;
  static const wxString WX_MIDI_TYPE_DESC;

  bool m_BiDirectionalCoupling;
  std::vector<unsigned> m_manuals;

  void ChangeState(bool on) override {}
  void SetupIsToStoreInCmb() override;

public:
  GODivisionalCoupler(GOOrganModel &organModel);
  void Load(GOConfigReader &cfg, wxString group);

  /**
   * If the coupler is engaged and start manual participates in the coupler
   * then returns other manuals in the coupler. Otherwise returns an empty set
   * @param startManual The manual that is checked for coupling
   * @return the resulting set of other manuals
   */
  std::set<unsigned> GetCoupledManuals(unsigned startManual) const;

  const wxString &GetMidiTypeCode() const override { return WX_MIDI_TYPE_CODE; }
  const wxString &GetMidiType() const override { return WX_MIDI_TYPE_DESC; }
};

#endif /* GODIVISIONALCOUPLER_H */
