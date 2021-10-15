/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOSoundProvider.h"

#include "GOSoundAudioSection.h"
#include "GOrgueCache.h"
#include "GOrgueCacheWriter.h"
#include "GOrgueMemoryPool.h"
#include "GOrgueReleaseAlignTable.h"
#include "GOrgueSampleStatistic.h"
#include <wx/intl.h>

#define DELETE_AND_NULL(x) do { if (x) { delete x; x = NULL; } } while (0)

GOSoundProvider::GOSoundProvider(GOrgueMemoryPool& pool) :
	m_MidiKeyNumber(0),
	m_MidiPitchFract(0),
	m_Tuning(1),
	m_SampleGroup(0),
	m_Attack(),
	m_AttackInfo(),
	m_Release(),
	m_ReleaseInfo(),
	m_pool(pool),
	m_VelocityVolumeBase(1),
	m_VelocityVolumeIncrement(0),
	m_ReleaseCrossfadeLength(184),
    m_ReleaseTruncationLength(0)
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
	if (!cache.Read(&m_ReleaseCrossfadeLength, sizeof(m_ReleaseCrossfadeLength)))
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

void GOSoundProvider::UseSampleGroup(unsigned sample_group)
{
	m_SampleGroup = sample_group;
}

bool GOSoundProvider::SaveCache(GOrgueCacheWriter& cache)
{
	if (!cache.Write(&m_MidiKeyNumber, sizeof(m_MidiKeyNumber)))
		return false;
	if (!cache.Write(&m_MidiPitchFract, sizeof(m_MidiPitchFract)))
		return false;
	if (!cache.Write(&m_ReleaseCrossfadeLength, sizeof(m_ReleaseCrossfadeLength)))
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
	for (int k = -1; k < 2; k++)
	{
		sections.clear();
		for(unsigned i = 0; i < m_Attack.size(); i++)
			if (m_AttackInfo[i].sample_group == k)
				sections.push_back(m_Attack[i]);
		for(unsigned i = 0; i < m_Release.size(); i++)
			if (m_ReleaseInfo[i].sample_group == k)
				m_Release[i]->SetupStreamAlignment(sections, 0);

		sections.clear();
		for(unsigned i = 0; i < m_Attack.size(); i++)
			if (m_AttackInfo[i].sample_group != k)
				sections.push_back(m_Attack[i]);
		for(unsigned i = 0; i < m_Attack.size(); i++)
			if (m_AttackInfo[i].sample_group == k)
				m_Attack[i]->SetupStreamAlignment(sections, 1);
	}

	for(unsigned i = 1; i < m_Attack.size(); i++)
		if (m_Attack[0]->IsOneshot() != m_Attack[i]->IsOneshot())
			throw (wxString)_("Mixing of percussive and non-percussive samples in one pipe not allowed");
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

unsigned GOSoundProvider::GetReleaseCrossfadeLength() const
{
	return m_ReleaseCrossfadeLength;
}

unsigned GOSoundProvider::GetReleaseTruncationLength() const
{
    return m_ReleaseTruncationLength;
}

void GOSoundProvider::SetVelocityParameter(float min_volume, float max_volume)
{
	if (min_volume == max_volume)
	{
		m_VelocityVolumeBase = min_volume / 100;
		m_VelocityVolumeIncrement = 0;
	}
	else
	{
		m_VelocityVolumeBase = min_volume / 100;
		m_VelocityVolumeIncrement = (max_volume - min_volume) / 12700.0;
	}
}


float GOSoundProvider::GetVelocityVolume(unsigned velocity) const
{
	return m_VelocityVolumeBase + (velocity * m_VelocityVolumeIncrement);
}

const GOAudioSection* GOSoundProvider::GetAttack(unsigned velocity, unsigned released_time) const
{
	const unsigned x = abs(rand());
	int best_match = -1;
	for (unsigned i = 0; i < m_Attack.size(); i++)
	{
		const unsigned idx = (i + x) % m_Attack.size();
		if (m_AttackInfo[idx].sample_group != -1 && 
		    m_AttackInfo[idx].sample_group != m_SampleGroup)
			continue;
		if (m_AttackInfo[idx].min_attack_velocity > velocity)
			continue;
		if (m_AttackInfo[idx].max_released_time < released_time)
			continue;
		if (best_match == -1)
			best_match = idx;
		else if (m_AttackInfo[best_match].min_attack_velocity < m_AttackInfo[idx].min_attack_velocity &&
			 m_AttackInfo[best_match].max_released_time > m_AttackInfo[idx].max_released_time)
			best_match = idx;
	}

	if (best_match != -1)
		return m_Attack[best_match];
	return NULL;
}

const GOAudioSection* GOSoundProvider::GetRelease(const audio_section_stream* handle, double playback_time) const
{
	unsigned attack_idx = 0;
	unsigned time = std::min(playback_time, 3600.0) * 1000;
	for (unsigned i = 0; i < m_Attack.size(); i++)
	{
		if (handle->audio_section == m_Attack[i])
			attack_idx = i;
	}

	const unsigned x = abs(rand());
	int best_match = -1;
	for (unsigned i = 0; i < m_Release.size(); i++)
	{
		const unsigned idx = (i + x) % m_Release.size();
		if (m_ReleaseInfo[idx].sample_group != m_AttackInfo[attack_idx].sample_group)
			continue;
		if (m_ReleaseInfo[idx].max_playback_time < time)
			continue;
		if (best_match == -1)
			best_match = idx;
		else if (m_ReleaseInfo[best_match].max_playback_time > m_ReleaseInfo[idx].max_playback_time)
			best_match = idx;
	}

	if (best_match != -1)
		return m_Release[best_match];

	return NULL;
}

bool GOSoundProvider::checkForMissingRelease()
{
	for (int k = -1; k < 2; k++)
	{
		unsigned cnt = 0;
		bool max_release = false;
		for(unsigned i = 0; i < m_Release.size(); i++)
			if (m_ReleaseInfo[i].sample_group == k)
			{
				cnt++;
				if (m_ReleaseInfo[i].max_playback_time == (unsigned)-1)
					max_release = true;
			}
		if (cnt && !max_release)
			return true;
	}
	return false;
}

bool GOSoundProvider::checkForMissingAttack()
{
	for (int k = -1; k < 2; k++)
	{
		unsigned cnt = 0;
		bool max_release = false;
		for(unsigned i = 0; i < m_Attack.size(); i++)
			if (m_AttackInfo[i].sample_group == k)
			{
				cnt++;
				if (m_AttackInfo[i].max_released_time == (unsigned)-1)
					max_release = true;
			}
		if (cnt && !max_release)
			return true;
	}
	return false;
}

bool GOSoundProvider::checkMissingRelease()
{
	if (IsOneshot())
		return false;
	for (int k = -1; k < 2; k++)
	{
		bool found = false;
		for (unsigned i = 0; i < m_Attack.size() && !found; i++)
			if (m_AttackInfo[i].sample_group == k)
				found = true;
		if (!found)
			continue;
		found = false;
		for (unsigned i = 0; i < m_Release.size() && !found; i++)
			if (m_ReleaseInfo[i].sample_group == k)
				found = true;
		if (!found)
			return true;
	}
	return false;
}
bool GOSoundProvider::checkNotNecessaryRelease()
{
	if (IsOneshot() && m_Release.size())
		return true;
	return false;
}

GOrgueSampleStatistic GOSoundProvider::GetStatistic()
{
	GOrgueSampleStatistic stat;
	for(unsigned i = 0; i < m_Attack.size(); i++)
		stat.Cumulate(m_Attack[i]->GetStatistic());
	for(unsigned i = 0; i < m_Release.size(); i++)
		stat.Cumulate(m_Release[i]->GetStatistic());
	return stat;
}
