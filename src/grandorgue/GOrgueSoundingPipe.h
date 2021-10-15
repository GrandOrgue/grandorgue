/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUESOUNDINGPIPE_H
#define GORGUESOUNDINGPIPE_H

#include "GOSoundProviderWave.h"
#include "GOrgueCacheObject.h"
#include "GOrguePipe.h"
#include "GOrguePipeConfigNode.h"
#include "GOrguePipeWindchestCallback.h"

class GO_SAMPLER;

class GOrgueSoundingPipe : public GOrguePipe, private GOrgueCacheObject, private GOrguePipeUpdateCallback, 
	private GOrguePipeWindchestCallback
{
private:
	GO_SAMPLER* m_Sampler;
	uint64_t m_LastStop;
	int m_Instances;
	bool m_Tremulant;
	std::vector<attack_load_info> m_AttackInfo;
	std::vector<release_load_info> m_ReleaseInfo;
	wxString m_Filename;

	/* states which windchest this pipe belongs to, see GOSoundEngine::StartSampler */
	int m_SamplerGroupID;
	unsigned m_AudioGroupID;
	bool m_Percussive;
	float m_TemperamentOffset;
	unsigned m_HarmonicNumber;
	unsigned m_LoopCrossfadeLength;
	unsigned m_ReleaseCrossfadeLength;
    unsigned m_ReleaseTruncationLength;
	float m_PitchCorrection;
	float m_MinVolume;
	float m_MaxVolume;
	int m_SampleMidiKeyNumber;
	bool m_RetunePipe;
	GOSoundProviderWave m_SoundProvider;
	GOrguePipeConfigNode m_PipeConfig;

	void SetOn(unsigned velocity);
	void SetOff();
	void Change(unsigned velocity, unsigned old_velocity);
	GOSoundProvider* GetSoundProvider();
	void Validate();

	void LoadAttack(GOrgueConfigReader& cfg, wxString group, wxString prefix);

	void Initialize();
	void LoadData();
	bool LoadCache(GOrgueCache& cache);
	bool SaveCache(GOrgueCacheWriter& cache);
	void UpdateHash(GOrgueHash& hash);
	const wxString& GetLoadTitle();

	void SetTremulant(bool on);

	void UpdateAmplitude();
	void UpdateReleaseTruncationLength();
	void UpdateTuning();
	void UpdateAudioGroup();

	void AbortPlayback();
	void PreparePlayback();

public:
	GOrgueSoundingPipe(GrandOrgueFile* organfile, GOrgueRank* rank, bool percussive, int sampler_group_id, unsigned midi_key_number, unsigned harmonic_number, float pitch_correction, float min_volume, float max_volume, bool retune);

	void Init(GOrgueConfigReader& cfg, wxString group, wxString prefix, wxString filename);
	void Load(GOrgueConfigReader& cfg, wxString group, wxString prefix);
	void SetTemperament(const GOrgueTemperament& temperament);
};

#endif
