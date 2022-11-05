/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSwitch.h"

#include <wx/intl.h>

#include "GOOrganController.h"

GOSwitch::GOSwitch(GODefinitionFile *organfile) : GODrawstop(organfile) {}

GOSwitch::~GOSwitch() {}

void GOSwitch::ChangeState(bool on) {}

void GOSwitch::SetupCombinationState() {
  m_StoreDivisional
    = m_organfile->CombinationsStoreNonDisplayedDrawstops() || IsDisplayed();
  m_StoreGeneral
    = m_organfile->CombinationsStoreNonDisplayedDrawstops() || IsDisplayed();
}

wxString GOSwitch::GetMidiType() { return _("Drawstop"); }
