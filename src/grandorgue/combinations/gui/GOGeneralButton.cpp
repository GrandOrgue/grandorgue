/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGeneralButton.h"

#include <wx/intl.h>

GOGeneralButton::GOGeneralButton(
  GOCombinationDefinition &general_template,
  GODefinitionFile *organfile,
  bool is_setter)
  : GOPushbutton(organfile),
    m_general(general_template, organfile, is_setter) {}

void GOGeneralButton::Load(GOConfigReader &cfg, wxString group) {
  m_general.Load(cfg, group);
  GOPushbutton::Load(cfg, group);
}

void GOGeneralButton::Push() { m_general.Push(); }

GOGeneralCombination &GOGeneralButton::GetGeneral() { return m_general; }

wxString GOGeneralButton::GetMidiType() { return _("General"); }
