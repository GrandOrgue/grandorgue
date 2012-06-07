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
 */

#ifndef GOSOUNDSAMPLERPOOL_H_
#define GOSOUNDSAMPLERPOOL_H_

#include <wx/wx.h>

#include "GOSoundSampler.h"
#include "GOLock.h"
#include "ptrvector.h"

class GOSoundSamplerPool
{

private:
	GOMutex                 m_Lock;
	std::atomic_uint        m_SamplerCount;
	unsigned                m_UsageLimit;
	std::atomic<GO_SAMPLER*> m_AvailableSamplers;
	ptr_vector<GO_SAMPLER>  m_Samplers;

public:
	GOSoundSamplerPool();
	GO_SAMPLER* GetSampler();
	void ReturnSampler(GO_SAMPLER* sampler);
	void ReturnAll();
	unsigned GetUsageLimit() const;
	void SetUsageLimit(unsigned count);
	unsigned UsedSamplerCount() const;
};

inline
unsigned GOSoundSamplerPool::GetUsageLimit() const
{
	return m_UsageLimit;
}

inline
unsigned GOSoundSamplerPool::UsedSamplerCount() const
{
	return m_SamplerCount;
}

#endif /* GOSOUNDSAMPLERPOOL_H_ */
