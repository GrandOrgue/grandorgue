/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GODivisionalCoupler.h"

#include <algorithm>

#include <wx/intl.h>

#include "config/GOConfigReader.h"

#include "GOOrganModel.h"

const wxString GODivisionalCoupler::WX_MIDI_TYPE_CODE
  = wxT("DivisionalCoupler");
const wxString GODivisionalCoupler::WX_MIDI_TYPE_DESC = _("Divisional Coupler");

GODivisionalCoupler::GODivisionalCoupler(GOOrganModel &organModel)
  : GODrawstop(organModel), m_BiDirectionalCoupling(false), m_manuals(0) {}

void GODivisionalCoupler::Load(GOConfigReader &cfg, wxString group) {
  wxString buffer;

  m_BiDirectionalCoupling
    = cfg.ReadBoolean(ODFSetting, group, wxT("BiDirectionalCoupling"));
  unsigned NumberOfManuals = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("NumberOfManuals"),
    1,
    r_OrganModel.GetManualAndPedalCount() - r_OrganModel.GetFirstManualIndex()
      + 1);

  m_manuals.resize(0);
  for (unsigned i = 0; i < NumberOfManuals; i++) {
    buffer.Printf(wxT("Manual%03d"), i + 1);
    m_manuals.push_back(cfg.ReadInteger(
      ODFSetting,
      group,
      buffer,
      r_OrganModel.GetFirstManualIndex(),
      r_OrganModel.GetManualAndPedalCount()));
  }
  GODrawstop::Load(cfg, group);
}

void GODivisionalCoupler::SetupIsToStoreInCmb() {
  m_IsToStoreInDivisional = false;
  m_IsToStoreInGeneral = r_OrganModel.GeneralsStoreDivisionalCouplers();
}

std::set<unsigned> GODivisionalCoupler::GetCoupledManuals(
  unsigned startManual) const {
  std::set<unsigned> res;

  if (IsEngaged()) {
    auto firstIt = m_manuals.cbegin();
    auto lastIt = m_manuals.cend();
    auto startIt = std::find(firstIt, lastIt, startManual);

    if (startIt != lastIt) { // startManual participates in the coupler
      auto copyIt = m_BiDirectionalCoupling ? firstIt : startIt;

      std::copy_if(
        copyIt, lastIt, std::inserter(res, res.begin()), [=](auto x) {
          return x != startManual;
        });
    }
  }
  return res;
}
