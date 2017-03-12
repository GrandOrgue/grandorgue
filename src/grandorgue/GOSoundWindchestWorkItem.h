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

#ifndef GOSOUNDWINDCHESTWORKITEM_H
#define GOSOUNDWINDCHESTWORKITEM_H

#include "GOSoundWorkItem.h"
#include "mutex.h"
#include "ptrvector.h"

class GOSoundEngine;
class GOSoundTremulantWorkItem;
class GOrgueWindchest;

class GOSoundWindchestWorkItem : public GOSoundWorkItem
{
private:
	GOSoundEngine& m_engine;
	GOMutex m_Mutex;
	float m_Volume;
	bool m_Done;
	GOrgueWindchest* m_Windchest;
	std::vector<GOSoundTremulantWorkItem*> m_Tremulants;

public:
	GOSoundWindchestWorkItem(GOSoundEngine& sound_engine, GOrgueWindchest* windchest);

	unsigned GetGroup();
	unsigned GetCost();
	bool GetRepeat();
	void Run();
	void Exec();

	void Clear();
	void Reset();
	void Init(ptr_vector<GOSoundTremulantWorkItem>& tremulants);

	float GetWindchestVolume();
	float GetVolume()
	{
		if (!m_Done)
			Run();
		return m_Volume;
	}
};

#endif
