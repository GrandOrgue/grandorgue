/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOSOUNDPROVIDER_H_
#define GOSOUNDPROVIDER_H_

#include "ptrvector.h"
#include "GOrgueStatisticCallback.h"
#include <vector>

class GOAudioSection;
class GOrgueCache;
class GOrgueCacheWriter;
class GOrgueMemoryPool;

typedef struct audio_section_stream_s audio_section_stream;

typedef struct
{
	int sample_group;
	unsigned min_attack_velocity;
	unsigned max_released_time;
} attack_section_info;

typedef struct
{
	int sample_group;
	unsigned max_playback_time;
} release_section_info;

class GOSoundProvider : public GOrgueStatisticCallback
{

protected:
 	unsigned m_MidiKeyNumber;
	float m_MidiPitchFract;
	float             m_Gain;
	float             m_Tuning;
	bool              m_SampleGroup;
	ptr_vector<GOAudioSection> m_Attack;
	std::vector<attack_section_info> m_AttackInfo;
	ptr_vector<GOAudioSection> m_Release;
	std::vector<release_section_info> m_ReleaseInfo;
	GOrgueMemoryPool &m_pool;
	void ComputeReleaseAlignmentInfo();
	float m_VelocityVolumeBase;
	float m_VelocityVolumeIncrement;
	unsigned m_ReleaseCrossfadeLength;
    unsigned m_ReleaseTruncationLength;

public:
	GOSoundProvider(GOrgueMemoryPool& pool);
	virtual ~GOSoundProvider();

	void ClearData();

	virtual bool LoadCache(GOrgueCache& cache);
	virtual bool SaveCache(GOrgueCacheWriter& cache);

	void UseSampleGroup(unsigned sample_group);
	void SetVelocityParameter(float min_volume, float max_volume);

	const GOAudioSection* GetRelease(const audio_section_stream* handle, double playback_time) const;
	const GOAudioSection* GetAttack(unsigned velocity, unsigned released_time) const;
	float GetGain() const;
	int IsOneshot() const;

	float GetTuning() const;
	void SetTuning(float cent);

	unsigned GetMidiKeyNumber() const;
	float GetMidiPitchFract() const;
	unsigned GetReleaseCrossfadeLength() const;
    unsigned GetReleaseTruncationLength() const;

	float GetVelocityVolume(unsigned velocity) const;

	bool checkForMissingAttack();
	bool checkForMissingRelease();
	bool checkMissingRelease();
	bool checkNotNecessaryRelease();

	GOrgueSampleStatistic GetStatistic();
};

inline
float GOSoundProvider::GetGain() const
{
	return m_Gain;
}

inline
float GOSoundProvider::GetTuning() const
{
	return m_Tuning;
}

#endif /* GOSOUNDPROVIDER_H_ */
