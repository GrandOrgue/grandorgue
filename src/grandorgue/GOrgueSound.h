/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2011 GrandOrgue contributors (see AUTHORS)
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

class GrandOrgueFile;
class GOrgueMidi;
class GOSoundThread;
class GOrgueSettings;

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
	wxCriticalSection m_thread_lock;

	RtAudioFormat format;

	bool logSoundErrors;

	std::map<wxString, GO_SOUND_DEV_CONFIG> m_audioDevices;
	RtAudio* audioDevice;

	unsigned m_SamplesPerBuffer;
	unsigned m_nb_buffers;

	short meter_counter;
	METER_INFO meter_info;

	wxString defaultAudio;

	GOrgueMidi* m_midi;
	GrandOrgueFile* m_organfile;
	GOSoundRecorder m_recorder;

	GOSoundEngine m_SoundEngine;
	ptr_vector <GOSoundThread> m_Threads;

	GOrgueSettings& m_Settings;

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
		) STACK_REALIGN;

	void StopThreads();
	void StartThreads(unsigned windchests);

	void ResetMeters();

public:

	GOrgueSound(GOrgueSettings& settings);
	~GOrgueSound();

	bool OpenSound();
	void CloseSound();
	bool ResetSound();

	GOrgueSettings& GetSettings();

	bool IsRecording();
	void StartRecording(wxString filename);
	void StopRecording();

	void PreparePlayback(GrandOrgueFile* organfile);

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
