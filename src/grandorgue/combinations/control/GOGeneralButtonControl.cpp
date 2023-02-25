/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGeneralButtonControl.h"

#include <wx/intl.h>

#include "combinations/GOSetter.h"

#include "GOOrganController.h"

GOGeneralButtonControl::GOGeneralButtonControl(
  GOCombinationDefinition &general_template,
  GOOrganController *organController,
  bool is_setter)
  : GOPushbuttonControl(organController),
    m_general(general_template, organController, is_setter) {}

void GOGeneralButtonControl::Load(GOConfigReader &cfg, wxString group) {
  m_general.Load(cfg, group);
  GOPushbuttonControl::Load(cfg, group);
}

void GOGeneralButtonControl::Push() {
  GOCombination::ExtraElementsSet elementSet;

  m_general.Push(
    m_OrganController->GetSetter()->GetCrescendoAddSet(elementSet));
}

GOGeneralCombination &GOGeneralButtonControl::GetGeneral() { return m_general; }

wxString GOGeneralButtonControl::GetMidiType() { return _("General"); }
