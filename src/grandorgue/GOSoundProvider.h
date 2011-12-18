/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2011 GrandOrgue contributors (see AUTHORS)
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

#ifndef GOSOUNDPROVIDER_H_
#define GOSOUNDPROVIDER_H_

#include "GOSoundAudioSection.h"
class GOrgueMemoryPool;
class GOrgueCache;
class GOrgueCacheWriter;

class GOSoundProvider
{

private:
	void GetMaxAmplitudeAndDerivative(GOAudioSection& section, int& runningMaxAmplitude, int& runningMaxDerivative);
	static bool SaveCacheAudioSection(GOrgueCacheWriter& cache, const GOAudioSection* section, bool save_align_tracker);
	static bool LoadCacheAudioSection(GOrgueCache& cache, GOAudioSection* section, bool load_align_tracker);

protected:
	float          m_Gain;
	float          m_Tuning;
	unsigned int   m_SampleRate;
	GOAudioSection  m_Attack;
	GOAudioSection  m_Loop;
	GOAudioSection  m_Release;
	GOrgueMemoryPool& m_pool;
	void ComputeReleaseAlignmentInfo();

public:
	GOSoundProvider(GOrgueMemoryPool& pool);
	virtual ~GOSoundProvider();

	void ClearData();

	virtual bool LoadCache(GOrgueCache& cache);
	virtual bool SaveCache(GOrgueCacheWriter& cache);

	const GOAudioSection* GetLoop() const;
	const GOAudioSection* GetRelease() const;
	const GOAudioSection* GetAttack() const;
	float GetGain() const;
	int IsOneshot() const;

	float GetTuning() const;
	void SetTuning(float cent);
};

inline
const GOAudioSection* GOSoundProvider::GetLoop() const
{
	return &m_Loop;
}

inline
const GOAudioSection* GOSoundProvider::GetRelease() const
{
	return &m_Release;
}

inline
const GOAudioSection* GOSoundProvider::GetAttack() const
{
	return &m_Attack;
}

inline
float GOSoundProvider::GetGain() const
{
	return m_Gain;
}

inline
float GOSoundProvider::GetTuning() const
{
	return m_Tuning;
}

#endif /* GOSOUNDPROVIDER_H_ */
