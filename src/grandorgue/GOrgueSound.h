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

#ifndef GORGUESOUND_H
#define GORGUESOUND_H

#include "portaudio.h"
#include "RtAudio.h"
#include "ptrvector.h"
#include "GOSoundEngine.h"
#include "GOSoundRecorder.h"
#include "GOLock.h"
#include <wx/string.h>
#include <wx/stopwatch.h>
#include <map>
#include <vector>

class GrandOrgueFile;
class GOrgueMidi;
class GOSoundThread;
class GOrgueSettings;

#define RTAPI_PORTAUDIO ((RtAudio::Api)(RtAudio::RTAUDIO_DUMMY + 1))

class GOrgueSound
{
	class GO_SOUND_OUTPUT
	{
	public:
		GOrgueSound* sound;
		unsigned index;

		wxString name;

		RtAudio::Api rt_api;
		int rt_api_subindex;
		unsigned channels;

		unsigned try_latency;
		unsigned nb_buffers;

		PaStream* audioStream;
		RtAudio* audioDevice;

		GOMutex mutex;
		GOCondition condition;
		bool wait;
		bool waiting;

		GO_SOUND_OUTPUT() :
			condition(mutex)
		{
			sound = 0;
			index = 0;
			name = wxEmptyString;
			rt_api = RtAudio::RTAUDIO_DUMMY;
			rt_api_subindex = 0;
			channels = 0;
			try_latency = 0;
			nb_buffers = 0;
			audioStream = 0;
			audioDevice = 0;
			wait = false;
			waiting = false;
		}

		GO_SOUND_OUTPUT(const GO_SOUND_OUTPUT& old) :
			condition(mutex)
		{
			sound = old.sound;
			index = old.index;
			name = old.name;
			rt_api = old.rt_api;
			rt_api_subindex = old.rt_api_subindex;
			channels = old.channels;
			try_latency = old.try_latency;
			nb_buffers = old.nb_buffers;
			audioStream = old.audioStream;
			audioDevice = old.audioDevice;
			wait = old.wait;
			waiting = old.waiting;
		}

		const GO_SOUND_OUTPUT& operator=(const GO_SOUND_OUTPUT& old)
		{
			sound = old.sound;
			index = old.index;
			name = old.name;
			rt_api = old.rt_api;
			rt_api_subindex = old.rt_api_subindex;
			channels = old.channels;
			try_latency = old.try_latency;
			nb_buffers = old.nb_buffers;
			audioStream = old.audioStream;
			audioDevice = old.audioDevice;
			wait = old.wait;
			waiting = old.waiting;
			return *this;
		}
	};
public:

	typedef struct
	{
		RtAudio::Api rt_api;
		int rt_api_subindex;
		unsigned channels;
	} GO_SOUND_DEV_CONFIG;

private:
	GOMutex m_lock;
	GOMutex m_thread_lock;

	bool logSoundErrors;

	std::map<wxString, GO_SOUND_DEV_CONFIG> m_audioDevices;
	std::vector<GO_SOUND_OUTPUT> m_AudioOutputs;

	unsigned m_SamplesPerBuffer;

	short meter_counter;
	METER_INFO meter_info;

	wxString defaultAudioDevice;

	GOrgueMidi* m_midi;
	GrandOrgueFile* m_organfile;
	GOSoundRecorder m_recorder;

	GOSoundEngine m_SoundEngine;
	ptr_vector <GOSoundThread> m_Threads;

	GOrgueSettings& m_Settings;

	int AudioCallbackLocal(GO_SOUND_OUTPUT* device, float* outputBuffer, unsigned int nFrames, double streamTime);

	/* This is the callback issued by RtAudio */
	static int AudioCallback(void *outputBuffer, void *inputBuffer, unsigned int nFrames, double streamTime, RtAudioStreamStatus status, void *userData);

	static int PaAudioCallback (const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData);

	void StopThreads();
	void StartThreads();

	void ResetMeters();

	void OpenMidi();
	void InitStreams();
	void OpenStreams();
	void StartStreams();

public:

	GOrgueSound(GOrgueSettings& settings);
	~GOrgueSound();

	bool OpenSound();
	void CloseSound();
	bool ResetSound(bool force = false);

	GOrgueSettings& GetSettings();

	bool IsRecording();
	void StartRecording(wxString filename);
	void StopRecording();

	void PreparePlayback(GrandOrgueFile* organfile);
	GrandOrgueFile* GetOrganFile();

	void SetLogSoundErrorMessages(bool settingsDialogVisible);

	const std::map<wxString, GO_SOUND_DEV_CONFIG>& GetAudioDevices();
	const wxString GetDefaultAudioDevice();

	GOrgueMidi& GetMidi();

	GOSoundEngine& GetEngine();

};

#endif
