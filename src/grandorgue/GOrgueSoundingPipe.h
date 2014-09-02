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

#ifndef GORGUESOUNDINGPIPE_H
#define GORGUESOUNDINGPIPE_H

#include "GOSoundProviderWave.h"
#include "GOrgueCacheObject.h"
#include "GOrguePipe.h"
#include "GOrguePipeConfigNode.h"
#include "GOrguePipeWindchestCallback.h"

typedef struct GO_SAMPLER_T* SAMPLER_HANDLE;

class GOrgueSoundingPipe : public GOrguePipe, private GOrgueCacheObject, private GOrguePipeUpdateCallback, 
	private GOrguePipeWindchestCallback
{
private:
	SAMPLER_HANDLE  m_Sampler;
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
	unsigned m_CrossfadeLength;
	float m_PitchCorrection;
	float m_MinVolume;
	float m_MaxVolume;
	int m_SampleMidiKeyNumber;
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
	void UpdateHash(SHA_CTX& ctx);
	const wxString& GetLoadTitle();

	void SetTremulant(bool on);

	void UpdateAmplitude();
	void UpdateTuning();
	void UpdateAudioGroup();

public:
	GOrgueSoundingPipe(GrandOrgueFile* organfile, GOrgueRank* rank, bool percussive, int sampler_group_id, unsigned midi_key_number, unsigned harmonic_number, float pitch_correction, float min_volume, float max_volume);

	void Load(GOrgueConfigReader& cfg, wxString group, wxString prefix);
	void PreparePlayback();
	void Abort();
	void SetTemperament(const GOrgueTemperament& temperament);
};

#endif
