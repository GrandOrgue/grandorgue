/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGeneralButtonControl.h"

#include <wx/intl.h>

#include "combinations/model/GOCombinationDefinition.h"
#include "model/GOOrganModel.h"

GOGeneralButtonControl::GOGeneralButtonControl(
  GOOrganModel &organModel, bool is_setter)
  : GOPushbuttonControl(organModel, OBJECT_TYPE_GENERAL),
    r_OrganModel(organModel),
    m_combination(r_OrganModel, is_setter) {}

void GOGeneralButtonControl::Load(GOConfigReader &cfg, const wxString &group) {
  m_combination.Load(cfg, group);
  GOPushbuttonControl::Load(cfg, group);
}

void GOGeneralButtonControl::Push() {
  r_OrganModel.PushGeneral(m_combination, this);
}

GOGeneralCombination &GOGeneralButtonControl::GetCombination() {
  return m_combination;
}
