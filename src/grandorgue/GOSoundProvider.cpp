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
	m_Attack(),
	m_AttackInfo(),
	m_Release(),
	m_ReleaseInfo(),
	m_pool(pool)
{
	m_Gain = 0.0f;
}

GOSoundProvider::~GOSoundProvider()
{
}

void GOSoundProvider::ClearData()
{
	m_Attack.clear();
	m_AttackInfo.clear();
	m_Release.clear();
	m_ReleaseInfo.clear();
}

bool GOSoundProvider::LoadCache(GOrgueCache& cache)
{
	if (!cache.Read(&m_MidiKeyNumber, sizeof(m_MidiKeyNumber)))
		return false;
	if (!cache.Read(&m_MidiPitchFract, sizeof(m_MidiPitchFract)))
		return false;

	unsigned attacks;
	if (!cache.Read(&attacks, sizeof(attacks)))
		return false;
	for(unsigned i = 0; i < attacks; i++)
	{
		attack_section_info info;
		if (!cache.Read(&info, sizeof(info)))
			return false;
		m_AttackInfo.push_back(info);
		m_Attack.push_back(new GOAudioSection(m_pool));
		if (!m_Attack[i]->LoadCache(cache))
			return false;
	}

	unsigned releases;
	if (!cache.Read(&releases, sizeof(releases)))
		return false;
	for(unsigned i = 0; i < releases; i++)
	{
		release_section_info info;
		if (!cache.Read(&info, sizeof(info)))
			return false;
		m_ReleaseInfo.push_back(info);
		m_Release.push_back(new GOAudioSection(m_pool));
		if (!m_Release[i]->LoadCache(cache))
			return false;
	}

	return true;
}


bool GOSoundProvider::SaveCache(GOrgueCacheWriter& cache)
{
	if (!cache.Write(&m_MidiKeyNumber, sizeof(m_MidiKeyNumber)))
		return false;
	if (!cache.Write(&m_MidiPitchFract, sizeof(m_MidiPitchFract)))
		return false;

	unsigned attacks = m_Attack.size();
	if (!cache.Write(&attacks, sizeof(attacks)))
		return false;
	for(unsigned i = 0; i < m_Attack.size(); i++)
	{
		if (!cache.Write(&m_AttackInfo[i], sizeof(m_AttackInfo[i])))
			return false;
		if (!m_Attack[i]->SaveCache(cache))
			return false;
	}

	unsigned releases = m_Release.size();
	if (!cache.Write(&releases, sizeof(releases)))
		return false;
	for(unsigned i = 0; i < m_Release.size(); i++)
	{
		if (!cache.Write(&m_ReleaseInfo[i], sizeof(m_ReleaseInfo[i])))
			return false;
		if (!m_Release[i]->SaveCache(cache))
			return false;
	}

	return true;
}

void GOSoundProvider::ComputeReleaseAlignmentInfo()
{
	std::vector<const GOAudioSection*> sections;
	for(unsigned i = 0; i < m_Attack.size(); i++)
		sections.push_back(m_Attack[i]);
	for(unsigned i = 0; i < m_Release.size(); i++)
		m_Release[i]->SetupStreamAlignment(sections);
}

int GOSoundProvider::IsOneshot() const
{
	if (!m_Attack.size())
		return false;
	for(unsigned i = 0; i < m_Attack.size(); i++)
		if (!m_Attack[i]->IsOneshot())
			return false;

	return true;
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
	if (m_Attack.size() != 0)
		return m_Attack[0];

	return NULL;
}

const GOAudioSection* GOSoundProvider::GetRelease(const audio_section_stream* handle, double playback_time) const
{
	if (m_Release.size() != 0)
		return m_Release[0];

	return NULL;
}
