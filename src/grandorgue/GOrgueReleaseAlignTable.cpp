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

#include <wx/stream.h>

#include "GOrgueReleaseAlignTable.h"
#include "GOSoundAudioSection.h"
#include <stdlib.h>

#ifndef NDEBUG
#ifdef PALIGN_DEBUG
#include <stdio.h>
#endif
#endif

GOrgueReleaseAlignTable::GOrgueReleaseAlignTable()
{
	memset(m_PositionEntries, 0, sizeof(m_PositionEntries));
	memset(m_HistoryEntries, 0, sizeof(m_HistoryEntries));
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
	cache->Read(&m_PositionEntries, sizeof(m_PositionEntries));
	if (cache->LastRead() != sizeof(m_PositionEntries))
		return false;
	cache->Read(&m_HistoryEntries, sizeof(m_HistoryEntries));
	if (cache->LastRead() != sizeof(m_HistoryEntries))
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
	cache->Write(&m_PositionEntries, sizeof(m_PositionEntries));
	if (cache->LastWrite() != sizeof(m_PositionEntries))
		return false;
	cache->Write(&m_HistoryEntries, sizeof(m_HistoryEntries));
	if (cache->LastWrite() != sizeof(m_HistoryEntries))
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
	unsigned required_search_len = sample_rate / PHASE_ALIGN_MIN_FREQUENCY;
	unsigned release_len = release.size / (m_Channels * sizeof(wxInt16));
	if (release_len < required_search_len + BLOCK_HISTORY)
		return;
	/* If number of samples in the release is not enough to fill the release
	 * table, abort - release alignment probably wont help. */
	if (required_search_len < PHASE_ALIGN_AMPLITUDES * PHASE_ALIGN_DERIVATIVES * 2)
		return;

	/* Generate the release table using the small portion of the release... */
	bool found[PHASE_ALIGN_DERIVATIVES][PHASE_ALIGN_AMPLITUDES];
	memset(found, 0, sizeof(found));

	int f_p = 0;
	for (unsigned int j = 0; j < m_Channels; j++)
		f_p += ((wxInt16*)release.data)[(BLOCK_HISTORY - 1) * m_Channels + j];

	for (unsigned i = BLOCK_HISTORY; i < required_search_len; i++)
	{

		/* Store previous values */
		int f = 0;
		for (unsigned int j = 0; j < m_Channels; j++)
			f += ((wxInt16*)release.data)[i * m_Channels + j];

		/* Bring v into the range -1..2*m_PhaseAlignMaxDerivative-1 */
		int v_mod = (f - f_p) + m_PhaseAlignMaxDerivative - 1;
		int derivIndex = (PHASE_ALIGN_DERIVATIVES * v_mod) / (2 * m_PhaseAlignMaxDerivative);

		/* Bring f into the range -1..2*m_PhaseAlignMaxAmplitude-1 */
		int f_mod = f + m_PhaseAlignMaxAmplitude - 1;
		int ampIndex = (PHASE_ALIGN_AMPLITUDES * f_mod) / (2 * m_PhaseAlignMaxAmplitude);

		/* Store this release point if it was not already found */
		assert((derivIndex >= 0) && (derivIndex < PHASE_ALIGN_DERIVATIVES));
		assert((ampIndex >= 0)   && (ampIndex < PHASE_ALIGN_AMPLITUDES));
		derivIndex = (derivIndex < 0) ? 0 : ((derivIndex >= PHASE_ALIGN_DERIVATIVES) ? PHASE_ALIGN_DERIVATIVES-1 : derivIndex);
		ampIndex = (ampIndex < 0) ? 0 : ((ampIndex >= PHASE_ALIGN_AMPLITUDES) ? PHASE_ALIGN_AMPLITUDES-1 : ampIndex);
		if (!found[derivIndex][ampIndex])
		{
			m_PositionEntries[derivIndex][ampIndex] = i + 1;
			for (unsigned j = 0; j < BLOCK_HISTORY; j++)
				for (unsigned k = 0; k < MAX_OUTPUT_CHANNELS; k++)
					m_HistoryEntries[derivIndex][ampIndex][j * MAX_OUTPUT_CHANNELS + k]
							= (k < m_Channels)
							? ((wxInt16*)release.data)[(i + j - BLOCK_HISTORY) * m_Channels + k]
							: 0;
			found[derivIndex][ampIndex] = true;
		}

		f_p = f;

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
						if ((l & 1) == 0)
							sl = -sl;
						int sk = (k + 2) / 2;
						if ((k & 1) == 0)
							sk = -sk;
						if  (
								(i + sl < PHASE_ALIGN_DERIVATIVES)
								&&
								(i + sl >= 0)
								&&
								(j + sk < PHASE_ALIGN_AMPLITUDES)
								&&
								(j + sk >= 0)
								&&
								(found[i + sl][j + sk])
							)
							{
								m_PositionEntries[i][j] = m_PositionEntries[i + sl][j + sk];
								memcpy
									(m_HistoryEntries[i][j]
									,m_HistoryEntries[i+sl][j+sk]
									,sizeof(m_HistoryEntries[i][j])
									);
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
	) const
{

	/* Get combined release f's and v's */
	int f_mod = 0;
	int v_mod = 0;
	for (unsigned i = 0; i < MAX_OUTPUT_CHANNELS; i++)
	{
		f_mod += old_sampler.history[(BLOCK_HISTORY - 1)][i];
		v_mod += old_sampler.history[(BLOCK_HISTORY - 2)][i];
	}
	v_mod = f_mod - v_mod;

	/* Bring f and v into the range -1..2*m_PhaseAlignMaxDerivative-1 */
	v_mod += (m_PhaseAlignMaxDerivative - 1);
	f_mod += (m_PhaseAlignMaxAmplitude - 1);

	int derivIndex = m_PhaseAlignMaxDerivative ?
			(PHASE_ALIGN_DERIVATIVES * v_mod) / (2 * m_PhaseAlignMaxDerivative) :
			PHASE_ALIGN_DERIVATIVES / 2;

	/* Bring f into the range -1..2*m_PhaseAlignMaxAmplitude-1 */
	int ampIndex = m_PhaseAlignMaxAmplitude ?
			(PHASE_ALIGN_AMPLITUDES * f_mod) / (2 * m_PhaseAlignMaxAmplitude) :
			PHASE_ALIGN_AMPLITUDES / 2;

	/* Store this release point if it was not already found */
	assert((derivIndex >= 0) && (derivIndex < PHASE_ALIGN_DERIVATIVES));
	assert((ampIndex >= 0) && (ampIndex < PHASE_ALIGN_AMPLITUDES));
	derivIndex = (derivIndex < 0) ? 0 : ((derivIndex >= PHASE_ALIGN_DERIVATIVES) ? PHASE_ALIGN_DERIVATIVES-1 : derivIndex);
	ampIndex = (ampIndex < 0) ? 0 : ((ampIndex >= PHASE_ALIGN_AMPLITUDES) ? PHASE_ALIGN_AMPLITUDES-1 : ampIndex);
	release_sampler.position  = m_PositionEntries[derivIndex][ampIndex];
	memcpy
		(release_sampler.history
		,m_HistoryEntries[derivIndex][ampIndex]
		,sizeof(release_sampler.history)
		);

#ifndef NDEBUG
#ifdef PALIGN_DEBUG
	printf("setup release using alignment:\n");
	printf("  pos:    %d\n", release_sampler.position);
	printf("  derIdx: %d\n", derivIndex);
	printf("  ampIdx: %d\n", ampIndex);
#endif
#endif

}
