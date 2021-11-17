/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOSOUNDENGINE_H_
#define GOSOUNDENGINE_H_

#include "GOSoundResample.h"
#include "scheduler/GOSoundScheduler.h"
#include "GOSoundSamplerPool.h"
#include <vector>

class GOWindchest;
class GOSoundProvider;
class GOSoundRecorder;
class GOSoundGroupWorkItem;
class GOSoundOutputWorkItem;
class GOSoundReleaseWorkItem;
class GOSoundTouchWorkItem;
class GOSoundTremulantWorkItem;
class GOSoundWindchestWorkItem;
class GOSoundWorkItem;
class GODefinitionFile;
class GOSettings;

typedef struct
{
	unsigned channels;
	std::vector< std::vector<float> > scale_factors;
} GOAudioOutputConfiguration;

class GO_SAMPLER;

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
	unsigned m_UsedPolyphony;
	unsigned                      m_WorkerSlots;
	std::vector<double> m_MeterInfo;
	ptr_vector<GOSoundTremulantWorkItem> m_Tremulants;
	ptr_vector<GOSoundWindchestWorkItem> m_Windchests;
	ptr_vector<GOSoundGroupWorkItem> m_AudioGroups;
	ptr_vector<GOSoundOutputWorkItem> m_AudioOutputs;
	GOSoundRecorder* m_AudioRecorder;
	GOSoundReleaseWorkItem* m_ReleaseProcessor;
	std::unique_ptr<GOSoundTouchWorkItem> m_TouchProcessor;

	GOSoundScheduler m_Scheduler;

	struct resampler_coefs_s      m_ResamplerCoefs;
	
	bool			      m_HasBeenSetup;

	/* samplerGroupID:
	   -1 .. -n Tremulants
	   0 detached release
	   1 .. n Windchests
	*/
	void StartSampler(GO_SAMPLER* sampler, int sampler_group_id, unsigned audio_group);
	void CreateReleaseSampler(GO_SAMPLER* sampler);
	void SwitchAttackSampler(GO_SAMPLER* sampler);
	float GetRandomFactor();

public:

	GOSoundEngine();
	~GOSoundEngine();
	void Reset();
	void Setup(GODefinitionFile* organ_file, unsigned release_count = 1);
	void ClearSetup();
	void SetAudioOutput(std::vector<GOAudioOutputConfiguration> audio_outputs);
	void SetupReverb(GOSettings& settings);
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
	const std::vector<double>& GetMeterInfo();
	void SetAudioRecorder(GOSoundRecorder* recorder, bool downmix);

	GO_SAMPLER* StartSample(const GOSoundProvider *pipe, int sampler_group_id, unsigned audio_group, unsigned velocity, unsigned delay, uint64_t last_stop);
	uint64_t StopSample(const GOSoundProvider *pipe, GO_SAMPLER* handle);
	void SwitchSample(const GOSoundProvider *pipe, GO_SAMPLER* handle);
	void UpdateVelocity(const GOSoundProvider* pipe, GO_SAMPLER* handle, unsigned velocity);

	void GetAudioOutput(float *output_buffer, unsigned n_frames, unsigned audio_output, bool last);
	void NextPeriod();
	GOSoundScheduler& GetScheduler();

	bool ProcessSampler(float *buffer, GO_SAMPLER* sampler, unsigned n_frames, float volume);
	void ProcessRelease(GO_SAMPLER* sampler);
	void PassSampler(GO_SAMPLER* sampler);
	void ReturnSampler(GO_SAMPLER* sampler);
	float GetGain();
	uint64_t GetTime() const
	{
		return m_CurrentTime;
	}
};

#endif /* GOSOUNDENGINE_H_ */
