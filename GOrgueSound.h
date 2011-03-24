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
#include "RtAudio.h"
#include "OrganFile.h"
#include "RtMidi.h"
#include "GrandOrgueDef.h"

BEGIN_DECLARE_EVENT_TYPES()
	DECLARE_EVENT_TYPE(wxEVT_DRAWSTOP, -1)
	DECLARE_EVENT_TYPE(wxEVT_PUSHBUTTON, -1)
	DECLARE_EVENT_TYPE(wxEVT_ENCLOSURE, -1)
	DECLARE_EVENT_TYPE(wxEVT_NOTEONOFF, -1)
	DECLARE_EVENT_TYPE(wxEVT_LISTENING, -1)
	DECLARE_EVENT_TYPE(wxEVT_METERS, -1)
	DECLARE_EVENT_TYPE(wxEVT_LOADFILE, -1)
END_DECLARE_EVENT_TYPES()

/* TODO: I think this should be imported using a #include ... there is some ambiguity */
class wxConfigBase;

class GOrguePipe;

#define DATA_TYPE_MONO_COMPRESSED     0
#define DATA_TYPE_MONO_UNCOMPRESSED   1
#define DATA_TYPE_STEREO_COMPRESSED   2
#define DATA_TYPE_STEREO_UNCOMPRESSED 3

#pragma pack(push, 1)

struct struct_WAVE
{
	char ChunkID[4];
	int ChunkSize;
	char Format[4];
    char Subchunk1ID[4];
	int Subchunk1Size;
	short AudioFormat;
	short NumChannels;
	int SampleRate;
	int ByteRate;
	short BlockAlign;
	short BitsPerSample;
	char Subchunk2ID[4];
	int Subchunk2Size;
};

#pragma pack(pop)

struct GOrgueSamplerT
{
	GOrgueSamplerT* next;		// must be first!
	GOrguePipe* pipe;
	wxByte* ptr;
	int fade, fadein, fadeout, faderemain, fademax, time, offset;
	int type; /* DATA_TYPE_xxxxx */
	int current, stage, overflowing, shift;
	wxInt64 overflow, f, v;
};

typedef struct GOrgueSamplerT GOrgueSampler;

class GOrgueSound
{

private:

	/* These are only used by the audio callback... */
	double final_buff[1024 * 2];
	float volume_buff[1024 * 2];
	int g_buff[11][(1024 + 2) * 2];
	/* end audio callback variables */

	wxStopWatch sw;

	wxConfigBase *pConfig;

	GOrgueSampler samplers[MAX_POLYPHONY];
	GOrgueSampler* samplers_open[MAX_POLYPHONY];
	RtAudioFormat format;
	/* wxWindow* m_parent; - this was pointless  */

	bool logSoundErrors;

	int samplers_count;
	int polyphony;
	int poly_soft;
	int volume;
	int transpose;

	std::map<wxString, std::pair<int, RtAudio::Api> > m_audioDevices;
	RtAudio* audioDevice;
	int n_latency;

	std::map<wxString, int> m_midiDevices;
	RtMidiIn** midiDevices;
	bool* b_midiDevices;
	int* i_midiDevices;
	int n_midiDevices;

	int b_limit, b_stereo, b_align, b_scale;
	int b_random;
	bool b_stoprecording;
	FILE *f_output;

	short meter_counter;
	short meter_poly;
	double meter_left;
	double meter_right;

	bool b_active;

	/* related to the midi listener */
	/* TODO: find out if b_listening is redundant */
	bool b_listening;
	wxEvtHandler* listen_evthandler;

	wxString defaultAudio;

	void MIDIAllNotesOff();

private:

	static int AudioCallback(void *outputBuffer, void *inputBuffer, unsigned int nFrames, double streamTime, RtAudioStreamStatus status, void *userData);
	static void MIDICallback(std::vector<unsigned char>& msg, int which, GOrgueSound* gOrgueSoundInstance);

public:

	bool b_memset;
	int i_midiEvents[16];
	GOrgueSampler* windchests[10 + 1 + 16];		// maximum 10 tremulants + 1 detach + 16 windchests
	int b_detach;

	GOrgueSound(void);
	~GOrgueSound(void);

	bool OpenSound(bool wait = true);
	void CloseSound();
	bool ResetSound();

	void CloseWAV();

    void UpdateOrganMIDI();
    std::map<long, wxString> organmidiEvents;

	static void MIDIPretend(bool on);

	void SetPolyphonyLimit(int polyphony);
	void SetPolyphonySoftLimit(int polyphony_soft);
	void SetVolume(int volume);
	int GetVolume();
	void SetTranspose(int transpose);
	GOrgueSampler* OpenNewSampler();
	bool HasRandomPipeSpeech();
	bool HasReleaseAlignment();
	bool HasScaledReleases();
	bool IsStereo();

	bool IsRecording();
	void StartRecording();
	void StopRecording();

	bool HasMIDIDevice();
	bool HasMIDIListener();
	void SetMIDIListener(wxEvtHandler* handler);

	bool IsActive();
	void ActivatePlayback();

	void SetLogSoundErrorMessages(bool settingsDialogVisible);

	const RtAudioFormat GetAudioFormat();

	/* TODO: these should have const scope */
	std::map<wxString, std::pair<int, RtAudio::Api> >& GetAudioDevices();
	std::map<wxString, int>& GetMIDIDevices();
	const wxString GetDefaultAudioDevice();


};

#endif
