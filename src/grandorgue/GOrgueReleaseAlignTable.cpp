/*
 * GOrgueReleaseAlignTable.cpp
 *
 *  Created on: 26/04/2011
 *      Author: nappleton
 */

#include <wx/stream.h>

#include "GOrgueReleaseAlignTable.h"
#include <stdlib.h>

#ifndef NDEBUG
#ifdef PALIGN_DEBUG
#include <stdio.h>
#endif
#endif

GOrgueReleaseAlignTable::GOrgueReleaseAlignTable()
{
	memset(m_PhaseAlignmentTable, 0, sizeof(m_PhaseAlignmentTable));
	memset(m_PhaseAlignmentTable_f, 0, sizeof(m_PhaseAlignmentTable_f));
	memset(m_PhaseAlignmentTable_v, 0, sizeof(m_PhaseAlignmentTable_v));
	m_PhaseAlignMaxAmplitude = 0;
	m_PhaseAlignMaxDerivative = 0;
}

GOrgueReleaseAlignTable::~GOrgueReleaseAlignTable()
{

}

bool GOrgueReleaseAlignTable::Load(wxInputStream* cache)
{
	cache->Read(&m_PhaseAlignMaxAmplitude, sizeof(m_PhaseAlignMaxAmplitude));
	if (cache->LastRead() != sizeof(m_PhaseAlignMaxAmplitude))
		return false;
	cache->Read(&m_PhaseAlignMaxDerivative, sizeof(m_PhaseAlignMaxDerivative));
	if (cache->LastRead() != sizeof(m_PhaseAlignMaxDerivative))
		return false;
	cache->Read(&m_PhaseAlignmentTable, sizeof(m_PhaseAlignmentTable));
	if (cache->LastRead() != sizeof(m_PhaseAlignmentTable))
		return false;
	cache->Read(&m_PhaseAlignmentTable_f, sizeof(m_PhaseAlignmentTable_f));
	if (cache->LastRead() != sizeof(m_PhaseAlignmentTable_f))
		return false;
	cache->Read(&m_PhaseAlignmentTable_v, sizeof(m_PhaseAlignmentTable_v));
	if (cache->LastRead() != sizeof(m_PhaseAlignmentTable_v))
		return false;
	return true;
}

bool GOrgueReleaseAlignTable::Save(wxOutputStream* cache)
{
	cache->Write(&m_PhaseAlignMaxAmplitude, sizeof(m_PhaseAlignMaxAmplitude));
	if (cache->LastWrite() != sizeof(m_PhaseAlignMaxAmplitude))
		return false;
	cache->Write(&m_PhaseAlignMaxDerivative, sizeof(m_PhaseAlignMaxDerivative));
	if (cache->LastWrite() != sizeof(m_PhaseAlignMaxDerivative))
		return false;
	cache->Write(&m_PhaseAlignmentTable, sizeof(m_PhaseAlignmentTable));
	if (cache->LastWrite() != sizeof(m_PhaseAlignmentTable))
		return false;
	cache->Write(&m_PhaseAlignmentTable_f, sizeof(m_PhaseAlignmentTable_f));
	if (cache->LastWrite() != sizeof(m_PhaseAlignmentTable_f))
		return false;
	cache->Write(&m_PhaseAlignmentTable_v, sizeof(m_PhaseAlignmentTable_v));
	if (cache->LastWrite() != sizeof(m_PhaseAlignmentTable_v))
		return false;
	return true;
}


void GOrgueReleaseAlignTable::ComputeTable
	(const AUDIO_SECTION& release
	,const int phase_align_max_amplitude
	,const int phase_align_max_derivative
	,const unsigned int sample_rate
	,const unsigned int channels
	)
{

	m_Channels = channels;
	m_PhaseAlignMaxDerivative = phase_align_max_derivative;
	m_PhaseAlignMaxAmplitude = phase_align_max_amplitude;

	/* We will use a short portion of the release to analyse to get the
	 * release offset table. This length is defined by the
	 * PHASE_ALIGN_MIN_FREQUENCY macro and should be set to the lowest
	 * frequency pipe you would ever expect... if this length is greater
	 * than the length of the release, truncate it */
	unsigned searchLen = sample_rate / PHASE_ALIGN_MIN_FREQUENCY;
	unsigned releaseLen = release.size / (m_Channels * sizeof(wxInt16));
	if (searchLen > releaseLen)
		searchLen = releaseLen;

	/* If number of samples in the release is not enough to fill the release
	 * table, abort - release alignment probably wont help. */
	if (searchLen < PHASE_ALIGN_AMPLITUDES * PHASE_ALIGN_DERIVATIVES * 2)
		return;

	/* Generate the release table using the small portion of the release... */
	bool found[PHASE_ALIGN_DERIVATIVES][PHASE_ALIGN_AMPLITUDES];
	memset(found, 0, sizeof(found));
	int f[MAX_OUTPUT_CHANNELS];
	int v[MAX_OUTPUT_CHANNELS];
	for (unsigned i = 0; i < searchLen; i++)
	{

		/* Store previous values */
		int f_p[MAX_OUTPUT_CHANNELS];
		memcpy(f_p, f, sizeof(int) * MAX_OUTPUT_CHANNELS);

		int fsum = 0;
		for (unsigned int j = 0; j < m_Channels; j++)
		{
			f[j] = *((wxInt16*)&release.data[(i * m_Channels + j) * sizeof(wxInt16)]);
			fsum += f[j];
		}

		if (i == 0)
			continue;

		int vsum = 0;
		for (unsigned int j = 0; j < m_Channels; j++)
		{
			v[j] = f[j] - f_p[j];
			vsum += v[j];
		}

		/* Bring v into the range -1..2*m_PhaseAlignMaxDerivative-1 */
		int v_mod = vsum + m_PhaseAlignMaxDerivative - 1;
		int derivIndex = (PHASE_ALIGN_DERIVATIVES * v_mod) / (2 * m_PhaseAlignMaxDerivative);

		/* Bring f into the range -1..2*m_PhaseAlignMaxAmplitude-1 */
		int f_mod = fsum + m_PhaseAlignMaxAmplitude - 1;
		int ampIndex = (PHASE_ALIGN_AMPLITUDES * f_mod) / (2 * m_PhaseAlignMaxAmplitude);

		/* Store this release point if it was not already found */
		assert((derivIndex >= 0) && (derivIndex < PHASE_ALIGN_DERIVATIVES));
		assert((ampIndex >= 0)   && (ampIndex < PHASE_ALIGN_AMPLITUDES));
		if (!found[derivIndex][ampIndex])
		{
			m_PhaseAlignmentTable[derivIndex][ampIndex] = i * m_Channels * sizeof(wxInt16);
			m_PhaseAlignmentTable_f[derivIndex][ampIndex] = fsum;
			m_PhaseAlignmentTable_v[derivIndex][ampIndex] = vsum;
			found[derivIndex][ampIndex] = true;
		}

	}

#ifndef NDEBUG
#ifdef PALIGN_DEBUG
	/* print some phase debugging information */
	for (unsigned int i = 0; i < PHASE_ALIGN_DERIVATIVES; i++)
	{
		printf("deridx: %d\n", i);
		for (unsigned int j = 0; j < PHASE_ALIGN_AMPLITUDES; j++)
			if (found[i][j])
				printf("  idx %d: found\n", j);
			else
				printf("  idx %d: not found\n", j);

	}
#endif
#endif

	/* Phase 2, if there are any entries in the table which were not found,
	 * attempt to fill them with the nearest available value. */
	for (int i = 0; i < PHASE_ALIGN_DERIVATIVES; i++)
		for (int j = 0; j < PHASE_ALIGN_AMPLITUDES; j++)
			if (!found[i][j])
			{
				bool foundsecond = false;
				for (int l = 0; (l < PHASE_ALIGN_DERIVATIVES) && (!foundsecond); l++)
					for (int k = 0; (k < PHASE_ALIGN_AMPLITUDES) && (!foundsecond); k++)
					{
						foundsecond = true;
						int sl = (l + 1) / 2;
						if ((sl & 1) == 0)
							sl = -sl;
						if ((i + sl < PHASE_ALIGN_DERIVATIVES) && (i + sl >= 0))
						{
							if ((j + k < PHASE_ALIGN_AMPLITUDES) && (found[i + sl][j + k]))
							{
								m_PhaseAlignmentTable[i][j] = m_PhaseAlignmentTable[i + sl][j + k];
								m_PhaseAlignmentTable_f[i][j] = m_PhaseAlignmentTable_f[i + sl][j + k];
								m_PhaseAlignmentTable_v[i][j] = m_PhaseAlignmentTable_v[i + sl][j + k];
							}
							else if ((j - k >= 0) && (found[i + sl][j - k]))
							{
								m_PhaseAlignmentTable[i][j] = m_PhaseAlignmentTable[i + sl][j - k];
								for (unsigned int z = 0; z < m_Channels; z++)
								{
									m_PhaseAlignmentTable_f[i][j] = m_PhaseAlignmentTable_f[i + sl][j - k];
									m_PhaseAlignmentTable_v[i][j] = m_PhaseAlignmentTable_v[i + sl][j - k];
								}
							}
							else
							{
								foundsecond = false;
							}
						}
						else
						{
							foundsecond = false;
						}
					}

				assert(foundsecond);
				foundsecond = false;

			}

}

void GOrgueReleaseAlignTable::SetupRelease
	(GO_SAMPLER& release_sampler
	,const GO_SAMPLER& old_sampler
	)
{

	/* Get combined release f's and v's
	 * TODO: it might be good to check that the compiler
	 * un-rolls this loop */
	int f_mod = old_sampler.release_tracker.f[0];
	int v_mod = f_mod - old_sampler.release_tracker.f[1];

	/* Bring f and v into the range -1..2*m_PhaseAlignMaxDerivative-1 */
	v_mod += (m_PhaseAlignMaxDerivative - 1);
	f_mod += (m_PhaseAlignMaxAmplitude - 1);

	int derivIndex = m_PhaseAlignMaxDerivative ?
			(PHASE_ALIGN_DERIVATIVES * v_mod) / (2 * m_PhaseAlignMaxDerivative) :
			PHASE_ALIGN_DERIVATIVES / 2;

	assert((derivIndex >= 0) && (derivIndex < PHASE_ALIGN_DERIVATIVES));

	/* Bring f into the range -1..2*m_PhaseAlignMaxAmplitude-1 */
	int ampIndex = m_PhaseAlignMaxAmplitude ?
			(PHASE_ALIGN_AMPLITUDES * f_mod) / (2 * m_PhaseAlignMaxAmplitude) :
			PHASE_ALIGN_AMPLITUDES / 2;

	assert((ampIndex >= 0) && (ampIndex < PHASE_ALIGN_AMPLITUDES));

	/* Store this release point if it was not already found */
	release_sampler.position  = m_PhaseAlignmentTable[derivIndex][ampIndex];
	release_sampler.release_tracker.f[0] = m_PhaseAlignmentTable_f[derivIndex][ampIndex];
	release_sampler.release_tracker.f[1] = release_sampler.release_tracker.f[0] - m_PhaseAlignmentTable_v[derivIndex][ampIndex];

#ifndef NDEBUG
#ifdef PALIGN_DEBUG
	printf("setup release using alignment:\n");
	printf("  pos:    %d\n", release_sampler.position);
	printf("  derIdx: %d\n", derivIndex);
	printf("  ampIdx: %d\n", ampIndex);
#endif
#endif

}
