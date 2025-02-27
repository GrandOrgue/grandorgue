/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOElementCreator.h"

#include "GOCallbackButtonControl.h"

void GOElementCreator::CreateButtons(GOOrganModel &organModel) {
  for (const ButtonDefinitionEntry *pEntry = GetButtonDefinitionList();
       !pEntry->name.IsEmpty() && pEntry->value >= 0;
       pEntry++) {
    GOCallbackButtonControl *pButton = new GOCallbackButtonControl(
      organModel, this, pEntry->is_pushbutton, pEntry->is_piston);
    const unsigned buttonIndex = (unsigned)pEntry->value;

    pButton->SetContext(pEntry->p_MidiContext);
    if (m_buttons.size() <= buttonIndex)
      m_buttons.resize(buttonIndex + 1);
    m_buttons[buttonIndex] = pButton;
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
