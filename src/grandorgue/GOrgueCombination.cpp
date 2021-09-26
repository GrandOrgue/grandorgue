/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueCombination.h"

#include "GOrgueCombinationDefinition.h"
#include "GOrgueCombinationElement.h"
#include "GOrgueDrawStop.h"
#include "GOrgueSetter.h"
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

bool GOrgueCombination::PushLocal()
{
	bool used = false;
	const std::vector<GOrgueCombinationDefinition::CombinationSlot>& elements = m_Template.GetCombinationElements();
	UpdateState();

	if (m_OrganFile->GetSetter()->IsSetterActive())
	{
		if (m_Protected)
			return false;
		if (m_OrganFile->GetSetter()->GetSetterType() == SETTER_REGULAR)
		{
			for(unsigned i = 0; i < elements.size(); i++)
			{
				if (!m_OrganFile->GetSetter()->StoreInvisibleObjects() &&
				    !elements[i].store_unconditional)
					m_State[i] = -1;
				else if (elements[i].control->GetCombinationState())
				{
					m_State[i] = 1;
					used |= 1;
				}
				else
					m_State[i] = 0;
			}
			m_OrganFile->Modified();
		}
		if (m_OrganFile->GetSetter()->GetSetterType() == SETTER_SCOPE)
		{
			for(unsigned i = 0; i < elements.size(); i++)
			{
				if (!m_OrganFile->GetSetter()->StoreInvisibleObjects() &&
				    !elements[i].store_unconditional)
					m_State[i] = -1;
				else if (elements[i].control->GetCombinationState())
				{
					m_State[i] = 1;
					used |= 1;
				}
				else
					m_State[i] = -1;
			}
			m_OrganFile->Modified();
		}
		if (m_OrganFile->GetSetter()->GetSetterType() == SETTER_SCOPED)
		{
			for(unsigned i = 0; i < elements.size(); i++)
			{
				if (m_State[i] != -1)
				{
					if (elements[i].control->GetCombinationState())
					{
						m_State[i] = 1;
						used |= 1;
					}
					else
						m_State[i] = 0;
				}
			}
		}
	}
	else
	{
		for(unsigned i = 0; i < elements.size(); i++)
		{
			if (m_State[i] != -1)
			{
				elements[i].control->SetCombination(m_State[i] == 1);
				used |= m_State[i] == 1;
			}
		}
	}

	return used;
}
