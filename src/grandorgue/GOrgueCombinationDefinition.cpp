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
#include "GOrgueCoupler.h"
#include "GOrgueDivisionalCoupler.h"
#include "GOrgueManual.h"
#include "GOrgueStop.h"
#include "GOrgueTremulant.h"
#include "GrandOrgueFile.h"

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

	for (unsigned j = m_organfile->GetFirstManualIndex(); j <= m_organfile->GetManualAndPedalCount(); j++)
	{
		for (unsigned i = 0; i < m_organfile->GetManual(j)->GetStopCount(); i++)
		{
			CombinationSlot def;
			def.type = COMBINATION_STOP;
			def.manual = j;
			def.index = i + 1;
			def.store_unconditional = m_organfile->CombinationsStoreNonDisplayedDrawstops() || m_organfile->GetManual(j)->GetStop(i)->IsDisplayed();
			def.control = m_organfile->GetManual(j)->GetStop(i);
			def.group = m_organfile->GetManual(j)->GetName();
			m_Content.push_back(def);
		}
	}
	
	for (unsigned j = m_organfile->GetFirstManualIndex(); j <= m_organfile->GetManualAndPedalCount(); j++)
	{
		for (unsigned i = 0; i < m_organfile->GetManual(j)->GetCouplerCount(); i++)
		{
			CombinationSlot def;
			def.type = COMBINATION_COUPLER;
			def.manual = j;
			def.index = i + 1;
			def.store_unconditional = m_organfile->CombinationsStoreNonDisplayedDrawstops() || m_organfile->GetManual(j)->GetCoupler(i)->IsDisplayed();
			def.control = m_organfile->GetManual(j)->GetCoupler(i);
			def.group = m_organfile->GetManual(j)->GetName();
			m_Content.push_back(def);
		}
	}

	for (unsigned i = 0; i < m_organfile->GetTremulantCount(); i++)
	{
		CombinationSlot def;
		def.type = COMBINATION_TREMULANT;
		def.manual = -1;
		def.index = i + 1;
		def.store_unconditional = m_organfile->CombinationsStoreNonDisplayedDrawstops() || m_organfile->GetTremulant(i)->IsDisplayed();
		def.control = m_organfile->GetTremulant(i);
		m_Content.push_back(def);
	}

	for (unsigned i = 0; i < m_organfile->GetDivisionalCouplerCount(); i++)
	{
		CombinationSlot def;
		def.type = COMBINATION_DIVISIONALCOUPLER;
		def.manual = -1;
		def.index = i + 1;
		def.store_unconditional = m_organfile->GeneralsStoreDivisionalCouplers();
		def.control = m_organfile->GetDivisionalCoupler(i);
		m_Content.push_back(def);
	}
}

void GOrgueCombinationDefinition::InitDivisional(unsigned manual_number)
{
	GOrgueManual* associatedManual = m_organfile->GetManual(manual_number);
	m_Content.resize(0);

	for (unsigned i = 0; i < associatedManual->GetStopCount(); i++)
	{
		CombinationSlot def;
		def.type = COMBINATION_STOP;
		def.manual = manual_number;
		def.index = i + 1;
		def.store_unconditional = m_organfile->CombinationsStoreNonDisplayedDrawstops() || associatedManual->GetStop(i)->IsDisplayed();
		def.group = associatedManual->GetName();
		def.control = associatedManual->GetStop(i);
		m_Content.push_back(def);
	}
	for (unsigned i = 0; i < associatedManual->GetCouplerCount(); i++)
	{
		CombinationSlot def;
		def.type = COMBINATION_COUPLER;
		def.manual = manual_number;
		def.index = i + 1;
		def.store_unconditional = ((m_organfile->DivisionalsStoreIntramanualCouplers() && !associatedManual->GetCoupler(i)->IsIntermanual()) || 
					   (m_organfile->DivisionalsStoreIntermanualCouplers() && associatedManual->GetCoupler(i)->IsIntermanual())) && 
			(m_organfile->CombinationsStoreNonDisplayedDrawstops() || associatedManual->GetCoupler(i)->IsDisplayed());
		def.group = associatedManual->GetName();
		def.control = associatedManual->GetCoupler(i);
		m_Content.push_back(def);
	}
	for (unsigned i = 0; i < associatedManual->GetTremulantCount(); i++)
	{
		CombinationSlot def;
		def.type = COMBINATION_TREMULANT;
		def.manual = manual_number;
		def.index = i + 1;
		def.store_unconditional = m_organfile->DivisionalsStoreTremulants() && (m_organfile->CombinationsStoreNonDisplayedDrawstops() || associatedManual->GetTremulant(i)->IsDisplayed());
		def.group = associatedManual->GetName();
		def.control = associatedManual->GetTremulant(i);
		m_Content.push_back(def);
	}
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
