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

struct GOrgueSamplerT
{
	GOrgueSamplerT* next;		// must be first!
	GOrguePipe* pipe;
	wxByte* ptr;
	int fade, fadein, fadeout, faderemain, fademax, time, offset, type;
	int current, stage, overflowing, shift;
	wxInt64 overflow, f, v;
};

typedef struct GOrgueSamplerT GOrgueSampler;

class GOrgueSound
{
public:
	GOrgueSound(void);
	~GOrgueSound(void);

	bool OpenSound(bool wait = true);
	void CloseSound();
	bool ResetSound();

	void OpenWAV();
	void CloseWAV();

    void UpdateOrganMIDI();
    std::map<long, wxString> organmidiEvents;

	static void MIDIPretend(bool on);

	wxStopWatch sw;

	wxConfigBase *pConfig;

	GOrgueSampler samplers[MAX_POLYPHONY];
	GOrgueSampler* samplers_open[MAX_POLYPHONY];
	GOrgueSampler* windchests[10 + 1 + 16];		// maximum 10 tremulants + 1 detach + 16 windchests
	RtAudioFormat format;
	wxWindow* m_parent;
	int samplers_count;
	int polyphony, poly_soft;
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
	int i_midiEvents[16];

	int b_limit, b_stereo, b_align, b_detach, b_scale, b_random;
	bool b_stoprecording, b_memset;
	FILE *f_output;

	short meter_counter;
	short meter_poly;
	double meter_left;
	double meter_right;

	bool b_active, b_listening;
	wxEvtHandler* listen_evthandler;

	wxString defaultAudio;
};

#pragma pack(1)

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

#pragma pack()

#endif
