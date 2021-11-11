/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOSOUND_H
#define GOSOUND_H

#include "ptrvector.h"
#include "GOSoundEngine.h"
#include "GOSoundRecorder.h"
#include "midi/GOMidi.h"
#include "GOSoundDevInfo.h"
#include "ports/GOSoundPortsConfig.h"

#include "threading/atomic.h"
#include "threading/GOMutex.h"
#include "threading/GOCondition.h"
#include <wx/string.h>
#include <map>
#include <vector>

class GODefinitionFile;
class GOMidi;
class GOSoundThread;
class GOSoundPort;
class GOSoundRtPort;
class GOSoundPortaudioPort;
class GOSettings;

class GOSound
{
	class GO_SOUND_OUTPUT
	{
	public:
		GOSoundPort* port;
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

	GODefinitionFile* m_organfile;
	GOSoundRecorder m_AudioRecorder;

	GOSoundEngine m_SoundEngine;
	ptr_vector <GOSoundThread> m_Threads;

	GOSettings& m_Settings;

	GOMidi m_midi;
	
	wxString m_LastErrorMessage;
	
	void StopThreads();
	void StartThreads();

	void ResetMeters();

	void OpenMidi();
	void StartStreams();
	void UpdateMeter();

public:

	GOSound(GOSettings& settings);
	~GOSound();

	bool OpenSound();
	void CloseSound();
	bool ResetSound(bool force = false);

	wxString getLastErrorMessage() const { return m_LastErrorMessage; }
	wxString getState();

	GOSettings& GetSettings();

	void AssignOrganFile(GODefinitionFile* organfile);
	GODefinitionFile* GetOrganFile();

	void SetLogSoundErrorMessages(bool settingsDialogVisible);

	std::vector<GOSoundDevInfo> GetAudioDevices(const GOSoundPortsConfig &portsConfig);
	const wxString GetDefaultAudioDevice(const GOSoundPortsConfig &portsConfig);

	GOMidi& GetMidi();

	GOSoundEngine& GetEngine();

	bool AudioCallback(unsigned dev_index, float* outputBuffer, unsigned int nFrames);
};

#endif
