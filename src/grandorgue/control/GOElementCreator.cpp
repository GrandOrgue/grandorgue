/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOElementCreator.h"

#include "GOCallbackButtonControl.h"

void GOElementCreator::CreateButtons(
  GOOrganModel &organModel, const ButtonDefinitionEntry *pEntries) {
  p_ButtonDefinitions = pEntries;
  for (const ButtonDefinitionEntry *pEntry = pEntries;
       !pEntry->name.IsEmpty() && pEntry->value >= 0;
       pEntry++) {
    GOCallbackButtonControl *pButton = new GOCallbackButtonControl(
      organModel, this, pEntry->is_pushbutton, pEntry->is_piston);
    const unsigned buttonIndex = (unsigned)pEntry->value;

    pButton->SetHardName(pEntry->name);
    pButton->SetContext(pEntry->p_MidiContext);
    if (m_buttons.size() <= buttonIndex)
      m_buttons.resize(buttonIndex + 1);
    m_buttons[buttonIndex] = pButton;
  }
}

GOButtonControl *GOElementCreator::GetButtonControl(
  const wxString &name, bool is_panel) {
  for (const ButtonDefinitionEntry *pEntry = p_ButtonDefinitions;
       pEntry && !pEntry->name.IsEmpty() && pEntry->value >= 0;
       pEntry++) {
    if (pEntry->name == name) {
      if (is_panel && !pEntry->is_public)
        return NULL;
      return m_buttons[pEntry->value];
    }
  }
  return NULL;
}

void GOElementCreator::ButtonStateChanged(
  GOButtonControl *button, bool newState) {
  for (unsigned i = 0; i < m_buttons.size(); i++)
    if (m_buttons[i] == button)
      ButtonStateChanged(i, newState);
}
