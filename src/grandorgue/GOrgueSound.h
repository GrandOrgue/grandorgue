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

/* BLOCKS_PER_FRAME specifies the number of mono samples or stereo sample
 * pairs which are decoded for each iteration of the audio engines main loop.
 * Setting this value too low will result in inefficiencies or certain
 * features (compression) failing to work. */
#define BLOCKS_PER_FRAME 64

/* Maximum number of blocks (1 block is nChannels samples) per frame */
#define MAX_FRAME_SIZE 1024

/* Maximum number of channels the engine supports. This value can not be
 * changed at present.
 */
#define MAX_OUTPUT_CHANNELS 2

#include <wx/wx.h>
#include <wx/stopwatch.h>
#include <map>
#include "RtAudio.h"
#include "RtMidi.h"
#include "GrandOrgueDef.h"
#include "GOrgueRtHelpers.h"

BEGIN_DECLARE_EVENT_TYPES()

/* Unfortunately, it seems to be necessary to use
 * DECLARE_EXPORTED_EVENT_TYPE when compiling for Windows as the wx headers
 * seem to declare the events as DLL exports when wx-config states that
 * wxWidgets was compiled using dynamic link libraries
 */

	DECLARE_EXPORTED_EVENT_TYPE(wxEMPTY_PARAMETER_VALUE, wxEVT_DRAWSTOP, -1)
	DECLARE_EXPORTED_EVENT_TYPE(wxEMPTY_PARAMETER_VALUE, wxEVT_PUSHBUTTON, -1)
	DECLARE_EXPORTED_EVENT_TYPE(wxEMPTY_PARAMETER_VALUE, wxEVT_ENCLOSURE, -1)
	DECLARE_EXPORTED_EVENT_TYPE(wxEMPTY_PARAMETER_VALUE, wxEVT_NOTEONOFF, -1)
	DECLARE_EXPORTED_EVENT_TYPE(wxEMPTY_PARAMETER_VALUE, wxEVT_LISTENING, -1)
	DECLARE_EXPORTED_EVENT_TYPE(wxEMPTY_PARAMETER_VALUE, wxEVT_METERS, -1)
	DECLARE_EXPORTED_EVENT_TYPE(wxEMPTY_PARAMETER_VALUE, wxEVT_LOADFILE, -1)

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

typedef enum
{
	AC_COMPRESSED_MONO = 0,
	AC_UNCOMPRESSED_MONO = 1,
	AC_COMPRESSED_STEREO = 2,
	AC_UNCOMPRESSED_STEREO = 3
} AUDIO_SECTION_TYPE;

typedef enum {
	GSS_ATTACK = 0,
	GSS_LOOP = 1,
	GSS_RELEASE = 2
} AUDIO_SECTION_STAGE;

struct AUDIO_SECTION_T;
struct GO_SAMPLER_T;

#include "GOrgueReleaseAlignTable.h"

typedef struct AUDIO_SECTION_T
{

	/* Size of the section in BYTES */
	unsigned size;
	unsigned alloc_size;

	/* Type of the data which is stored in the data pointer */
	AUDIO_SECTION_TYPE type;
	AUDIO_SECTION_STAGE stage; /*overflowing,*/

	/* The starting sample and derivatives for each channel (used in the
	 * compression and release-alignment schemes */
	GO_RELEASE_TRACKING_INFO release_tracker_initial;

	/* Pointer to (size) bytes of data encoded in the format (type) */
	unsigned char* data;

} AUDIO_SECTION;

typedef struct GO_SAMPLER_T
{
	GO_SAMPLER_T* next;		// must be first!

	GOrguePipe* pipe;
	const AUDIO_SECTION* pipe_section;

	GO_RELEASE_TRACKING_INFO release_tracker;

	/* the fade parameter is would be more appropriately named "gain". It is
	 * modified on a frame-by frame basis by the fadein and fadeout parameters
	 * which get added to it. The maximum value is defined by fademax and is
	 * a NEGATIVE value. When fade is zero, the sampler will be discarded back
	 * into the sampler pool. When it is equal to fademax, the sample will be
	 * being played back at its appropriate volume (determined by amplitude
	 * factors throughout the organ definition file.
	 *
	 * The time taken for a sample to fade out for a given a value of fadeout
	 * is:
	 * { frames to fadeout } =  { fademax } / { fadeout }
	 * { frame length } = 2 / { sample rate }
	 * { time to fadeout } = { frames to fadeout } * { frame length }
	 *                     = ( 2 * { fademax } ) / ( { fadeout } * { samplerate } )
	 * therefore:
	 * { fadeout } = ( 2 * { fademax } ) / ( { samplerate } * { time to fadeout } )
	 */
	int fade;
	int fadein;
	int fadeout;
	unsigned faderemain;
	int fademax;

	int time;

	int shift;

	/* current byte index of the current block into this sample */
	unsigned position;

} GO_SAMPLER;

class GrandOrgueFile;
class GOrgueMidi;

class GOrgueSound
{

public:

	typedef struct
	{
		RtAudio::Api rt_api;
		int rt_api_subindex;
	} GO_SOUND_DEV_CONFIG;

private:

	/* These are only used by the audio callback... */
	double final_buff[MAX_FRAME_SIZE * MAX_OUTPUT_CHANNELS];
	float volume_buff[MAX_FRAME_SIZE * MAX_OUTPUT_CHANNELS];
	int g_buff[11][(MAX_FRAME_SIZE + BLOCKS_PER_FRAME) * MAX_OUTPUT_CHANNELS];

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

	GO_SAMPLER* windchests[16];		// maximum 16 windchests
	GO_SAMPLER* detachedRelease;		// maximum 1 detach
	GO_SAMPLER* tremulants[10];		// maximum 10 tremulants

public:

	GOrgueSound(void);
	~GOrgueSound(void);

	bool OpenSound(bool wait, GrandOrgueFile* organfile);
	void CloseSound(GrandOrgueFile* organfile);
	bool ResetSound(GrandOrgueFile* organfile);

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
	void StartRecording();
	void StopRecording();

	bool IsActive();
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
