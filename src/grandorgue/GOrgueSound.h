/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUESOUND_H
#define GORGUESOUND_H

#include "ptrvector.h"
#include "GOSoundEngine.h"
#include "GOSoundRecorder.h"
#include "GOrgueMidi.h"
#include "GOrgueSoundDevInfo.h"
#include "GOrgueSoundPortsConfig.h"

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

	GrandOrgueFile* m_organfile;
	GOSoundRecorder m_AudioRecorder;

	GOSoundEngine m_SoundEngine;
	ptr_vector <GOSoundThread> m_Threads;

	GOrgueSettings& m_Settings;

	GOrgueMidi m_midi;
	
	wxString m_LastErrorMessage;
	
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

	wxString getLastErrorMessage() const { return m_LastErrorMessage; }
	wxString getState();

	GOrgueSettings& GetSettings();

	void AssignOrganFile(GrandOrgueFile* organfile);
	GrandOrgueFile* GetOrganFile();

	void SetLogSoundErrorMessages(bool settingsDialogVisible);

	std::vector<GOrgueSoundDevInfo> GetAudioDevices(const GOrgueSoundPortsConfig &portsConfig);
	const wxString GetDefaultAudioDevice(const GOrgueSoundPortsConfig &portsConfig);

	GOrgueMidi& GetMidi();

	GOSoundEngine& GetEngine();

	bool AudioCallback(unsigned dev_index, float* outputBuffer, unsigned int nFrames);
};

#endif
