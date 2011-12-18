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

#include "GOSoundAudioSection.h"
#include "GOrgueCache.h"
#include "GOrgueCacheWriter.h"
#include "GOrgueReleaseAlignTable.h"
#include "GOrgueMemoryPool.h"

GOAudioSection::GOAudioSection(GOrgueMemoryPool& pool)
	: m_Pool(pool)
	, m_Data(NULL)
	, m_ReleaseAligner(NULL)
{
	ClearData();
}

GOAudioSection::~GOAudioSection()
{
	ClearData();
}

void GOAudioSection::ClearData()
{
	m_Size = 0;
	m_AllocSize = 0;
	m_SampleCount = 0;
	m_SampleRate = 0;
	memset(m_History, 0, sizeof(m_History));
	m_Type = AC_UNCOMPRESSED8_MONO;
	m_Stage = GSS_ATTACK;
	if (m_Data)
	{
		m_Pool.Free(m_Data);
		m_Data = NULL;
	}
	if (m_ReleaseAligner)
	{
		delete m_ReleaseAligner;
		m_ReleaseAligner = NULL;
	}
	m_SampleFracBits = 0;
}

bool GOAudioSection::LoadCache(GOrgueCache& cache)
{
	if (!cache.Read(&m_Size, sizeof(m_Size)))
		return false;
	if (!cache.Read(&m_AllocSize, sizeof(m_AllocSize)))
		return false;
	if (!cache.Read(&m_SampleCount, sizeof(m_SampleCount)))
		return false;
	if (!cache.Read(&m_SampleRate, sizeof(m_SampleRate)))
		return false;
	if (!cache.Read(&m_Type, sizeof(m_Type)))
		return false;
	if (!cache.Read(&m_Stage, sizeof(m_Stage)))
		return false;
	if (!cache.Read(&m_History, sizeof(m_History)))
		return false;
	if (!cache.Read(&m_SampleFracBits, sizeof(m_SampleFracBits)))
		return false;
	bool load_align_tracker;
	if (!cache.Read(&load_align_tracker, sizeof(load_align_tracker)))
		return false;

	m_ReleaseAligner = NULL;
	m_Data = (unsigned char*)cache.ReadBlock(m_AllocSize);
	if (!m_Data)
		return false;

	if (load_align_tracker)
	{
		m_ReleaseAligner = new GOrgueReleaseAlignTable();
		if (!m_ReleaseAligner->Load(cache))
			return false;
	}

	return true;
}

bool GOAudioSection::SaveCache(GOrgueCacheWriter& cache) const
{
	if (!cache.Write(&m_Size, sizeof(m_Size)))
		return false;
	if (!cache.Write(&m_AllocSize, sizeof(m_AllocSize)))
		return false;
	if (!cache.Write(&m_SampleCount, sizeof(m_SampleCount)))
		return false;
	if (!cache.Write(&m_SampleRate, sizeof(m_SampleRate)))
		return false;
	if (!cache.Write(&m_Type, sizeof(m_Type)))
		return false;
	if (!cache.Write(&m_Stage, sizeof(m_Stage)))
		return false;
	if (!cache.Write(&m_History, sizeof(m_History)))
		return false;
	if (!cache.Write(&m_SampleFracBits, sizeof(m_SampleFracBits)))
		return false;
	const bool save_align_tracker = (m_ReleaseAligner != NULL);
	if (!cache.Write(&save_align_tracker, sizeof(save_align_tracker)))
		return false;

	if (!cache.WriteBlock(m_Data, m_AllocSize))
		return false;

	if (save_align_tracker)
	{
		if (!m_ReleaseAligner->Save(cache))
			return false;
	}

	return true;
}





