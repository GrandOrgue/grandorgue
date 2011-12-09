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

#ifndef GOSOUNDPROVIDER_H_
#define GOSOUNDPROVIDER_H_

#include "GOSoundAudioSection.h"
class GOrgueMemoryPool;
class GOrgueCache;
class GOrgueCacheWriter;

class GOSoundProvider
{

private:
	void GetMaxAmplitudeAndDerivative(AUDIO_SECTION& section, int& runningMaxAmplitude, int& runningMaxDerivative);
	static bool SaveCacheAudioSection(GOrgueCacheWriter& cache, const AUDIO_SECTION* section, bool save_align_tracker);
	static bool LoadCacheAudioSection(GOrgueCache& cache, AUDIO_SECTION* section, bool load_align_tracker);

protected:
	float          m_Gain;
	unsigned int   m_SampleRate;
	AUDIO_SECTION  m_Attack;
	AUDIO_SECTION  m_Loop;
	AUDIO_SECTION  m_Release;
	GOrgueMemoryPool& m_pool;
	void ComputeReleaseAlignmentInfo();

public:
	GOSoundProvider(GOrgueMemoryPool& pool);
	virtual ~GOSoundProvider();

	void ClearData();

	virtual bool LoadCache(GOrgueCache& cache);
	virtual bool SaveCache(GOrgueCacheWriter& cache);

	const AUDIO_SECTION* GetLoop() const;
	const AUDIO_SECTION* GetRelease() const;
	const AUDIO_SECTION* GetAttack() const;
	float GetGain() const;
	int IsOneshot() const;

};

inline
const AUDIO_SECTION* GOSoundProvider::GetLoop() const
{
	return &m_Loop;
}

inline
const AUDIO_SECTION* GOSoundProvider::GetRelease() const
{
	return &m_Release;
}

inline
const AUDIO_SECTION* GOSoundProvider::GetAttack() const
{
	return &m_Attack;
}

inline
float GOSoundProvider::GetGain() const
{
	return m_Gain;
}

#endif /* GOSOUNDPROVIDER_H_ */
