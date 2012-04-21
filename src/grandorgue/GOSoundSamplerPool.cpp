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
 * MA 02111-1307, USA.
 */

#include "GOSoundSamplerPool.h"
#include <string.h>
#include <assert.h>

GOSoundSamplerPool::GOSoundSamplerPool() :
	m_SamplerCount(0),
	m_UsageLimit(0),
	m_AvailableSamplers(NULL),
	m_Samplers()
{
	ReturnAll();
}

void GOSoundSamplerPool::ReturnAll()
{
	GOMutexLocker locker(m_Lock);

	m_SamplerCount = 0;

	if (m_Samplers.size() > m_UsageLimit)
		m_Samplers.resize(m_UsageLimit);

	if (m_Samplers.size())
		m_AvailableSamplers.exchange(m_Samplers[0]);
	else
		m_AvailableSamplers.exchange(NULL);

	for (unsigned i = 0; i < m_Samplers.size(); i++)
		m_Samplers[i]->next = (i + 1 < m_Samplers.size() ? m_Samplers[i + 1] : NULL);
}

void GOSoundSamplerPool::SetUsageLimit(unsigned count)
{
	m_UsageLimit = count;

	GOMutexLocker locker(m_Lock);
	while(m_Samplers.size() < m_UsageLimit)
	{
		GO_SAMPLER* sampler = new GO_SAMPLER;
		m_SamplerCount.fetch_add(1);
		m_Samplers.push_back(sampler);
		ReturnSampler(sampler);
	}
}

GO_SAMPLER* GOSoundSamplerPool::GetSampler()
{
	GO_SAMPLER* sampler = NULL;

	if (m_SamplerCount < m_UsageLimit)
	{
		sampler = m_AvailableSamplers;
		while (sampler)
		{
			if (m_AvailableSamplers.compare_exchange_strong(sampler, sampler->next))
				break;
		}
		if (sampler)
			m_SamplerCount.fetch_add(1);
	}
	if (sampler)
		memset(sampler, 0, sizeof(GO_SAMPLER));
	return sampler;
}

void GOSoundSamplerPool::ReturnSampler(GO_SAMPLER* sampler)
{
	assert(m_SamplerCount > 0);
	m_SamplerCount.fetch_add(-1);
	do
	{
		sampler->next = m_AvailableSamplers;
	}
	while(!m_AvailableSamplers.compare_exchange_strong(sampler->next, sampler));
}

