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

#include "GOSoundWindchestWorkItem.h"

#include "GOSoundEngine.h"
#include "GOSoundTremulantWorkItem.h"
#include "GOrgueWindchest.h"

GOSoundWindchestWorkItem::GOSoundWindchestWorkItem(GOSoundEngine& sound_engine, GOrgueWindchest* windchest) :
	m_engine(sound_engine),
	m_Volume(0),
	m_Done(false),
	m_Windchest(windchest),
	m_Tremulants()
{
}

void GOSoundWindchestWorkItem::Init(ptr_vector<GOSoundTremulantWorkItem>& tremulants)
{
	m_Tremulants.clear();
	if (!m_Windchest)
		return;
	for (unsigned i = 0; i < m_Windchest->GetTremulantCount(); i++)
		m_Tremulants.push_back(tremulants[m_Windchest->GetTremulantId(i)]);
}

void GOSoundWindchestWorkItem::Clear()
{
	Reset();
}

void GOSoundWindchestWorkItem::Reset()
{
	GOMutexLocker locker(m_Mutex);
	m_Done = false;
}

unsigned GOSoundWindchestWorkItem::GetCost()
{
	return (unsigned)-2;
}

float GOSoundWindchestWorkItem::GetWindchestVolume()
{
	if (m_Windchest != NULL)
		return m_Windchest->GetVolume();
	else
		return 1;
}

void GOSoundWindchestWorkItem::Run()
{
	if (m_Done)
		return;

	GOMutexLocker locker(m_Mutex);

	if (m_Done)
		return;

	float volume = m_engine.GetGain();
	if (m_Windchest != NULL)
	{
		volume *= m_Windchest->GetVolume();
		for(unsigned i = 0; i < m_Tremulants.size(); i++)
			volume *= m_Tremulants[i]->GetVolume();
	}
	m_Volume = volume;
	m_Done = true;
}
