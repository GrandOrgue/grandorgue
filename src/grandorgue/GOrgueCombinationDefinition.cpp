/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2012 GrandOrgue contributors (see AUTHORS)
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

#include "GOrgueCombinationDefinition.h"

GOrgueCombinationDefinition::GOrgueCombinationDefinition(GrandOrgueFile* organfile) :
	m_organfile(organfile),
	m_Content(0)
{
}

GOrgueCombinationDefinition::~GOrgueCombinationDefinition()
{
}

void GOrgueCombinationDefinition::InitGeneral()
{
	m_Content.resize(0);
}

void GOrgueCombinationDefinition::InitDivisional(unsigned manual_number)
{
	m_Content.resize(0);
}

const std::vector<GOrgueCombinationDefinition::CombinationSlot>& GOrgueCombinationDefinition::GetCombinationElements()
{
	return m_Content;
}

int GOrgueCombinationDefinition::findEntry(CombinationType type, int manual, unsigned index)
{
	for(unsigned i = 0; i < m_Content.size(); i++)
	{
		if (m_Content[i].type == type && m_Content[i].manual == manual && m_Content[i].index == index)
			return i;
	}
	return -1;
}
