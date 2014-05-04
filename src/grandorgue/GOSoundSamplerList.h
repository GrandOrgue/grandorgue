/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2014 GrandOrgue contributors (see AUTHORS)
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

#ifndef GOSOUNDSAMPLERLIST_H
#define GOSOUNDSAMPLERLIST_H

#include "GOSoundSampler.h"
#include "GOLock.h"

class GOSoundSamplerList
{
private:
	std::atomic<GO_SAMPLER*> m_GetList;
	std::atomic<GO_SAMPLER*> m_PutList;
	std::atomic_uint m_PutCount;

public:
	GOSoundSamplerList()
	{
		Clear();
	}

	void Clear()
	{
		m_GetList = 0;
		m_PutList = 0;
		m_PutCount = 0;
	}

	GO_SAMPLER* Peek()
	{
		return m_GetList;
	}

	GO_SAMPLER* Get()
	{
		do
		{
			GO_SAMPLER* sampler = m_GetList;
			if (!sampler)
				return NULL;
			GO_SAMPLER* next = sampler->next;
			if (m_GetList.compare_exchange_strong(sampler, next))
				return sampler;
		}
		while(true);
	}

	void Put(GO_SAMPLER* sampler)
	{
		do
		{
			GO_SAMPLER* current = m_PutList;
			sampler->next = current;
			if (m_PutList.compare_exchange_strong(current, sampler))
			{
				m_PutCount.fetch_add(1);
				return;
			}
		}
		while(true);
	}

	unsigned GetCount()
	{
		return m_PutCount;
	}
	
	void Move()
	{
		GO_SAMPLER* sampler;
		do
		{
			sampler = m_PutList;
			if (m_PutList.compare_exchange_strong(sampler, NULL))
				break;
		}
		while(true);
		m_PutCount.exchange(0);

		if (!sampler)
			return;
		do
		{
			GO_SAMPLER* current = m_GetList;
			GO_SAMPLER* next = sampler;
			if (current)
			{
				while(next)
				{
					if (next->next)
						next = next->next;
					else
					{
						next->next = current;
						break;
					}
				}
			}
			if (m_GetList.compare_exchange_strong(current, sampler))
				return;
			if (next)
				next->next = NULL;
		}
		while(true);
	}
};

#endif
