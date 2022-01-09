/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGeneral.h"

#include <wx/intl.h>

GOGeneral::GOGeneral(
  GOCombinationDefinition &general_template,
  GODefinitionFile *organfile,
  bool is_setter)
  : GOPushbutton(organfile),
    m_general(general_template, organfile, is_setter) {}

void GOGeneral::Load(GOConfigReader &cfg, wxString group) {
  m_general.Load(cfg, group);
  GOPushbutton::Load(cfg, group);
}

void GOGeneral::Push() { m_general.Push(); }

GOFrameGeneral &GOGeneral::GetGeneral() { return m_general; }

wxString GOGeneral::GetMidiType() { return _("General"); }
