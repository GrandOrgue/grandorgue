/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
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

#ifndef GORGUESOUNDSCHEDULER_H
#define GORGUESOUNDSCHEDULER_H

#include "atomic.h"
#include "mutex.h"
#include <vector>

class GOSoundWorkItem;

class GOSoundScheduler
{
private:
	std::vector<GOSoundWorkItem*> m_Work;
	std::vector<GOSoundWorkItem**> m_WorkItems;
	atomic_uint m_NextItem;
	unsigned m_ItemCount;
	unsigned m_RepeatCount;
	GOMutex m_Mutex;

	void Lock();
	void Unlock();
	void Update();

	bool CompareItem(GOSoundWorkItem* a, GOSoundWorkItem* b);
	void SortList(std::vector<GOSoundWorkItem*>& list);
	void ResetList(std::vector<GOSoundWorkItem*>& list);
	void AddList(GOSoundWorkItem* item, std::vector<GOSoundWorkItem*>& list);
	void RemoveList(GOSoundWorkItem* item, std::vector<GOSoundWorkItem*>& list);
	void ExecList(std::vector<GOSoundWorkItem*>& list);

public:
	GOSoundScheduler();
	~GOSoundScheduler();

	void SetRepeatCount(unsigned count);

	void Clear();
	void Reset();
	void Exec();
	void Add(GOSoundWorkItem* item);
	void Remove(GOSoundWorkItem* item);

	GOSoundWorkItem* GetNextGroup();
};

#endif
