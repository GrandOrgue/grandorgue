/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOElementCreator.h"

#include "GOCallbackButtonControl.h"

void GOElementCreator::CreateButtons(GOOrganModel &organModel) {
  const struct ButtonDefinitionEntry *entries = GetButtonDefinitionList();
  for (unsigned i = 0;
       entries[i].name != wxEmptyString && entries[i].value >= 0;
       i++) {
    if (m_buttons.size() <= (unsigned)entries[i].value)
      m_buttons.resize(entries[i].value + 1);
    m_buttons[entries[i].value] = new GOCallbackButtonControl(
      organModel, this, entries[i].is_pushbutton, entries[i].is_piston);
  }
}

GOButtonControl *GOElementCreator::GetButtonControl(
  const wxString &name, bool is_panel) {
  const struct ButtonDefinitionEntry *entries = GetButtonDefinitionList();
  for (unsigned i = 0;
       entries[i].name != wxEmptyString && entries[i].value >= 0;
       i++)
    if (name == entries[i].name) {
      if (is_panel && !entries[i].is_public)
        return NULL;
      return m_buttons[entries[i].value];
    }

  return NULL;
}

void GOElementCreator::ButtonStateChanged(
  GOButtonControl *button, bool newState) {
  for (unsigned i = 0; i < m_buttons.size(); i++)
    if (m_buttons[i] == button)
      ButtonStateChanged(i, newState);
}
