/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GODivisionalCoupler.h"

#include <wx/intl.h>

#include "config/GOConfigReader.h"

#include "GOOrganController.h"

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

void GODivisionalCoupler::SetupCombinationState() {
  m_StoreDivisional = false;
  m_StoreGeneral = r_OrganModel.GeneralsStoreDivisionalCouplers();
}

void GODivisionalCoupler::ChangeState(bool on) {}

unsigned GODivisionalCoupler::GetNumberOfManuals() { return m_manuals.size(); }

unsigned GODivisionalCoupler::GetManual(unsigned index) {
  return m_manuals[index];
}

bool GODivisionalCoupler::IsBidirectional() { return m_BiDirectionalCoupling; }

wxString GODivisionalCoupler::GetMidiType() { return _("Divisional Coupler"); }
