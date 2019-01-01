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

#ifndef GOSOUNDGROUPWORKITEM_H
#define GOSOUNDGROUPWORKITEM_H

#include "GOSoundBufferItem.h"
#include "GOSoundSamplerList.h"
#include "GOSoundWorkItem.h"
#include "mutex.h"

class GOSoundEngine;

class GOSoundGroupWorkItem : public GOSoundWorkItem, public GOSoundBufferItem
{
private:
	GOSoundEngine& m_engine;
	GOSoundSamplerList m_Active;
	GOSoundSamplerList m_Release;
	GOMutex m_Mutex;
	GOCondition m_Condition;
	unsigned m_ActiveCount;
	unsigned m_Done;
	volatile bool m_Stop;

	void ProcessList(GOSoundSamplerList& list, float* output_buffer);
	void ProcessReleaseList(GOSoundSamplerList& list, float* output_buffer);

public:
	GOSoundGroupWorkItem(GOSoundEngine& sound_engine, unsigned samples_per_buffer);

	unsigned GetGroup();
	unsigned GetCost();
	bool GetRepeat();
	void Run();
	void Exec();
	void Finish(bool stop);

	void Reset();
	void Clear();
	void Add(GO_SAMPLER* sampler);
};

#endif
