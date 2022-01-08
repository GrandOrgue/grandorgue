/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOElementCreator.h"

#include "GOSetterButton.h"

GOElementCreator::GOElementCreator() : m_button() {}

GOElementCreator::~GOElementCreator() {}

void GOElementCreator::CreateButtons(GODefinitionFile *organfile) {
  const struct ElementListEntry *entries = GetButtonList();
  for (unsigned i = 0;
       entries[i].name != wxEmptyString && entries[i].value >= 0;
       i++) {
    if (m_button.size() <= (unsigned)entries[i].value)
      m_button.resize(entries[i].value + 1);
    m_button[entries[i].value]
      = new GOSetterButton(organfile, this, entries[i].is_pushbutton);
  }
}

GOButton *GOElementCreator::GetButton(const wxString &name, bool is_panel) {
  const struct ElementListEntry *entries = GetButtonList();
  for (unsigned i = 0;
       entries[i].name != wxEmptyString && entries[i].value >= 0;
       i++)
    if (name == entries[i].name) {
      if (is_panel && !entries[i].is_public)
        return NULL;
      return m_button[entries[i].value];
    }

  return NULL;
}

void GOElementCreator::SetterButtonChanged(GOSetterButton *button) {
  for (unsigned i = 0; i < m_button.size(); i++)
    if (m_button[i] == button)
      ButtonChanged(i);
}
