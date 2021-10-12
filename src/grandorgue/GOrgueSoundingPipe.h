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
	unsigned m_HarmonicNumber; // Harmonic Number
	unsigned m_LoopCrossfadeLength; // Loop Crossfade Length
	unsigned m_ReleaseCrossfadeLength; // Release Crossfade Length
    unsigned m_ReleaseTruncationLength; // 11-20-20 — New Pipe Release Truncation Mechanism
	float m_PitchCorrection; // Pitch Correction
	float m_MinVolume; // Minimum Volume
	float m_MaxVolume; // Maximum Volume
	int m_SampleMidiKeyNumber;
	// Pipe Tuning / Retuning Parameter
	bool m_RetunePipe; // Retune Pipe
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
	void UpdateReleaseTruncationLength(); // Release Truncation Parameter added on 12-9-20
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
