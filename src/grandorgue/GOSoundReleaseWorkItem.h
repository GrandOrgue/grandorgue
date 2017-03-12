/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2017 GrandOrgue contributors (see AUTHORS)
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

#ifndef GOSOUNDRELEASEWORKITEM_H
#define GOSOUNDRELEASEWORKITEM_H

#include "GOSoundSimpleSamplerList.h"
#include "GOSoundWorkItem.h"
#include "ptrvector.h"

class GOSoundEngine;
class GOSoundGroupWorkItem;
class GO_SAMPLER;

class GOSoundReleaseWorkItem : public GOSoundWorkItem
{
private:
	GOSoundEngine& m_engine;
	ptr_vector<GOSoundGroupWorkItem>& m_AudioGroups;
	GOSoundSimpleSamplerList m_List;
	atomic_uint m_WaitCnt;
	atomic_uint m_Cnt;
	volatile bool m_Stop;

public:
	GOSoundReleaseWorkItem(GOSoundEngine& sound_engine, ptr_vector<GOSoundGroupWorkItem>& audio_groups);

	unsigned GetGroup();
	unsigned GetCost();
	bool GetRepeat();
	void Run();
	void Exec();

	void Clear();
	void Reset();

	void Add(GO_SAMPLER* sampler);
};

#endif
