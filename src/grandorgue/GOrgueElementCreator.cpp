/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2015 GrandOrgue contributors (see AUTHORS)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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

GOrgueButton* GOrgueElementCreator::GetButton(const wxString& name, bool is_panel)
{
	const struct ElementListEntry* entries = GetButtonList();
	for(unsigned i = 0; entries[i].name != wxEmptyString; i++)
		if (name == entries[i].name)
		{
			if (is_panel && !entries[i].is_public)
				return NULL;
			return m_button[entries[i].value];
		}

	return NULL;
}

