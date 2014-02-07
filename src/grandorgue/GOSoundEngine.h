/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2014 GrandOrgue contributors (see AUTHORS)
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
 */

#ifndef GOSOUNDENGINE_H_
#define GOSOUNDENGINE_H_

#include "GOSoundDefs.h"
#include "GOSoundResample.h"
#include "GOSoundSamplerPool.h"
#include "GOLock.h"
#include <vector>

#define GO_SOUND_BUFFER_SIZE ((MAX_FRAME_SIZE) * (MAX_OUTPUT_CHANNELS))

class GOrgueWindchest;
class GOSoundProvider;
class GOSoundReverb;
class GrandOrgueFile;
class GOrgueSettings;

typedef struct
{
	unsigned current_polyphony;
	double meter_left;
	double meter_right;
} METER_INFO;

typedef struct
{
	unsigned channels;
	std::vector< std::vector<float> > scale_factors;
} GOAudioOutputConfiguration;

struct GO_SAMPLER_T;
typedef struct GO_SAMPLER_T GO_SAMPLER;
typedef GO_SAMPLER* SAMPLER_HANDLE;

class GOSoundEngine
{

private:

	typedef int sound_buffer[GO_SOUND_BUFFER_SIZE];

	/* This is inteded to be struct, but needs copy constructors to make GOMutex work with std::vector */
	class GOSamplerEntry
	{
	public:
		GO_SAMPLER       *sampler;
		GO_SAMPLER       *new_sampler;
		GO_SAMPLER      **end_new_sampler;
		float             buff[GO_SOUND_BUFFER_SIZE];
		/* access lock for sampler data */
		GOMutex           lock;
		/* access lock for data buffer */
		GOMutex           mutex;
		GOrgueWindchest  *windchest;
		unsigned          done;
		unsigned          count;
		bool              is_tremulant;

		GOSamplerEntry()
		{
			sampler = NULL;
			new_sampler = NULL;
			end_new_sampler = NULL;
			windchest = NULL;
			done = 0;
			count = 0;
			is_tremulant = false;
		}

		GOSamplerEntry(const GOSamplerEntry& entry)
		{
			sampler = entry.sampler;
			new_sampler = entry.new_sampler;
			end_new_sampler = entry.end_new_sampler;
			windchest = entry.windchest;
			done = 0;
			count = entry.count;
			is_tremulant = entry.is_tremulant;
		}

		const GOSamplerEntry& operator=(const GOSamplerEntry& entry)
		{
			sampler = entry.sampler;
			new_sampler = entry.new_sampler;
			end_new_sampler = entry.end_new_sampler;
			windchest = entry.windchest;
			count = entry.count;
			is_tremulant = entry.is_tremulant;
			done = 0;
			return *this;
		}
	};

	class GOOutputGroup
	{
	public:
		float buff[GO_SOUND_BUFFER_SIZE];
		GOMutex mutex;
	        bool done;

		GOOutputGroup()
		{
			done = false;
		}

		GOOutputGroup(const GOOutputGroup& entry)
		{
			done = false;
		}

		const GOOutputGroup& operator=(const GOOutputGroup& entry)
		{
			done = false;
			return *this;
		}
	};

	class GOAudioOutput
	{
	public:
		unsigned channels;
		std::vector<float> scale_factors;
	};

	unsigned                      m_PolyphonySoftLimit;
	bool                          m_PolyphonyLimiting;
	bool                          m_ScaledReleases;
	bool                          m_ReleaseAlignmentEnabled;
	bool                          m_RandomizeSpeaking;
	int                           m_Volume;
	unsigned                      m_ReleaseLength;
	float                         m_Gain;
	unsigned                      m_SampleRate;
	uint64_t                      m_CurrentTime;
	GOSoundSamplerPool            m_SamplerPool;
	unsigned                      m_AudioGroupCount;
	unsigned                      m_WindchestCount;
	unsigned                      m_DetachedReleaseCount;
	std::vector<GOSamplerEntry>   m_DetachedRelease;
	std::vector<GOSamplerEntry>   m_Windchests;
	std::vector<GOSamplerEntry>   m_Tremulants;
	std::vector<GOOutputGroup>    m_OutputGroups;
	ptr_vector<GOSoundReverb>     m_ReverbEngine;
	std::vector<GOAudioOutput>    m_AudioOutputs;

	std::vector<unsigned>         m_WorkItems;
	std::atomic_uint              m_NextItem;

	struct resampler_coefs_s      m_ResamplerCoefs;

	/* samplerGroupID:
	   -1 .. -n Tremulants
	   0 detached release
	   1 .. n Windchests
	   n+1 .. ? additional detached release processors
	*/
	void StartSampler(GO_SAMPLER* sampler, int sampler_group_id, unsigned audio_group);
	void CreateReleaseSampler(const GO_SAMPLER* sampler);
	void SwitchAttackSampler(GO_SAMPLER* sampler);
	void ProcessAudioSamplers (GOSamplerEntry& state, unsigned int n_frames, bool depend = true);
	void ResetDoneFlags();
	unsigned GetFaderLength(unsigned MidiKeyNumber);
	float GetRandomFactor();
	void ProcessTremulants(unsigned n_frames);
	void ProcessOutputGroup(unsigned audio_group, unsigned n_frames);

public:

	GOSoundEngine();
	~GOSoundEngine();
	void Reset();
	void Setup(GrandOrgueFile* organ_file, unsigned release_count = 1);
	void SetAudioOutput(std::vector<GOAudioOutputConfiguration> audio_outputs);
	void SetupReverb(GOrgueSettings& settings);
	void SetVolume(int volume);
	void SetSampleRate(unsigned sample_rate);
	void SetInterpolationType(unsigned type);
	unsigned GetSampleRate();
	void SetAudioGroupCount(unsigned groups);
	unsigned GetAudioGroupCount();
	void SetHardPolyphony(unsigned polyphony);
	void SetPolyphonyLimiting(bool limiting);
	unsigned GetHardPolyphony() const;
	int GetVolume() const;
	void SetScaledReleases(bool enable);
	void SetRandomizeSpeaking(bool enable);
	void SetReleaseLength(unsigned reverb);

	SAMPLER_HANDLE StartSample(const GOSoundProvider *pipe, int sampler_group_id, unsigned audio_group, unsigned velocity, unsigned delay);
	void StopSample(const GOSoundProvider *pipe, SAMPLER_HANDLE handle);
	void SwitchSample(const GOSoundProvider *pipe, SAMPLER_HANDLE handle);
	void UpdateVelocity(SAMPLER_HANDLE handle, unsigned velocity);

	int GetAudioOutput(float *output_buffer, unsigned n_frames, unsigned audio_output);

	int GetSamples
		(float      *output_buffer
		,unsigned    n_frames
		,double      stream_time
		,METER_INFO *meter_info
		);
	void Process(unsigned group_id, unsigned n_frames);
	unsigned GetGroupCount();
	int GetNextGroup();
};

#endif /* GOSOUNDENGINE_H_ */
