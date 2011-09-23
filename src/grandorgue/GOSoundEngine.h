/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef GOSOUNDENGINE_H_
#define GOSOUNDENGINE_H_

#include "GOrgueSoundTypes.h"
#include "GOSoundSamplerPool.h"
#include "GrandOrgueDef.h"
#include <vector>

#define GO_SOUND_BUFFER_SIZE ((MAX_FRAME_SIZE) * (MAX_OUTPUT_CHANNELS))

class GOrgueWindchest;
class GOSoundProvider;
class GrandOrgueFile;

typedef struct
{
	unsigned current_polyphony;
	double meter_left;
	double meter_right;
} METER_INFO;

typedef GO_SAMPLER* SAMPLER_HANDLE;

class GOSoundEngine
{

private:

	typedef int sound_buffer[GO_SOUND_BUFFER_SIZE];

	/* This is inteded to be struct, but needs copy constructors to make wxCriticalSection work with std::vector */
	class GOSamplerEntry
	{
	public:
		GO_SAMPLER       *sampler;
		sound_buffer      buff;
		wxCriticalSection lock;
		GOrgueWindchest  *windchest;

		GOSamplerEntry()
		{
			sampler = NULL;
			windchest = NULL;
		}

		GOSamplerEntry(const GOSamplerEntry& entry)
		{
			sampler = entry.sampler;
			windchest = entry.windchest;
		}

		const GOSamplerEntry& operator=(const GOSamplerEntry& entry)
		{
			sampler = entry.sampler;
			windchest = entry.windchest;
			return *this;
		}
	};

	GOSamplerEntry                m_DetachedRelease;
	unsigned                      m_PolyphonySoftLimit;
	bool                          m_PolyphonyLimiting;
	bool                          m_ScaledReleases;
	bool                          m_ReleaseAlignmentEnabled;
	int                           m_Volume;
	unsigned long                 m_CurrentTime;
	GOSoundSamplerPool            m_SamplerPool;
	std::vector<GOSamplerEntry>   m_Windchests;
	std::vector<GOSamplerEntry>   m_Tremulants;
	/* Per sampler decode buffers */
	sound_buffer                  m_TempDecodeBuffer;
	double                        m_FinalBuffer[GO_SOUND_BUFFER_SIZE];
	float                         m_VolumeBuffer[GO_SOUND_BUFFER_SIZE];
	/* Per sampler list decode buffer */
	sound_buffer                  m_TempSoundBuffer;

	/* samplerGroupID:
	   -1 .. -n Tremulants
	   0 detached release
	   1 .. n Windchests
	*/
	void StartSampler(GO_SAMPLER* sampler, int sampler_group_id);
	void StartSamplerUnlocked(GO_SAMPLER* sampler, int sampler_group_id);
	void CreateReleaseSampler(const GO_SAMPLER* sampler);
	void ProcessAudioSamplers (GOSamplerEntry& state, unsigned int n_frames, int* output_buffer);

public:

	GOSoundEngine();
	void Reset();
	void Setup(GrandOrgueFile* organ_file);
	void SetVolume(int volume);
	void SetHardPolyphony(unsigned polyphony);
	void SetPolyphonyLimiting(bool limiting);
	unsigned GetHardPolyphony() const;
	int GetVolume() const;
	void SetScaledReleases(bool enable);

	SAMPLER_HANDLE StartSample(const GOSoundProvider *pipe, int sampler_group_id);
	void StopSample(const GOSoundProvider *pipe, SAMPLER_HANDLE handle);

	int GetSamples
		(float      *output_buffer
		,unsigned    n_frames
		,double      stream_time
		,METER_INFO *meter_info
		);

};

#endif /* GOSOUNDENGINE_H_ */
