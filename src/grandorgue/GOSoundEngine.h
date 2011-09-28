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
		GO_SAMPLER       *new_sampler;
		sound_buffer      buff;
		sound_buffer      temp;
		/* access lock for sampler data */
		wxCriticalSection lock;
		/* access lock for data buffer */
		wxCriticalSection mutex;
		GOrgueWindchest  *windchest;
		bool              done;
		unsigned          count;

		GOSamplerEntry()
		{
			sampler = NULL;
			new_sampler = NULL;
			windchest = NULL;
			done = false;
			count = 0;
		}

		GOSamplerEntry(const GOSamplerEntry& entry)
		{
			sampler = entry.sampler;
			new_sampler = entry.new_sampler;
			windchest = entry.windchest;
			done = false;
			count = entry.count;
		}

		const GOSamplerEntry& operator=(const GOSamplerEntry& entry)
		{
			sampler = entry.sampler;
			new_sampler = entry.new_sampler;
			windchest = entry.windchest;
			count = entry.count;
			done = false;
			return *this;
		}
	};

	unsigned                      m_PolyphonySoftLimit;
	bool                          m_PolyphonyLimiting;
	bool                          m_ScaledReleases;
	bool                          m_ReleaseAlignmentEnabled;
	int                           m_Volume;
	unsigned                      m_SampleRate;
	unsigned long                 m_CurrentTime;
	GOSoundSamplerPool            m_SamplerPool;
	std::vector<GOSamplerEntry>   m_DetachedRelease;
	std::vector<GOSamplerEntry>   m_Windchests;
	std::vector<GOSamplerEntry>   m_Tremulants;

	/* Per sampler decode buffers */
	double                        m_FinalBuffer[GO_SOUND_BUFFER_SIZE];
	float                         m_VolumeBuffer[GO_SOUND_BUFFER_SIZE];

	/* samplerGroupID:
	   -1 .. -n Tremulants
	   0 detached release
	   1 .. n Windchests
	   n+1 .. ? additional detached release processors
	*/
	void StartSampler(GO_SAMPLER* sampler, int sampler_group_id);
	void CreateReleaseSampler(const GO_SAMPLER* sampler);
	void ReadSamplerFrames(GO_SAMPLER* sampler, unsigned int n_blocks, int* decoded_sampler_audio_frame);
	void ProcessAudioSamplers (GOSamplerEntry& state, unsigned int n_frames, int* output_buffer);
	void ResetDoneFlags();

public:

	GOSoundEngine();
	void Reset();
	void Setup(GrandOrgueFile* organ_file, unsigned release_count = 1);
	void SetVolume(int volume);
	void SetSampleRate(unsigned sample_rate);
	unsigned GetSampleRate();
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
	void Process(unsigned sampler_group_id, unsigned n_frames);
};

#endif /* GOSOUNDENGINE_H_ */
