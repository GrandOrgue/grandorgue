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
	memset(&m_attack, 0, sizeof(m_attack));
	memset(&m_loop, 0, sizeof(m_loop));
	memset(&m_release, 0, sizeof(m_release));
	m_Channels = 0;
	ra_amp = 0;
	ra_shift = 0;
	m_SampleRate = 0;
}

GOSoundProvider::~GOSoundProvider()
{
	ClearData();
}

void GOSoundProvider::ClearData()
{
	FREE_AND_NULL(m_attack.data);
	FREE_AND_NULL(m_loop.data);
	FREE_AND_NULL(m_release.data);
	DELETE_AND_NULL(m_release.release_aligner);
}

bool GOSoundProvider::LoadCache(wxInputStream* cache)
{

	cache->Read(&m_attack, sizeof(m_attack));
	if (cache->LastRead() != sizeof(m_attack))
	{
		m_attack.data = NULL;
		return false;
	}
	m_attack.data = (unsigned char*)malloc(m_attack.alloc_size);
	if (m_attack.data == NULL)
		throw (wxString)_("< out of memory allocating samples");
	cache->Read(m_attack.data, m_attack.alloc_size);
	if (cache->LastRead() != m_attack.alloc_size)
		return false;

	cache->Read(&m_loop, sizeof(m_loop));
	if (cache->LastRead() != sizeof(m_loop))
	{
		m_loop.data = NULL;
		return false;
	}
	m_loop.data = (unsigned char*)malloc(m_loop.alloc_size);
	if (m_loop.data == NULL)
		throw (wxString)_("< out of memory allocating samples");
	cache->Read(m_loop.data, m_loop.alloc_size);
	if (cache->LastRead() != m_loop.alloc_size)
		return false;

	cache->Read(&m_release, sizeof(m_release));
	m_release.release_aligner = NULL;
	m_release.data = NULL;
	if (cache->LastRead() != sizeof(m_release))
		return false;

	m_release.data = (unsigned char*)malloc(m_release.alloc_size);
	if (m_release.data == NULL)
		throw (wxString)_("< out of memory allocating samples");
	cache->Read(m_release.data, m_release.alloc_size);
	if (cache->LastRead() != m_release.alloc_size)
		return false;

	cache->Read(&ra_amp, sizeof(ra_amp));
	if (cache->LastRead() != sizeof(ra_amp))
		return false;

	cache->Read(&ra_shift, sizeof(ra_shift));
	if (cache->LastRead() != sizeof(ra_shift))
		return false;

	bool release;
	cache->Read(&release, sizeof(release));
	if (cache->LastRead() != sizeof(release))
		return false;

	if (release)
	{
		m_release.release_aligner = new GOrgueReleaseAlignTable();
		if (!m_release.release_aligner->Load(cache))
			return false;
	}

	return true;

}

bool GOSoundProvider::SaveCache(wxOutputStream* cache)
{
	cache->Write(&m_attack, sizeof(m_attack));
	if (cache->LastWrite() != sizeof(m_attack))
		return false;
	cache->Write(m_attack.data, m_attack.alloc_size);
	if (cache->LastWrite() != m_attack.alloc_size)
		return false;

	cache->Write(&m_loop, sizeof(m_loop));
	if (cache->LastWrite() != sizeof(m_loop))
		return false;
	cache->Write(m_loop.data, m_loop.alloc_size);
	if (cache->LastWrite() != m_loop.alloc_size)
		return false;

	cache->Write(&m_release, sizeof(m_release));
	if (cache->LastWrite() != sizeof(m_release))
		return false;
	cache->Write(m_release.data, m_release.alloc_size);
	if (cache->LastWrite() != m_release.alloc_size)
		return false;

	cache->Write(&ra_amp, sizeof(ra_amp));
	if (cache->LastWrite() != sizeof(ra_amp))
		return false;

	cache->Write(&ra_shift, sizeof(ra_shift));
	if (cache->LastWrite() != sizeof(ra_shift))
		return false;

	bool release = m_release.release_aligner != NULL;
	cache->Write(&release, sizeof(release));
	if (cache->LastWrite() != sizeof(release))
		return false;

	if (release)
		if (!m_release.release_aligner->Save(cache))
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

	DELETE_AND_NULL(m_release.release_aligner);

	/* Find the maximum amplitude and derivative of the waveform */
	int phase_align_max_amplitude = 0;
	int phase_align_max_derivative = 0;
	GetMaxAmplitudeAndDerivative(m_attack,  phase_align_max_amplitude, phase_align_max_derivative);
	GetMaxAmplitudeAndDerivative(m_loop,    phase_align_max_amplitude, phase_align_max_derivative);
	GetMaxAmplitudeAndDerivative(m_release, phase_align_max_amplitude, phase_align_max_derivative);

	if ((phase_align_max_derivative != 0) && (phase_align_max_amplitude != 0))
	{

		m_release.release_aligner = new GOrgueReleaseAlignTable();
		m_release.release_aligner->ComputeTable
			(m_release
			,phase_align_max_amplitude
			,phase_align_max_derivative
			,m_SampleRate
			,m_Channels
			);

	}

}

int GOSoundProvider::IsOneshot() const
{
	return (m_loop.data == NULL);
}
