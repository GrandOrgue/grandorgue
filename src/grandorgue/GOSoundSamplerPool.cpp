/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOSoundSamplerPool.h"

#include "GOSoundSampler.h"
#include "threading/GOMutexLocker.h"
#include <assert.h>
#include <string.h>

GOSoundSamplerPool::GOSoundSamplerPool() :
	m_SamplerCount(0),
	m_UsageLimit(0),
	m_AvailableSamplers(),
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

	m_AvailableSamplers.Clear();

	for (unsigned i = 0; i < m_Samplers.size(); i++)
		m_AvailableSamplers.Put(m_Samplers[i]);
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
		sampler = m_AvailableSamplers.Get();
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
	m_AvailableSamplers.Put(sampler);
}

