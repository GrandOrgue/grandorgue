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

#include "GOrgueSound.h"

#if defined(__WXMSW__)
    #include <wx/msw/regconf.h>
#endif
#include "GrandOrgue.h"
#include "GrandOrgueFrame.h"
#include "GrandOrgueFile.h"
#include "GrandOrgueID.h"
#include "GOrguePipe.h"
#include "GOrgueStop.h"
#include "GOrgueTremulant.h"
#include "OrganDocument.h"

struct_WAVE WAVE = {{'R','I','F','F'}, 0, {'W','A','V','E'}, {'f','m','t',' '}, 16, 3, 2, 44100, 352800, 8, 32, {'d','a','t','a'}, 0};

#define DELETE_AND_NULL(x) do { if (x) { delete x; x = NULL; } } while (0)

//extern GrandOrgueFile* organfile;
extern const char* s_MIDIMessages[];
extern long i_MIDIMessages[];

GOrgueSound* g_sound = 0;

typedef struct COMPILED_SOUND_API_T
{
	const char* name;
	RtAudio::Api api;
} COMPILED_SOUND_API;

static const COMPILED_SOUND_API g_available_apis[] = {

#ifdef _WIN32

#ifdef __WINDOWS_DS__
	{"DirectSound: ", RtAudio::WINDOWS_DS},
#endif
#ifdef __WINDOWS_ASIO__
	{"ASIO: ", RtAudio::WINDOWS_ASIO},
#endif

#elif linux

#ifdef __UNIX_JACK__
	{"Jack: ", RtAudio::UNIX_JACK},
#endif
#ifdef __LINUX_ALSA__
	{"Alsa: ", RtAudio::LINUX_ALSA},
#endif
#ifdef __LINUX_OSS__
	{"OSS: ", RtAudio::LINUX_OSS},
#endif

#endif /* linux sound api's */

	/* sentinel */
	{NULL, RtAudio::UNSPECIFIED}

};

DEFINE_EVENT_TYPE(wxEVT_DRAWSTOP)
DEFINE_EVENT_TYPE(wxEVT_PUSHBUTTON)
DEFINE_EVENT_TYPE(wxEVT_ENCLOSURE)
DEFINE_EVENT_TYPE(wxEVT_NOTEONOFF)
DEFINE_EVENT_TYPE(wxEVT_LISTENING)
DEFINE_EVENT_TYPE(wxEVT_METERS)
DEFINE_EVENT_TYPE(wxEVT_LOADFILE)

GOrgueSound::GOrgueSound(void) :
	sw(),
	pConfig(wxConfigBase::Get()),
	format(0),
	logSoundErrors(false),
	samplers_count(0),
	polyphony(0),
	poly_soft(0),
	volume(0),
	transpose(0),
	m_audioDevices(),
	audioDevice(NULL),
	n_latency(0),
	m_midi_device_map(),
	m_midi_devices(),
/*	midiDevices(NULL),
	b_midiDevices(NULL),
	i_midiDevices(NULL),
	n_midiDevices(0),*/
	b_limit(0),
	b_stereo(0),
	b_align(0),
	b_scale(0),
	b_random(0),
	b_stoprecording(false),
	f_output(NULL),
	meter_counter(0),
	meter_poly(0),
	meter_left(0),
	meter_right(0),
	b_active(false), b_listening(false),
	listen_evthandler(NULL),
	defaultAudio(""),
	b_memset(false),
	b_detach(0),
	organmidiEvents()
{

	g_sound = this;

	try
	{

		for (unsigned k = 0; g_available_apis[k].name != NULL; k++)
		{

			audioDevice = new RtAudio(g_available_apis[k].api);

			for (size_t i = 0; i < audioDevice->getDeviceCount(); i++)
			{

				RtAudio::DeviceInfo info = audioDevice->getDeviceInfo(i);

				wxString name = info.name.c_str();
				name.Replace("\\", "|");
				name = wxString(g_available_apis[k].name) + name;

				if (info.isDefaultOutput && defaultAudio.IsEmpty())
					defaultAudio = name;

				unsigned sample_rate_index = info.sampleRates.size();
				if (g_available_apis[k].api != RtAudio::WINDOWS_DS)
				{
					/* TODO: can a Windows developer explain why this is necessary? */
					for (unsigned j = 0; j < sample_rate_index; j++)
						if (info.sampleRates[j] == 44100)
							sample_rate_index = j;
				}
				else
				{
					sample_rate_index = 0;
					if (info.sampleRates.size() && info.sampleRates.back() < 44100)
						sample_rate_index = info.sampleRates.size();
				}

				if (
						(info.outputChannels < 2) ||
						(!info.probed) ||
						(sample_rate_index == (int)info.sampleRates.size()) ||
						(m_audioDevices.find(name) != m_audioDevices.end())
					)
					continue;

				m_audioDevices[name] = std::make_pair(i, g_available_apis[k].api);

			}

			delete audioDevice;
			audioDevice = 0;

		}

		RtMidiIn midi_dev;
		for (unsigned i = 0; i <  midi_dev.getPortCount(); i++)
		{

			MIDI_DEVICE t;
			t.midi_in = new RtMidiIn();
			t.id = 0;
			t.active = false;
			m_midi_devices.push_back(t);

			wxString name = t.midi_in->getPortName(i).c_str();
			name.Replace("\\", "|");
			m_midi_device_map[name] = i;

		}

	}
	catch (RtError &e)
	{
		e.printMessage();
		CloseSound(NULL);
	}

}

GOrgueSound::~GOrgueSound(void)
{

	CloseSound(NULL);

	try
	{

		/* dispose of all midi devices */
		while (m_midi_devices.size())
		{
			if (m_midi_devices.back().midi_in)
				m_midi_devices.back().midi_in->closePort();
			DELETE_AND_NULL(m_midi_devices.back().midi_in);
			m_midi_devices.pop_back();
		}

		/* dispose of the audio playback device */
		DELETE_AND_NULL(audioDevice);

	}
	catch (RtError &e)
	{
		e.printMessage();
	}

}

static
void GetDefaultDirectSoundConfig(const int latency, unsigned &nb_buffers, unsigned &buffer_size)
{

	int bufferCalc = (latency * 25);

	nb_buffers = 2;

	if (bufferCalc <= 256)
	{
		buffer_size = 128;
		return;
	}

	if (bufferCalc <= 512)
	{
		buffer_size = 256;
		return;
	}

	if (bufferCalc <= 768)
	{
		buffer_size = 384;
		return;
	}

	if (bufferCalc <= 3072)
	{
		buffer_size = 512;
		nb_buffers = bufferCalc / 512;
		if (bufferCalc % 512)
			nb_buffers++;
		return;
	}

	buffer_size = 1024;
	nb_buffers = bufferCalc / 1024;
	if (bufferCalc % 1024)
		nb_buffers++;

}

static
void GetDefaultAsioSoundConfig(const int latency, unsigned &nb_buffers, unsigned &buffer_size)
{

	int buffer_calc = (latency * 25);

	nb_buffers = 2;

	if (buffer_calc <= 256)
		buffer_size = 128;
	else if (buffer_calc <= 512)
		buffer_size = 256;
	else if (buffer_calc <= 768)
		buffer_size = 384;
	else if (buffer_calc <= 1024)
		buffer_size = 512;
	else if (buffer_calc <= 1536)
		buffer_size = 768;
	else
		buffer_size = 1024;

}

static
void GetDefaultJackSoundConfig(const int latency, unsigned &nb_buffers, unsigned &buffer_size)
{

	nb_buffers = 1;
	buffer_size = 1024;

}

static
void GetDefaultUnknownSoundConfig(const int latency, unsigned &nb_buffers, unsigned &buffer_size)
{

	buffer_size = 128;

	nb_buffers = (latency * 25) / buffer_size;
	if (nb_buffers < 2)
		nb_buffers = 2;

}

bool GOrgueSound::OpenSound(bool wait, GrandOrgueFile* organfile)
{
	int i;

	std::map<wxString, int>::iterator it2;
	for (it2 = m_midi_device_map.begin(); it2 != m_midi_device_map.end(); it2++)
		pConfig->Read("Devices/MIDI/" + it2->first, -1);
	for (i = 0; i < 16; i++)
		i_midiEvents[i] = pConfig->Read(wxString("MIDI/") + s_MIDIMessages[i], i_MIDIMessages[i]);

	defaultAudio = pConfig->Read("Devices/DefaultSound", defaultAudio);
	n_latency = pConfig->Read("Devices/Sound/" + defaultAudio, 12);
	volume = pConfig->Read("Volume", 50);
	polyphony = pConfig->Read("PolyphonyLimit", 2048);
	poly_soft = (polyphony * 3) / 4;
	b_stereo = pConfig->Read("StereoEnabled", 1);
	b_limit  = pConfig->Read("ManagePolyphony", 1);
	b_align  = pConfig->Read("AlignRelease", 1);
	b_detach = pConfig->Read("DetachRelease", 1);
	b_scale  = pConfig->Read("ScaleRelease", 1);
	b_random = pConfig->Read("RandomizeSpeaking", 1);

	samplers_count = 0;
	for (i = 0; i < MAX_POLYPHONY; i++)
		samplers_open[i] = samplers + i;
	for (i = 0; i < 26; i++)
		windchests[i] = 0;

	try
	{
		std::map<wxString, int>::iterator it2;
		int n_midi = 0;
		for (it2 = m_midi_device_map.begin(); it2 != m_midi_device_map.end(); it2++)
		{
			i = pConfig->Read("Devices/MIDI/" + it2->first, 0L);
			MIDI_DEVICE& this_dev = m_midi_devices[it2->second];
			if (i >= 0)
			{
				n_midi++;
				this_dev.id = i;
				if (!this_dev.active)
				{
					assert(this_dev.midi_in);
					this_dev.midi_in->openPort(it2->second);
					this_dev.active = true;
				}
			}
			else if (this_dev.active)
			{
				this_dev.active = false;
				this_dev.midi_in->closePort();
			}
		}

		std::map<wxString, std::pair<int, RtAudio::Api> >::iterator it;
		it = m_audioDevices.find(defaultAudio.c_str());
		if (it != m_audioDevices.end())
		{
			unsigned int bufferSize, numberOfBuffers;
			audioDevice = new RtAudio(it->second.second);
			if (defaultAudio.StartsWith("ASIO: "))
				GetDefaultAsioSoundConfig(n_latency, numberOfBuffers, bufferSize);
			else if (defaultAudio.StartsWith("Jack: "))
				GetDefaultJackSoundConfig(n_latency, numberOfBuffers, bufferSize);
            else if (defaultAudio.StartsWith("DirectSound: "))
            	GetDefaultDirectSoundConfig(n_latency, numberOfBuffers, bufferSize);
			else
				GetDefaultUnknownSoundConfig(n_latency, numberOfBuffers, bufferSize);

			RtAudio::StreamParameters aOutputParam;
			aOutputParam.deviceId=it->second.first;
			aOutputParam.nChannels=2; //stereo

			RtAudio::StreamOptions aOptions;
			aOptions.numberOfBuffers=numberOfBuffers;
			audioDevice->openStream(&aOutputParam, NULL, RTAUDIO_FLOAT32, 44100, &bufferSize, &GOrgueSound::AudioCallback, this, &aOptions);

			if (bufferSize <= 1024)
			{
				n_latency = (bufferSize * numberOfBuffers ) / 25;
				pConfig->Write("Devices/Sound/" + defaultAudio, n_latency);
				audioDevice->startStream();
			}
			else
			{
				::wxSleep(1);
				if (logSoundErrors)
					::wxLogError("Cannot use buffer size above 1024 samples; unacceptable quantization would occur.");
				CloseSound(organfile);
				return false;
			}
			RtAudio::DeviceInfo info = audioDevice->getDeviceInfo(it->second.first);
            format = RTAUDIO_FLOAT32;
		}

		if (!n_midi || it == m_audioDevices.end())
		{
            ::wxSleep(1);
			if (logSoundErrors)
                ::wxLogWarning(n_midi ? "No audio device is selected; neither MIDI input nor sound output will occur!" : "No MIDI devices are selected for listening; neither MIDI input nor sound output will occur!");
			CloseSound(organfile);
			return false;
		}
	}
	catch (RtError &e)
	{
		::wxSleep(1);
		if (logSoundErrors)
            e.printMessage();
		CloseSound(organfile);
		return false;
	}

	if (wait)
		::wxSleep(1);

	return true;
}

void GOrgueSound::CloseSound(GrandOrgueFile* organfile)
{
	bool was_active = b_active;

	if (f_output)
	{
		b_stoprecording = true;
		::wxMilliSleep(100);
		CloseWAV();		// this should never be necessary...
	}

	b_active = false;

	try
	{
		if (audioDevice)
		{
			audioDevice->abortStream();
			audioDevice->closeStream();
			delete audioDevice;
			audioDevice = 0;
		}
	}
	catch (RtError &e)
	{
		e.printMessage();
	}

	::wxMilliSleep(10);
	if (was_active)
		MIDIAllNotesOff(organfile);
}

bool GOrgueSound::ResetSound(GrandOrgueFile* organfile)
{
	wxBusyCursor busy;
	bool was_active = b_active;

	int temp = volume;
	CloseSound(organfile);
	if (!OpenSound(true, organfile))
		return false;
	if (!temp)  // don't let resetting sound reactivate an organ
        g_sound->volume = temp;
	b_active = was_active;
	if (organfile)
	{
        OrganDocument* doc = (OrganDocument*)(::wxGetApp().m_docManager->GetCurrentDocument());
        if (doc && doc->b_loaded)
        {
            b_active = true;
            for (int i = 0; i < organfile->GetTremulantCount(); i++)
            {
                if (organfile->GetTremulant(i)->DefaultToEngaged)
                {
                    organfile->GetTremulant(i)->Set(false);
                    organfile->GetTremulant(i)->Set(true);
                }
            }
        }
	}
	return true;
}

void GOrgueSound::CloseWAV()
{
	if (!f_output)
		return;
	WAVE.Subchunk2Size = (WAVE.ChunkSize = ftell(f_output) - 8) - 36;
    fseek(f_output, 0, SEEK_SET);
	fwrite(&WAVE, sizeof(WAVE), 1, f_output);
	fclose(f_output);
	b_stoprecording = false;
	f_output = 0;
}

void GOrgueSound::UpdateOrganMIDI()
{
    long count=pConfig->Read("OrganMIDI/Count", 0L);
    for (long i=0; i<count; i++) {
        wxString itemstr = "OrganMIDI/Organ" + wxString::Format("%ld", i);
        long j=pConfig->Read(itemstr+".midi", 0L);
        wxString file=pConfig->Read(itemstr+".file");
        organmidiEvents.insert(std::pair<long, wxString>(j, file) );
    }
}

void GOrgueSound::SetPolyphonyLimit(int polyphony)
{
	this->polyphony = polyphony;
}

void GOrgueSound::SetPolyphonySoftLimit(int polyphony_soft)
{
	this->poly_soft = polyphony_soft;
}

void GOrgueSound::SetVolume(int volume)
{
	this->volume = volume;
}

void GOrgueSound::SetTranspose(int transpose)
{
	this->transpose = transpose;
}

GO_SAMPLER* GOrgueSound::OpenNewSampler()
{

	if (samplers_count >= polyphony)
		return NULL;

	GO_SAMPLER* sampler = samplers_open[samplers_count++];
	memset(sampler, 0, sizeof(GO_SAMPLER));
	return sampler;

}

bool GOrgueSound::HasRandomPipeSpeech()
{
	return b_random;
}

bool GOrgueSound::HasReleaseAlignment()
{
	return b_align;
}

bool GOrgueSound::HasScaledReleases()
{
	return b_scale;
}

void GOrgueSound::MIDIAllNotesOff(GrandOrgueFile* organfile)
{
	int i, j, k;

	if (!organfile)
		return;

	for (i = organfile->GetFirstManualIndex(); i <= organfile->GetManualAndPedalCount(); i++)
	{
		organfile->GetManual(i)->AllNotesOff();
		for (j = 0; j < organfile->GetManual(i)->GetStopCount(); j++)
		{
			for (k = 0; k < organfile->GetManual(i)->GetStop(j)->NumberOfLogicalPipes; k++)
				organfile->GetPipe(organfile->GetManual(i)->GetStop(j)->pipe[k])->FastAbort();
			if (organfile->GetManual(i)->GetStop(j)->m_auto)
                organfile->GetManual(i)->GetStop(j)->Set(false);
		}
	}
}

bool GOrgueSound::IsStereo()
{
	return b_stereo;
}

int GOrgueSound::GetVolume()
{
	return volume;
}

bool GOrgueSound::IsRecording()
{
	return f_output && !b_stoprecording;
}

/* FIXME: This code is not thread-safe and is likely to cause future problems */
void GOrgueSound::StartRecording()
{
	if (f_output)
        return;
	b_stoprecording = false;
	wxFileDialog dlg(::wxGetApp().frame, _("Save as"), wxConfig::Get()->Read("wavPath", ::wxGetApp().m_path), wxEmptyString, _("WAV files (*.wav)|*.wav"), wxSAVE | wxOVERWRITE_PROMPT);
	if (dlg.ShowModal() == wxID_OK)
	{
        wxConfig::Get()->Write("wavPath", dlg.GetDirectory());
        wxString filepath = dlg.GetPath();
        if (filepath.Find(".wav") == wxNOT_FOUND) {
            filepath.append(".wav");
        }
        if ((f_output = fopen(filepath, "wb")))
            fwrite(&WAVE, sizeof(WAVE), 1, f_output);
        else
            ::wxLogError("Unable to open file for writing");

	}
}

void GOrgueSound::StopRecording()
{

	if (!f_output)
		return;

	b_stoprecording = true;

}

bool GOrgueSound::HasMIDIDevice()
{

	for (unsigned i = 0; i < m_midi_devices.size(); i++)
		if (m_midi_devices[i].active)
			return true;

	return false;

}

bool GOrgueSound::HasMIDIListener()
{

	return (listen_evthandler) && (b_listening);

}

void GOrgueSound::SetMIDIListener(wxEvtHandler* handler)
{
	if (handler == NULL)
	{
		b_listening = false;
		listen_evthandler = NULL;
	}
	else
	{
		listen_evthandler = handler;
		b_listening = true;
	}
}

bool GOrgueSound::IsActive()
{
	return b_active;
}

void GOrgueSound::ActivatePlayback()
{
	/* FIXME: we should probably check that the driver is actually open */
	b_active = true;
}

void GOrgueSound::SetLogSoundErrorMessages(bool settingsDialogVisible)
{
	logSoundErrors = settingsDialogVisible;
}

std::map<wxString, std::pair<int, RtAudio::Api> >& GOrgueSound::GetAudioDevices()
{
	return m_audioDevices;
}

std::map<wxString, int>& GOrgueSound::GetMIDIDevices()
{
	return m_midi_device_map;
}

const wxString GOrgueSound::GetDefaultAudioDevice()
{
	return defaultAudio;
}

const RtAudioFormat GOrgueSound::GetAudioFormat()
{
	return format;
}
