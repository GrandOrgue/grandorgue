/*
 * GOSoundEngine.h
 *
 *  Created on: Sep 17, 2011
 *      Author: nick
 */

#ifndef GOSOUNDENGINE_H_
#define GOSOUNDENGINE_H_

#include "GOrgueSoundTypes.h"
#include "GrandOrgueDef.h"
#include <vector>
#include <string.h>

class GOrgueWindchest;
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

	typedef int sound_buffer[(MAX_FRAME_SIZE + BLOCKS_PER_FRAME) * MAX_OUTPUT_CHANNELS];

	typedef struct
	{
		GO_SAMPLER      *base_sampler;
		GOrgueWindchest *windchest;
	} windchest_entry;

	typedef struct
	{
		GO_SAMPLER   *sampler;
		sound_buffer  buff;
	} tremulant_data;

	GO_SAMPLER    *m_DetachedRelease;
	unsigned       m_SamplerCount;
	unsigned       m_PolyphonyLimit;
	unsigned       m_PolyphonySoftLimit;
	bool           m_PolyphonyLimiting;
	bool           m_ScaledReleases;
	bool           m_ReleaseAlignmentEnabled;
	int            m_Volume;
	unsigned long  m_CurrentTime;
	std::vector<windchest_entry> m_Windchests;
	std::vector<tremulant_data>  m_Tremulants;

	void ProcessAudioSamplers
		(GO_SAMPLER** list_start
		,unsigned int n_frames
		,int* output_buffer
		);

	void MIDICallback
		(std::vector<unsigned char>& msg
		,int which
		);

	/* These are only used by the audio callback... */
	GO_SAMPLER    *m_AvailableSamplers[MAX_POLYPHONY];

	/* Per sampler decode buffer */
	int            m_TempDecodeBuffer[(MAX_FRAME_SIZE + BLOCKS_PER_FRAME) * MAX_OUTPUT_CHANNELS];
	double         m_FinalBuffer[MAX_FRAME_SIZE * MAX_OUTPUT_CHANNELS];
	float          m_VolumeBuffer[MAX_FRAME_SIZE * MAX_OUTPUT_CHANNELS];

	/* Per sampler list decode buffer */
	sound_buffer   m_TempSoundBuffer;

	GO_SAMPLER     m_Samplers[MAX_POLYPHONY];

	GO_SAMPLER* OpenNewSampler();

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

	void StartSampler(GO_SAMPLER* sampler, int samplerGroupId);
	unsigned GetCurrentTime() const;
	SAMPLER_HANDLE StartSample(const GOrguePipe *pipe);
	void StopSample(const GOrguePipe *pipe, SAMPLER_HANDLE handle);

	int GetSamples
		(float      *output_buffer
		,unsigned    n_frames
		,double      stream_time
		,METER_INFO *meter_info
		);

};

#endif /* GOSOUNDENGINE_H_ */
