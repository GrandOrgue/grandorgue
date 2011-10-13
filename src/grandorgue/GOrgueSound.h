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
#include "ptrvector.h"
#include "RtAudio.h"
#include "GrandOrgueDef.h"
#include "GOSoundEngine.h"
#include "GOSoundRecorder.h"

class wxConfigBase;
class GrandOrgueFile;
class GOrgueMidi;
class GOSoundThread;

class GOrgueSound
{

public:

	typedef struct
	{
		RtAudio::Api rt_api;
		int rt_api_subindex;
	} GO_SOUND_DEV_CONFIG;

private:
	wxCriticalSection m_lock;

	/* end audio callback variables */
	wxConfigBase *pConfig;

	RtAudioFormat format;

	bool logSoundErrors;

	std::map<wxString, GO_SOUND_DEV_CONFIG> m_audioDevices;
	RtAudio* audioDevice;
/*	int n_latency;*/

	unsigned m_SamplesPerBuffer;
	unsigned m_nb_buffers;

	int b_stereo, b_align;
	int b_random;
	unsigned m_Concurrency;
	unsigned m_ReleaseConcurrency;

	short meter_counter;
	METER_INFO meter_info;

	bool b_active;

	wxString defaultAudio;

	GOrgueMidi* m_midi;
	GrandOrgueFile* m_organfile;
	GOSoundRecorder m_recorder;

	GOSoundEngine m_SoundEngine;
	ptr_vector <GOSoundThread> m_Threads;

	int AudioCallbackLocal
		(float* outputBuffer
		,unsigned int nFrames
		,double streamTime
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

	void StopThreads();
	void StartThreads(unsigned windchests);

public:

	GOrgueSound(void);
	~GOrgueSound(void);

	bool OpenSound();
	void CloseSound();
	bool ResetSound();

	bool HasRandomPipeSpeech();
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

	GOSoundEngine& GetEngine();

};

#endif
