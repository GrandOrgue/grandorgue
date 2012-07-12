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

#include "GOrgueCombination.h"
#include "GOrgueCombinationDefinition.h"
#include "GrandOrgueFile.h"

GOrgueCombination::GOrgueCombination(GOrgueCombinationDefinition& combination_template, GrandOrgueFile* organfile) :
	m_OrganFile(organfile),
	m_Template(combination_template),
	m_State(0),
	m_Protected(false)
{
}

GOrgueCombination::~GOrgueCombination()
{
}

void GOrgueCombination::Clear()
{
	UpdateState();
	for(unsigned i = 0; i < m_State.size(); i++)
		m_State[i] = -1;
}

void GOrgueCombination::Copy(GOrgueCombination* combination)
{
	assert(GetTemplate() == combination->GetTemplate());
	m_State = combination->m_State;
	UpdateState();
	m_OrganFile->Modified();
}

int GOrgueCombination::GetState(unsigned no)
{
	return m_State[no];
}

void GOrgueCombination::SetState(unsigned no, int value)
{
	m_State[no] = value;
}

void GOrgueCombination::UpdateState()
{
	const std::vector<GOrgueCombinationDefinition::CombinationSlot>& elements = m_Template.GetCombinationElements();
	if (m_State.size() > elements.size())
		m_State.resize(elements.size());
	else if (m_State.size() < elements.size())
	{
		unsigned current = m_State.size();
		m_State.resize(elements.size());
		while(current < elements.size())
			m_State[current++] = -1;
	}
}

GOrgueCombinationDefinition* GOrgueCombination::GetTemplate()
{
	return &m_Template;
}
