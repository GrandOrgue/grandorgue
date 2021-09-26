/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueElementCreator.h"

#include "GOrgueSetterButton.h"

GOrgueElementCreator::GOrgueElementCreator() :
	m_button()
{
}

GOrgueElementCreator::~GOrgueElementCreator()
{
}

void GOrgueElementCreator::CreateButtons(GrandOrgueFile* organfile)
{
	const struct ElementListEntry* entries = GetButtonList();
	for(unsigned i = 0; entries[i].name != wxEmptyString && entries[i].value >= 0; i++)
	{
		if (m_button.size() <= (unsigned)entries[i].value)
			m_button.resize(entries[i].value + 1);
		m_button[entries[i].value] = new GOrgueSetterButton(organfile, this, entries[i].is_pushbutton);
	}
}

GOrgueButton* GOrgueElementCreator::GetButton(const wxString& name, bool is_panel)
{
	const struct ElementListEntry* entries = GetButtonList();
	for(unsigned i = 0; entries[i].name != wxEmptyString && entries[i].value >= 0; i++)
		if (name == entries[i].name)
		{
			if (is_panel && !entries[i].is_public)
				return NULL;
			return m_button[entries[i].value];
		}

	return NULL;
}

void GOrgueElementCreator::SetterButtonChanged(GOrgueSetterButton* button)
{
	for(unsigned i = 0; i < m_button.size(); i++)
		if (m_button[i] == button)
			ButtonChanged(i);
}
