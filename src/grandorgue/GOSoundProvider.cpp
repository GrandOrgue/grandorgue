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

#include "GOSoundProvider.h"
#include "GOrgueReleaseAlignTable.h"

#define FREE_AND_NULL(x) do { if (x) { free(x); x = NULL; } } while (0)
#define DELETE_AND_NULL(x) do { if (x) { delete x; x = NULL; } } while (0)

GOSoundProvider::GOSoundProvider()
{
	memset(&m_Attack, 0, sizeof(m_Attack));
	memset(&m_Loop, 0, sizeof(m_Loop));
	memset(&m_Release, 0, sizeof(m_Release));
	m_Channels = 0;
	m_Gain = 0.0f;
	m_SampleRate = 0;
}

GOSoundProvider::~GOSoundProvider()
{
	ClearData();
}

void GOSoundProvider::ClearData()
{
	FREE_AND_NULL(m_Attack.data);
	FREE_AND_NULL(m_Loop.data);
	FREE_AND_NULL(m_Release.data);
	DELETE_AND_NULL(m_Release.release_aligner);
	memset(&m_Attack, 0, sizeof(m_Attack));
	memset(&m_Loop, 0, sizeof(m_Loop));
	memset(&m_Release, 0, sizeof(m_Release));
}

bool GOSoundProvider::LoadCacheAudioSection
	(wxInputStream* cache, AUDIO_SECTION* section, bool load_align_tracker)
{

	cache->Read(section, sizeof(AUDIO_SECTION));
	section->release_aligner = NULL;
	section->data = NULL;
	if (cache->LastRead() != sizeof(AUDIO_SECTION))
		return false;

	section->data = (unsigned char*)malloc(section->alloc_size);
	if (section->data == NULL)
		throw (wxString)_("< out of memory allocating samples");

	cache->Read(section->data, section->alloc_size);
	if (cache->LastRead() != section->alloc_size)
		return false;

	if (load_align_tracker)
	{

		bool has_align_tracker;
		cache->Read(&has_align_tracker, sizeof(has_align_tracker));
		if (cache->LastRead() != sizeof(has_align_tracker))
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

bool GOSoundProvider::LoadCache(wxInputStream* cache)
{

	cache->Read(&m_Gain, sizeof(m_Gain));
	if (cache->LastRead() != sizeof(m_Gain))
		return false;

	if (!LoadCacheAudioSection(cache, &m_Attack, false))
		return false;

	if (!LoadCacheAudioSection(cache, &m_Loop, false))
		return false;

	if (!LoadCacheAudioSection(cache, &m_Release, true))
		return false;

	return true;

}

bool GOSoundProvider::SaveCacheAudioSection
	(wxOutputStream* cache, const AUDIO_SECTION* section, bool save_align_tracker)
{

	cache->Write(section, sizeof(AUDIO_SECTION));
	if (cache->LastWrite() != sizeof(AUDIO_SECTION))
		return false;

	cache->Write(section->data, section->alloc_size);
	if (cache->LastWrite() != section->alloc_size)
		return false;

	if (save_align_tracker)
	{

		bool has_align_tracker = section->release_aligner != NULL;
		cache->Write(&has_align_tracker, sizeof(has_align_tracker));
		if (cache->LastWrite() != sizeof(has_align_tracker))
			return false;

		if (has_align_tracker)
			if (!section->release_aligner->Save(cache))
				return false;

	}

	return true;

}

bool GOSoundProvider::SaveCache(wxOutputStream* cache)
{

	cache->Write(&m_Gain, sizeof(m_Gain));
	if (cache->LastWrite() != sizeof(m_Gain))
		return false;

	if (!SaveCacheAudioSection(cache, &m_Attack, false))
		return false;

	if (!SaveCacheAudioSection(cache, &m_Loop, false))
		return false;

	if (!SaveCacheAudioSection(cache, &m_Release, true))
		return false;

	return true;

}

void GOSoundProvider::GetMaxAmplitudeAndDerivative
	(AUDIO_SECTION& section
	,int& runningMaxAmplitude
	,int& runningMaxDerivative
	)
{

	assert((section.size % (m_Channels * sizeof(wxInt16))) == 0);
	unsigned int sectionLen = section.size / (m_Channels * sizeof(wxInt16));

	int f = 0; /* to avoid compiler warning */
	for (unsigned int i = 0; i < sectionLen; i++)
	{

		/* Get sum of amplitudes in channels */
		int f_p = f;
		f = 0;
		for (unsigned int j = 0; j < m_Channels; j++)
			f += *((wxInt16*)&section.data[(i * m_Channels + j) * sizeof(wxInt16)]);

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
			,m_Channels
			);

	}

}

int GOSoundProvider::IsOneshot() const
{
	return (m_Loop.data == NULL);
}
