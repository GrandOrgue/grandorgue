/*
 * GrandOrgue - free pipe organ simulator
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
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
	wxCriticalSectionLocker locker(m_Lock);

	m_SamplerCount = 0;

	if (m_Samplers.size() > m_UsageLimit)
		m_Samplers.resize(m_UsageLimit);

	if (m_Samplers.size())
		m_AvailableSamplers = m_Samplers[0];
	else
		m_AvailableSamplers = NULL;

	for (unsigned i = 0; i < m_Samplers.size(); i++)
		m_Samplers[i]->next = (i + 1 < m_Samplers.size() ? m_Samplers[i + 1] : NULL);
}

void GOSoundSamplerPool::SetUsageLimit(unsigned count)
{
	m_UsageLimit = count;

	wxCriticalSectionLocker locker(m_Lock);
	while(m_Samplers.size() < m_UsageLimit)
	{
		GO_SAMPLER* sampler = new GO_SAMPLER;
		sampler->next = m_AvailableSamplers;
		m_AvailableSamplers = sampler;
		m_Samplers.push_back(sampler);
	}
}

GO_SAMPLER* GOSoundSamplerPool::GetSampler()
{
	wxCriticalSectionLocker locker(m_Lock);

	GO_SAMPLER* sampler = NULL;
	if (m_SamplerCount < m_UsageLimit && m_AvailableSamplers)
	{
		sampler = m_AvailableSamplers;
		m_AvailableSamplers = m_AvailableSamplers->next;
		m_SamplerCount++;
		memset(sampler, 0, sizeof(GO_SAMPLER));
	}
	return sampler;
}

void GOSoundSamplerPool::ReturnSampler(GO_SAMPLER* sampler)
{
	wxCriticalSectionLocker locker(m_Lock);

	assert(m_SamplerCount > 0);
	m_SamplerCount--;
	sampler->next = m_AvailableSamplers;
	m_AvailableSamplers = sampler;
}

