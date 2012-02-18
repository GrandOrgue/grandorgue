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
 * MA 02111-1307, USA.
 */

#include "GOSoundProvider.h"
#include "GOrgueReleaseAlignTable.h"
#include "GOrgueCache.h"
#include "GOrgueCacheWriter.h"
#include "GOrgueMemoryPool.h"
#include <wx/wx.h>

#define DELETE_AND_NULL(x) do { if (x) { delete x; x = NULL; } } while (0)

GOSoundProvider::GOSoundProvider(GOrgueMemoryPool& pool) :
	m_MidiKeyNumber(0),
	m_MidiPitchFract(0),
	m_Tuning(1),
	m_Attack(pool),
	m_Release(pool),
	m_pool(pool)
{
	m_Gain = 0.0f;
	m_SampleRate = 0;
}

GOSoundProvider::~GOSoundProvider()
{
}

void GOSoundProvider::ClearData()
{
	m_Attack.ClearData();
	m_Release.ClearData();
}

bool GOSoundProvider::LoadCache(GOrgueCache& cache)
{
	if (!cache.Read(&m_MidiKeyNumber, sizeof(m_MidiKeyNumber)))
		return false;
	if (!cache.Read(&m_MidiPitchFract, sizeof(m_MidiPitchFract)))
		return false;

	if (!m_Attack.LoadCache(cache))
		return false;

	if (!m_Release.LoadCache(cache))
		return false;

	return true;
}


bool GOSoundProvider::SaveCache(GOrgueCacheWriter& cache)
{
	if (!cache.Write(&m_MidiKeyNumber, sizeof(m_MidiKeyNumber)))
		return false;
	if (!cache.Write(&m_MidiPitchFract, sizeof(m_MidiPitchFract)))
		return false;

	if (!m_Attack.SaveCache(cache))
		return false;

	if (!m_Release.SaveCache(cache))
		return false;

	return true;
}

void GOSoundProvider::ComputeReleaseAlignmentInfo()
{
	std::vector<const GOAudioSection*> sections;
	sections.push_back(&m_Attack);
	m_Release.SetupStreamAlignment(sections);
}

int GOSoundProvider::IsOneshot() const
{
	return m_Attack.IsOneshot();
}

void GOSoundProvider::SetTuning(float cent)
{
	m_Tuning = pow (pow(2, 1.0 / 1200.0), cent);
}

unsigned GOSoundProvider::GetMidiKeyNumber() const
{
	return m_MidiKeyNumber;
}

float GOSoundProvider::GetMidiPitchFract() const
{
	return m_MidiPitchFract;
}

const GOAudioSection* GOSoundProvider::GetAttack() const
{
	return &m_Attack;
}

const GOAudioSection* GOSoundProvider::GetRelease(const audio_section_stream* handle, double playback_time) const
{
	return &m_Release;
}
