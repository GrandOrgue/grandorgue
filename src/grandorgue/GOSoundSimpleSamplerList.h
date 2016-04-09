/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2016 GrandOrgue contributors (see AUTHORS)
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

#ifndef GOSOUNDSIMPLESAMPLERLIST_H
#define GOSOUNDSIMPLESAMPLERLIST_H

#include "GOSoundSampler.h"
#include "atomic.h"

class GOSoundSimpleSamplerList
{
private:
	atomic<GO_SAMPLER*> m_List;
public:
	GOSoundSimpleSamplerList()
	{
		Clear();
	}

	void Clear()
	{
		m_List = 0;
	}

	GO_SAMPLER* Get()
	{
		do
		{
			GO_SAMPLER* sampler = m_List;
			if (!sampler)
				return NULL;
			GO_SAMPLER* next = sampler->next;
			if (m_List.compare_exchange(sampler, next))
				return sampler;
		}
		while(true);
	}

	void Put(GO_SAMPLER* sampler)
	{
		do
		{
			GO_SAMPLER* current = m_List;
			sampler->next = current;
			if (m_List.compare_exchange(current, sampler))
				return;
		}
		while(true);
	}
};

#endif
