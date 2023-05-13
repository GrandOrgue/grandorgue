/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSwitch.h"

#include <wx/intl.h>

#include "GOOrganModel.h"

GOSwitch::GOSwitch(GOOrganModel &organModel) : GODrawstop(organModel) {}

GOSwitch::~GOSwitch() {}

void GOSwitch::ChangeState(bool on) {}

void GOSwitch::SetupCombinationState() {
  m_StoreDivisional
    = r_OrganModel.CombinationsStoreNonDisplayedDrawstops() || IsDisplayed();
  m_StoreGeneral
    = r_OrganModel.CombinationsStoreNonDisplayedDrawstops() || IsDisplayed();
}

wxString GOSwitch::GetMidiType() { return _("Drawstop"); }
