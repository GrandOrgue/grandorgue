/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2015 GrandOrgue contributors (see AUTHORS)
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

class GOrgueWindchest;
class GOSoundProvider;
class GOSoundReverb;
class GOSoundGroupWorkItem;
class GOSoundOutputWorkItem;
class GOSoundTremulantWorkItem;
class GOSoundWindchestWorkItem;
class GOSoundWorkItem;
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

	unsigned                      m_PolyphonySoftLimit;
	bool                          m_PolyphonyLimiting;
	bool                          m_ScaledReleases;
	bool                          m_ReleaseAlignmentEnabled;
	bool                          m_RandomizeSpeaking;
	int                           m_Volume;
	unsigned                      m_ReleaseLength;
	unsigned m_SamplesPerBuffer;
	float                         m_Gain;
	unsigned                      m_SampleRate;
	uint64_t                      m_CurrentTime;
	GOSoundSamplerPool            m_SamplerPool;
	unsigned                      m_AudioGroupCount;
	unsigned                      m_WorkerSlots;
	unsigned                      m_DetachedReleaseCount;
	ptr_vector<GOSoundTremulantWorkItem> m_Tremulants;
	ptr_vector<GOSoundWindchestWorkItem> m_Windchests;
	ptr_vector<GOSoundGroupWorkItem> m_AudioGroups;
	ptr_vector<GOSoundOutputWorkItem> m_AudioOutputs;
	ptr_vector<GOSoundReverb>     m_ReverbEngine;

	std::vector<GOSoundWorkItem*>  m_WorkItems;
	std::atomic_uint              m_NextItem;

	struct resampler_coefs_s      m_ResamplerCoefs;

	/* samplerGroupID:
	   -1 .. -n Tremulants
	   0 detached release
	   1 .. n Windchests
	*/
	void StartSampler(GO_SAMPLER* sampler, int sampler_group_id, unsigned audio_group);
	void CreateReleaseSampler(GO_SAMPLER* sampler);
	void SwitchAttackSampler(GO_SAMPLER* sampler);
	void ResetDoneFlags();
	unsigned GetFaderLength(unsigned MidiKeyNumber);
	float GetRandomFactor();
	void ProcessTremulants();

public:

	GOSoundEngine();
	~GOSoundEngine();
	void Reset();
	void Setup(GrandOrgueFile* organ_file, unsigned release_count = 1);
	void ClearSetup();
	void SetAudioOutput(std::vector<GOAudioOutputConfiguration> audio_outputs);
	void SetupReverb(GOrgueSettings& settings);
	void SetVolume(int volume);
	void SetSampleRate(unsigned sample_rate);
	void SetSamplesPerBuffer(unsigned sample_per_buffer);
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

	void GetAudioOutput(float *output_buffer, unsigned n_frames, unsigned audio_output);

	void GetSamples(float *output_buffer, unsigned n_frames, METER_INFO *meter_info);
	GOSoundWorkItem* GetNextGroup();

	bool ProcessSampler(float buffer[GO_SOUND_BUFFER_SIZE], GO_SAMPLER* sampler, unsigned n_frames, float volume);
	void ReturnSampler(GO_SAMPLER* sampler);
	float GetGain();
};

#endif /* GOSOUNDENGINE_H_ */
