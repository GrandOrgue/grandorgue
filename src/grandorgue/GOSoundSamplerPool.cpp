/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
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
	m_UsageLimit(MAX_POLYPHONY)
{
	ReturnAll();
}

void GOSoundSamplerPool::ReturnAll()
{
	m_SamplerCount = 0;
	for (unsigned i = 0; i < MAX_POLYPHONY; i++)
		m_AvailableSamplers[i] = m_Samplers + i;
}

void GOSoundSamplerPool::SetUsageLimit(unsigned count)
{
	m_UsageLimit = (m_UsageLimit <= MAX_POLYPHONY) ? m_UsageLimit : MAX_POLYPHONY;
}

GO_SAMPLER* GOSoundSamplerPool::GetSampler()
{
	GO_SAMPLER* sampler = NULL;
	if (m_SamplerCount < m_UsageLimit)
	{
		sampler = m_AvailableSamplers[m_SamplerCount++];
		memset(sampler, 0, sizeof(GO_SAMPLER));
	}
	return sampler;
}

void GOSoundSamplerPool::ReturnSampler(GO_SAMPLER* sampler)
{
	assert(m_SamplerCount > 0);
	m_AvailableSamplers[--m_SamplerCount] = sampler;
}

