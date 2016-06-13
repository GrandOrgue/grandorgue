/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2016 GrandOrgue contributors (see AUTHORS)
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

#include "ptrvector.h"
#include "GOSoundEngine.h"
#include "GOSoundRecorder.h"
#include "GOrgueSoundDevInfo.h"
#include "atomic.h"
#include "mutex.h"
#include <wx/string.h>
#include <map>
#include <vector>

class GrandOrgueFile;
class GOrgueMidi;
class GOSoundThread;
class GOrgueSoundPort;
class GOrgueSoundRtPort;
class GOrgueSoundPortaudioPort;
class GOrgueSettings;

class GOrgueSound
{
	class GO_SOUND_OUTPUT
	{
	public:
		GOrgueSoundPort* port;
		GOMutex mutex;
		GOCondition condition;
		bool wait;
		bool waiting;

		GO_SOUND_OUTPUT() :
			condition(mutex)
		{
			port = 0;
			wait = false;
			waiting = false;
		}

		GO_SOUND_OUTPUT(const GO_SOUND_OUTPUT& old) :
			condition(mutex)
		{
			port = old.port;
			wait = old.wait;
			waiting = old.waiting;
		}

		const GO_SOUND_OUTPUT& operator=(const GO_SOUND_OUTPUT& old)
		{
			port = old.port;
			wait = old.wait;
			waiting = old.waiting;
			return *this;
		}
	};

private:
	GOMutex m_lock;
	GOMutex m_thread_lock;

	bool logSoundErrors;

	std::vector<GO_SOUND_OUTPUT> m_AudioOutputs;
	atomic_uint m_WaitCount;
	atomic_uint m_CalcCount;

	unsigned m_SamplesPerBuffer;

	unsigned meter_counter;

	wxString m_defaultAudioDevice;

	GOrgueMidi* m_midi;
	GrandOrgueFile* m_organfile;
	GOSoundRecorder m_AudioRecorder;

	GOSoundEngine m_SoundEngine;
	ptr_vector <GOSoundThread> m_Threads;

	GOrgueSettings& m_Settings;

	void StopThreads();
	void StartThreads();

	void ResetMeters();

	void OpenMidi();
	void StartStreams();
	void UpdateMeter();

public:

	GOrgueSound(GOrgueSettings& settings);
	~GOrgueSound();

	bool OpenSound();
	void CloseSound();
	bool ResetSound(bool force = false);

	wxString getState();

	GOrgueSettings& GetSettings();

	bool IsMidiPlaying();
	void StartMidiPlaying(wxString filename);
	void StopMidiPlaying();

	void AssignOrganFile(GrandOrgueFile* organfile);
	GrandOrgueFile* GetOrganFile();

	void SetLogSoundErrorMessages(bool settingsDialogVisible);

	std::vector<GOrgueSoundDevInfo> GetAudioDevices();
	const wxString GetDefaultAudioDevice();

	GOrgueMidi& GetMidi();

	GOSoundEngine& GetEngine();

	bool AudioCallback(unsigned dev_index, float* outputBuffer, unsigned int nFrames);
};

#endif
