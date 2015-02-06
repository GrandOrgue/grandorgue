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

#include "GOSoundScheduler.h"

#include "GOSoundWorkItem.h"

GOSoundScheduler::GOSoundScheduler() :
	m_Work1(),
	m_Work2(),
	m_Work3(),
	m_WorkItems(),
	m_ItemCount(0),
	m_RepeatCount(0)
{
}

GOSoundScheduler::~GOSoundScheduler()
{
	GOMutexLocker lock(m_Mutex);
	Lock();
}

void GOSoundScheduler::SetRepeatCount(unsigned count)
{
	m_RepeatCount = count;
	GOMutexLocker lock(m_Mutex);
	Lock();
	Update();
	Unlock();
}

void GOSoundScheduler::Clear()
{
	GOMutexLocker lock(m_Mutex);
	Lock();
	m_Work1.clear();
	m_Work2.clear();
	m_Work3.clear();
	Update();
	Unlock();
}

void GOSoundScheduler::Lock()
{
	m_ItemCount = 0;
	__sync_synchronize();
}

void GOSoundScheduler::Unlock()
{
	m_ItemCount = m_WorkItems.size();
	__sync_synchronize();
}

void GOSoundScheduler::Update()
{
	m_WorkItems.clear();
	for(unsigned i = 0; i < m_Work1.size(); i++)
		m_WorkItems.push_back(&m_Work1[i]);
	for(unsigned j = 0; j < m_RepeatCount; j++)
		for(unsigned i = 0; i < m_Work2.size(); i++)
			m_WorkItems.push_back(&m_Work2[i]);
	for(unsigned i = 0; i < m_Work3.size(); i++)
		m_WorkItems.push_back(&m_Work3[i]);
}

void GOSoundScheduler::AddList(GOSoundWorkItem* item, std::vector<GOSoundWorkItem*>& list)
{
	list.push_back(item);
}

void GOSoundScheduler::Add(GOSoundWorkItem* item)
{
	if (!item)
		return;
	item->Clear();
	GOMutexLocker lock(m_Mutex);
	Lock();
	if (item->GetGroup() < GOSoundWorkItem::AUDIOGROUP)
		AddList(item, m_Work1);
	else if (item->GetGroup() == GOSoundWorkItem::AUDIOGROUP)
		AddList(item, m_Work2);
	else
		AddList(item, m_Work3);
	Update();
	Unlock();
}

void GOSoundScheduler::RemoveList(GOSoundWorkItem* item, std::vector<GOSoundWorkItem*>& list)
{
	for(unsigned i = 0; i < list.size(); i++)
		if (list[i] == item)
			list[i] = NULL;
}

void GOSoundScheduler::Remove(GOSoundWorkItem* item)
{
	GOMutexLocker lock(m_Mutex);
	RemoveList(item, m_Work1);
	RemoveList(item, m_Work2);
	RemoveList(item, m_Work3);
}

bool GOSoundScheduler::CompareItem(GOSoundWorkItem* a, GOSoundWorkItem* b)
{
	if (a && b)
	{
 		if (a->GetGroup() > b->GetGroup())
			return true;
 		if (a->GetCost() < b->GetCost())
			return true;
		return false;
	}
	if (!a && b)
		return true;
	return false;
}

void GOSoundScheduler::ResetList(std::vector<GOSoundWorkItem*>& list)
{
	for(unsigned i = 0; i + 1 < list.size(); i++)
	{
		for(unsigned j = i; j > 0 && CompareItem(list[j], list[j + 1]); j--)
		{
			GOSoundWorkItem* tmp = list[j];
			list[j + 1] = list[j];
			list[j] = tmp;
		}
	}
	for(unsigned i = 0; i < list.size(); i++)
		if (list[i])
			list[i]->Reset();
}

void GOSoundScheduler::Reset()
{
	GOMutexLocker lock(m_Mutex);
	ResetList(m_Work1);
	ResetList(m_Work2);
	ResetList(m_Work3);
	m_NextItem.exchange(0);
}

void GOSoundScheduler::FinishList(std::vector<GOSoundWorkItem*>& list)
{
	for(unsigned i = 0; i < list.size(); i++)
		if (list[i])
			list[i]->Run();
}

void GOSoundScheduler::Finish()
{
	GOMutexLocker lock(m_Mutex);
	FinishList(m_Work1);
	FinishList(m_Work2);
	FinishList(m_Work3);
}

GOSoundWorkItem* GOSoundScheduler::GetNextGroup()
{
	do
	{
		unsigned next = m_NextItem.fetch_add(1);
		if (next >= m_ItemCount)
			return NULL;
		GOSoundWorkItem* item = *m_WorkItems[next];
		if (item)
			return item;
	} while(true);
}

