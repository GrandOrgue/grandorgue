/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2013 GrandOrgue contributors (see AUTHORS)
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

#ifndef GORGUEPIPE_H
#define GORGUEPIPE_H

#include <wx/wx.h>
#include "GOrgueCacheObject.h"
#include "GOrguePipeConfig.h"
#include "GOSoundProviderWave.h"

class GOrgueCache;
class GOrgueCacheWriter;
class GOrgueConfigReader;
class GOrgueConfigWriter;
class GOrgueRank;
class GOrgueTemperament;
class GrandOrgueFile;
typedef struct GO_SAMPLER_T* SAMPLER_HANDLE;

class GOrguePipe : public GOrguePipeUpdateCallback, public GOrgueCacheObject
{

private:
	GrandOrgueFile* m_OrganFile;
	GOrgueRank* m_Rank;
	SAMPLER_HANDLE  m_Sampler;
	int m_Instances;
	bool m_Tremulant;
	std::vector<attack_load_info> m_AttackInfo;
	std::vector<release_load_info> m_ReleaseInfo;

	/* states which windchest this pipe belongs to, see GOSoundEngine::StartSampler */
	int m_SamplerGroupID;
	unsigned m_AudioGroupID;
	wxString m_Filename;
	bool m_Percussive;
	unsigned m_MidiKeyNumber;
	float m_TemperamentOffset;
	unsigned m_HarmonicNumber;
	float m_PitchCorrection;
	float m_MinVolume;
	float m_MaxVolume;
	int m_SampleMidiKeyNumber;
	GOrguePipe* m_Reference;
	unsigned m_ReferenceID;
	GOSoundProviderWave m_SoundProvider;
	GOrguePipeConfig m_PipeConfig;

	unsigned m_Velocity;
	std::vector<unsigned> m_Velocities;

	void SetOn();
	void SetOff();
	GOSoundProvider* GetSoundProvider();
	void LoadAttack(GOrgueConfigReader& cfg, wxString group, wxString prefix);

public:
	GOrguePipe(GrandOrgueFile* organfile, GOrgueRank* m_Rank, bool percussive, int sampler_group_id, unsigned midi_key_number, unsigned harmonic_number, float pitch_correction, float min_volume, float max_volume);
	~GOrguePipe();
	void Load(GOrgueConfigReader& cfg, wxString group, wxString prefix);
	void Save(GOrgueConfigWriter& cfg);
	void Set(unsigned velocity, unsigned referenceID = 0);
	void SetTremulant(bool on);
	bool InitializeReference();
	unsigned RegisterReference(GOrguePipe* pipe);
	void LoadData();
	bool LoadCache(GOrgueCache& cache);
	bool SaveCache(GOrgueCacheWriter& cache);
	void Initialize();
	void UpdateHash(SHA_CTX& ctx);
	void Abort();
	void PreparePlayback();
	wxString GetFilename();
	wxString GetLoadTitle();
	bool IsReference();
	GOrguePipeConfig& GetPipeConfig();
	void SetTemperament(const GOrgueTemperament& temperament);

	float GetEffectiveAmplitude();
	float GetEffectiveGain();
	void UpdateAmplitude();

	float GetEffectiveTuning();
	void UpdateTuning();

	wxString GetEffectiveAudioGroup();
	void UpdateAudioGroup();

	unsigned GetEffectiveBitsPerSample();
	bool GetEffectiveCompress();
	unsigned GetEffectiveLoopLoad();
	unsigned GetEffectiveAttackLoad();
	unsigned GetEffectiveReleaseLoad();
	unsigned GetEffectiveChannels();
};

#endif
