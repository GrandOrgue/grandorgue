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
#include <stdlib.h>

GOrgueReleaseAlignTable::GOrgueReleaseAlignTable()
{
	m_Channels = 0;
	memset(m_Table, 0, sizeof(m_Table));
}

GOrgueReleaseAlignTable::~GOrgueReleaseAlignTable()
{

}

bool GOrgueReleaseAlignTable::Load(wxInputStream* cache)
{
	cache->Read(&m_Table, sizeof(m_Table));
	if (cache->LastRead() != sizeof(m_Table))
		return false;
	cache->Read(&m_Channels, sizeof(m_Channels));
	if (cache->LastRead() != sizeof(m_Channels))
		return false;
	return true;
}

bool GOrgueReleaseAlignTable::Save(wxOutputStream* cache)
{
	cache->Write(&m_Table, sizeof(m_Table));
	if (cache->LastWrite() != sizeof(m_Table))
		return false;
	cache->Write(&m_Channels, sizeof(m_Channels));
	if (cache->LastWrite() != sizeof(m_Channels))
		return false;
	return true;
}


void GOrgueReleaseAlignTable::ComputeTable
	(const AUDIO_SECTION& release
	,const unsigned int sample_rate
	,const unsigned int channels
	)
{
	m_Channels = channels;
	memset(m_Table, 0, sizeof(m_Table));

	unsigned release_length = release.size / (m_Channels * sizeof(wxInt16));
	unsigned search_length = BLOCK_HISTORY + TABLE_SIZE;
	if (search_length > release_length)
		return;

	const wxInt16* data = (wxInt16*)release.data;
	for (unsigned i = 0; i < search_length; i++)
		for (unsigned j = 0; j < channels; j++)
			m_Table[i * MAX_OUTPUT_CHANNELS + j] = data[i * channels + j];
}

void GOrgueReleaseAlignTable::SetupRelease
	(GO_SAMPLER& release_sampler
	,const GO_SAMPLER& old_sampler
	) const
{
	int min_error = 0;
	unsigned min_error_pos = 0;
	unsigned i;

	for (i = 0; i < TABLE_SIZE; i++)
	{
		int this_error = 0;
		for (unsigned j = 0; (j < BLOCK_HISTORY) && (this_error < INT_MAX); j++)
			for (unsigned k = 0; (k < m_Channels) && (this_error < INT_MAX); k++)
			{
				int error = abs(old_sampler.history[j * MAX_OUTPUT_CHANNELS + k] - m_Table[(i + j) * MAX_OUTPUT_CHANNELS + k]);
				if (error > 8192)
				{
					this_error = INT_MAX;
				}
				else
				{
					error = (error * error) >> 1;
					this_error += error;
				}
			}
		if ((this_error < min_error) || (i == 0))
		{
			min_error_pos = i;
			min_error = this_error;
			if (min_error < 10)
				break;
		}
	}

	release_sampler.position = (BLOCK_HISTORY + min_error_pos) * m_Channels * sizeof(wxInt16);
	memcpy
		(release_sampler.history
		,&(m_Table[i * MAX_OUTPUT_CHANNELS])
		,sizeof(release_sampler.history)
		);
}
