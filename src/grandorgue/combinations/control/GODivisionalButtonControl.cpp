/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GODivisionalButtonControl.h"

#include "wx/intl.h"

#include "combinations/GOSetter.h"
#include "model/GOOrganModel.h"

GODivisionalButtonControl::GODivisionalButtonControl(
  GOOrganModel &organModel, unsigned manualNumber, unsigned divisionalIndex)
  : GOPushbuttonControl(organModel),
    r_OrganModel(organModel),
    m_combination(organModel, manualNumber, false, divisionalIndex) {}

const wxString WX_MIDI_TYPE_CODE = wxT("Divisional");
const wxString WX_MIDI_TYPE = _("Divisional");

const wxString &GODivisionalButtonControl::GetMidiTypeCode() const {
  return WX_MIDI_TYPE_CODE;
}

const wxString &GODivisionalButtonControl::GetMidiType() const {
  return WX_MIDI_TYPE;
}

void GODivisionalButtonControl::Init(
  GOConfigReader &cfg, const wxString &group, const wxString &name) {
  GOPushbuttonControl::Init(cfg, group, name);
  m_combination.Init(group);
}

void GODivisionalButtonControl::Load(
  GOConfigReader &cfg, const wxString &group) {
  GOPushbuttonControl::Load(cfg, group);
  m_combination.Load(cfg, group);
}

void GODivisionalButtonControl::LoadCombination(GOConfigReader &cfg) {
  m_combination.LoadCombination(cfg);
}

void GODivisionalButtonControl::Save(GOConfigWriter &cfg) {
  GOPushbuttonControl::Save(cfg);
  m_combination.Save(cfg);
}

void GODivisionalButtonControl::Push() {
  r_OrganModel.PushDivisional(m_combination, this);
}
