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

#ifndef GORGUESOUND_H
#define GORGUESOUND_H


#include <wx/wx.h>
#include <wx/stopwatch.h>
#include <map>
#include <vector>
#include "RtAudio.h"
#include "GrandOrgueDef.h"

class wxConfigBase;
class GrandOrgueFile;
class GOrgueMidi;

#include "GOrgueSoundTypes.h"

class GOrgueSound
{

public:

	typedef struct
	{
		RtAudio::Api rt_api;
		int rt_api_subindex;
	} GO_SOUND_DEV_CONFIG;

private:
	typedef int sound_buffer[(MAX_FRAME_SIZE + BLOCKS_PER_FRAME) * MAX_OUTPUT_CHANNELS];
	typedef struct {
		GO_SAMPLER* sampler;
		sound_buffer buff;
	} tremulant_data;

	/* These are only used by the audio callback... */
	double final_buff[MAX_FRAME_SIZE * MAX_OUTPUT_CHANNELS];
	float volume_buff[MAX_FRAME_SIZE * MAX_OUTPUT_CHANNELS];
	sound_buffer g_buff;

	/* this buffer is used as a temprary when decoding frame data */
	int m_TempDecodeBuffer[(MAX_FRAME_SIZE + BLOCKS_PER_FRAME) * MAX_OUTPUT_CHANNELS];

	/* end audio callback variables */

	wxStopWatch sw;

	wxConfigBase *pConfig;

	GO_SAMPLER samplers[MAX_POLYPHONY];
	GO_SAMPLER* samplers_open[MAX_POLYPHONY];
	RtAudioFormat format;

	bool logSoundErrors;

	int samplers_count;
	int polyphony;
	int poly_soft;
	int volume;

	std::map<wxString, GO_SOUND_DEV_CONFIG> m_audioDevices;
	RtAudio* audioDevice;
/*	int n_latency;*/

	unsigned m_samples_per_buffer;
	unsigned m_nb_buffers;

	int b_limit, b_stereo, b_align, b_scale;
	int b_random;
	bool b_stoprecording;
	FILE *f_output;

	short meter_counter;
	short meter_poly;
	double meter_left;
	double meter_right;

	bool b_active;

	wxString defaultAudio;

	GOrgueMidi* m_midi;
	GrandOrgueFile* m_organfile;

private:

	void ProcessAudioSamplers
		(GO_SAMPLER** list_start
		,unsigned int n_frames
		,int* output_buffer
		);

	void MIDICallback
		(std::vector<unsigned char>& msg
		,int which
		);

	/* The RtAudio callback (below) will call this function, localising it to
	 * the correct GOrgueSound object. */
	int AudioCallbackLocal
		(float *output_buffer
		,unsigned int n_frames
		,double stream_time
		);

	/* This is the callback issued by RtAudio */
	static
	int AudioCallback
		(void *outputBuffer
		,void *inputBuffer
		,unsigned int nFrames
		,double streamTime
		,RtAudioStreamStatus status
		,void *userData
		);

	std::vector<GO_SAMPLER*> m_windchests;
	GO_SAMPLER* m_detachedRelease;
	std::vector<tremulant_data> m_tremulants;

public:

	GOrgueSound(void);
	~GOrgueSound(void);

	bool OpenSound(bool wait, GrandOrgueFile* organfile);
	void CloseSound();
	bool ResetSound();

	void CloseWAV();

	void SetPolyphonyLimit(int polyphony);
	void SetPolyphonySoftLimit(int polyphony_soft);
	void SetVolume(int volume);
	int GetVolume();

	GO_SAMPLER* OpenNewSampler();
	/* samplerGroupID:
	   -1 .. -n Tremulants
	   0 detached release
	   1 .. n Windchests
	*/
	void StartSampler(GO_SAMPLER* sampler, int samplerGroupID);

	bool HasRandomPipeSpeech();
	bool HasReleaseAlignment();
	bool HasScaledReleases();
	bool IsStereo();

	bool IsRecording();
	void StartRecording(wxString filename);
	void StopRecording();

	bool IsActive();
	void PreparePlayback(GrandOrgueFile* organfile);
	void ActivatePlayback();

	void SetLogSoundErrorMessages(bool settingsDialogVisible);

	const RtAudioFormat GetAudioFormat();

	/* TODO: these should have const scope */
	std::map<wxString, GO_SOUND_DEV_CONFIG>& GetAudioDevices();
	const wxString GetDefaultAudioDevice();

	const int GetLatency();

	GOrgueMidi& GetMidi();

};

#endif
