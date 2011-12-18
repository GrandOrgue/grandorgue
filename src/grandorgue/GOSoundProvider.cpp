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

#include "GOSoundAudioSectionAccessor.h"
#include "GOSoundProvider.h"
#include "GOrgueReleaseAlignTable.h"
#include "GOrgueCache.h"
#include "GOrgueCacheWriter.h"
#include "GOrgueMemoryPool.h"
#include <wx/wx.h>

#define DELETE_AND_NULL(x) do { if (x) { delete x; x = NULL; } } while (0)

GOSoundProvider::GOSoundProvider(GOrgueMemoryPool& pool) :
	m_Tuning(1),
	m_pool(pool)
{
	memset(&m_Attack, 0, sizeof(m_Attack));
	memset(&m_Loop, 0, sizeof(m_Loop));
	memset(&m_Release, 0, sizeof(m_Release));
	m_Gain = 0.0f;
	m_SampleRate = 0;
}

GOSoundProvider::~GOSoundProvider()
{
	ClearData();
}

void GOSoundProvider::ClearData()
{
	m_pool.Free(m_Attack.data);
	m_pool.Free(m_Loop.data);
	m_pool.Free(m_Release.data);
	DELETE_AND_NULL(m_Release.release_aligner);
	memset(&m_Attack, 0, sizeof(m_Attack));
	memset(&m_Loop, 0, sizeof(m_Loop));
	memset(&m_Release, 0, sizeof(m_Release));
}

bool GOSoundProvider::LoadCacheAudioSection
	(GOrgueCache& cache, GOAudioSection* section, bool load_align_tracker)
{

	bool result = cache.Read(section, sizeof(GOAudioSection));
	section->release_aligner = NULL;
	section->data = NULL;
	if (!result)
		return false;

	section->data = (unsigned char*)cache.ReadBlock(section->alloc_size);
	if (section->data == NULL)
		return false;

	if (load_align_tracker)
	{

		bool has_align_tracker;
		if (!cache.Read(&has_align_tracker, sizeof(has_align_tracker)))
			return false;


		if (has_align_tracker)
		{
			section->release_aligner = new GOrgueReleaseAlignTable();
			if (!section->release_aligner->Load(cache))
				return false;
		}

	}

	return true;

}

bool GOSoundProvider::LoadCache(GOrgueCache& cache)
{

	if (!LoadCacheAudioSection(cache, &m_Attack, false))
		return false;

	if (!LoadCacheAudioSection(cache, &m_Loop, false))
		return false;

	if (!LoadCacheAudioSection(cache, &m_Release, true))
		return false;

	return true;

}

bool GOSoundProvider::SaveCacheAudioSection
	(GOrgueCacheWriter& cache, const GOAudioSection* section, bool save_align_tracker)
{

	if (!cache.Write(section, sizeof(GOAudioSection)))
		return false;

	if (!cache.WriteBlock(section->data, section->alloc_size))
		return false;

	if (save_align_tracker)
	{

		bool has_align_tracker = section->release_aligner != NULL;
		if (!cache.Write(&has_align_tracker, sizeof(has_align_tracker)))
			return false;

		if (has_align_tracker)
			if (!section->release_aligner->Save(cache))
				return false;

	}

	return true;

}

bool GOSoundProvider::SaveCache(GOrgueCacheWriter& cache)
{

	if (!SaveCacheAudioSection(cache, &m_Attack, false))
		return false;

	if (!SaveCacheAudioSection(cache, &m_Loop, false))
		return false;

	if (!SaveCacheAudioSection(cache, &m_Release, true))
		return false;

	return true;

}

void GOSoundProvider::GetMaxAmplitudeAndDerivative
	(GOAudioSection& section
	,int& runningMaxAmplitude
	,int& runningMaxDerivative
	)
{
	DecompressionCache cache;
	unsigned int sectionLen = section.sample_count;
	unsigned channels = section.GetChannels();

	InitDecompressionCache(cache);

	int f = 0; /* to avoid compiler warning */
	for (unsigned int i = 0; i < sectionLen; i++)
	{

		/* Get sum of amplitudes in channels */
		int f_p = f;
		f = 0;
		for (unsigned int j = 0; j < channels; j++)
			f += GetAudioSectionSample(section, i, j, &cache);

		if (abs(f) > runningMaxAmplitude)
			runningMaxAmplitude = abs(f);

		if (i == 0)
			continue;

		/* Get v */
		int v = f - f_p;
		if (abs(v) > runningMaxDerivative)
			runningMaxDerivative = abs(v);

	}

}

/* REGRESSION TODO: It would be good for somebody to do some rigorous
 * testing on this code before this comment is removed. It is designed
 * based on a dream. */
void GOSoundProvider::ComputeReleaseAlignmentInfo()
{

	DELETE_AND_NULL(m_Release.release_aligner);

	/* Find the maximum amplitude and derivative of the waveform */
	int phase_align_max_amplitude = 0;
	int phase_align_max_derivative = 0;
	GetMaxAmplitudeAndDerivative(m_Attack,  phase_align_max_amplitude, phase_align_max_derivative);
	GetMaxAmplitudeAndDerivative(m_Loop,    phase_align_max_amplitude, phase_align_max_derivative);
	GetMaxAmplitudeAndDerivative(m_Release, phase_align_max_amplitude, phase_align_max_derivative);

	if ((phase_align_max_derivative != 0) && (phase_align_max_amplitude != 0))
	{

		m_Release.release_aligner = new GOrgueReleaseAlignTable();
		m_Release.release_aligner->ComputeTable
			(m_Release
			,phase_align_max_amplitude
			,phase_align_max_derivative
			,m_SampleRate
			);

	}

}

int GOSoundProvider::IsOneshot() const
{
	return (m_Loop.data == NULL);
}

void GOSoundProvider::SetTuning(float cent)
{
	m_Tuning = pow (pow(2, 1.0 / 1200.0), cent);
}
