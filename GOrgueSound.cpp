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

extern GrandOrgueFile* organfile;
extern const char* s_MIDIMessages[];
extern long i_MIDIMessages[];

GOrgueSound* g_sound = 0;

#ifdef _WIN32
 RtAudio::Api i_APIs[] = { RtAudio::WINDOWS_DS, RtAudio::WINDOWS_ASIO };
 const char* s_APIs[] = { "DirectSound: ", "ASIO: " };
#endif

#ifdef linux
  RtAudio::Api i_APIs[] = {RtAudio::UNIX_JACK, RtAudio::LINUX_ALSA};// RtAudio::LINUX_OSS };
  std::string  s_APIs[] = {"Jack: ", "Alsa: " }; //{ " Jack: ", "Alsa: ", "OSS: " };
#endif

#include "GOrgueSoundCallbackMIDI.h"
#include "GOrgueSoundCallbackAudio.h"

DEFINE_EVENT_TYPE(wxEVT_DRAWSTOP)
DEFINE_EVENT_TYPE(wxEVT_PUSHBUTTON)
DEFINE_EVENT_TYPE(wxEVT_ENCLOSURE)
DEFINE_EVENT_TYPE(wxEVT_NOTEONOFF)
DEFINE_EVENT_TYPE(wxEVT_LISTENING)
DEFINE_EVENT_TYPE(wxEVT_METERS)
DEFINE_EVENT_TYPE(wxEVT_LOADFILE)

GOrgueSound::GOrgueSound(void)
: organmidiEvents(),
  sw(),
  pConfig(wxConfigBase::Get()),
  format(0),
  logSoundErrors(NULL),
  samplers_count(0),
  polyphony(0),
  poly_soft(0),
  volume(0),
  transpose(0),
  m_audioDevices(),
  audioDevice(NULL),
  n_latency(0),
  m_midiDevices(),
  midiDevices(NULL),
  b_midiDevices(NULL),
  i_midiDevices(NULL),
  n_midiDevices(0),
  b_limit(0), b_stereo(0), b_align(0), b_detach(0), b_scale(0), b_random(0),
  b_stoprecording(false), b_memset(false),
  f_output(NULL),
  meter_counter(0),
  meter_poly(0),
  meter_left(0),
  meter_right(0),
  b_active(false), b_listening(false),
  listen_evthandler(NULL),
  defaultAudio("")
{
	int j, k;
	g_sound = this;

	try
	{
		for (k = 0; k < (int)(sizeof(i_APIs) / sizeof(RtAudio::Api)); k++)
		{
        	    audioDevice = new RtAudio(i_APIs[k]);

			for (size_t i = 0; i < audioDevice->getDeviceCount(); i++)
			{
			  RtAudio::DeviceInfo info = audioDevice->getDeviceInfo(i);
				wxString name = info.name.c_str();
				name.Replace("\\", "|");
				name = s_APIs[k] + name;
				if (info.isDefaultOutput && defaultAudio.IsEmpty())
					defaultAudio = name;
				if (i_APIs[k] != RtAudio::WINDOWS_DS)
				{
					for (j = 0; j < (int)info.sampleRates.size(); j++)
						if (info.sampleRates[j] == 44100)
							break;
				}
				else
				{
					j = 0;
					if (info.sampleRates.size() && info.sampleRates.back() < 44100)
						j = info.sampleRates.size();
				}
				if (info.outputChannels < 2 || !info.probed || j == (int)info.sampleRates.size() || m_audioDevices.find(name) != m_audioDevices.end())
					continue;
				m_audioDevices[name] = std::make_pair(i, i_APIs[k]);
			}
			delete audioDevice;
			audioDevice = 0;
		}

		RtMidiIn midiDevice;
		n_midiDevices = midiDevice.getPortCount();

		midiDevices = new RtMidiIn*[n_midiDevices];
		b_midiDevices = new bool[n_midiDevices];
		i_midiDevices = new int[n_midiDevices];
		for (int i = 0; i < n_midiDevices; i++)
		{
			midiDevices[i] = new RtMidiIn();
			wxString name = midiDevices[i]->getPortName(i).c_str();
			name.Replace("\\", "|");
			m_midiDevices[name] = i;
			b_midiDevices[i] = false;
			i_midiDevices[i] = 0;
		}
	}
	catch (RtError &e)
	{
		e.printMessage();
		CloseSound();
	}
}

GOrgueSound::~GOrgueSound(void)
{
	CloseSound();
	try
	{
		if (midiDevices)
		{
			for (int i = 0; i < n_midiDevices; i++)
			{
				if (midiDevices[i])
				{
					if (b_midiDevices[i])
						midiDevices[i]->closePort();
					delete midiDevices[i];
				}
			}
			delete [] midiDevices;
			midiDevices = 0;
		}
		if (b_midiDevices)
			delete [] b_midiDevices;
		if (i_midiDevices)
			delete [] i_midiDevices;
		if (audioDevice)
		{
			delete audioDevice;
			audioDevice = 0;
		}
	}
	catch (RtError &e)
	{
		e.printMessage();
	}
}

bool GOrgueSound::OpenSound(bool wait)
{
	int i;

	std::map<wxString, int>::iterator it2;
	for (it2 = m_midiDevices.begin(); it2 != m_midiDevices.end(); it2++)
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
		for (it2 = m_midiDevices.begin(); it2 != m_midiDevices.end(); it2++)
		{
			i = pConfig->Read("Devices/MIDI/" + it2->first, 0L);
			if (i >= 0)
			{
				n_midi++;
				i_midiDevices[it2->second] = i;
				if (!b_midiDevices[it2->second])
				{
					b_midiDevices[it2->second] = true;
 					midiDevices[it2->second]->openPort(it2->second);
 					//midiDevices[it2->second]->ignoreTypes(false,false,false);
				}
			}
			else if (b_midiDevices[it2->second])
			{
				b_midiDevices[it2->second] = false;
				midiDevices[it2->second]->closePort();
			}
		}

		std::map<wxString, std::pair<int, RtAudio::Api> >::iterator it;
		it = m_audioDevices.find(defaultAudio.c_str());
		if (it != m_audioDevices.end())
		{
			unsigned int bufferSize, numberOfBuffers;
			audioDevice = new RtAudio(it->second.second);
			if (defaultAudio.StartsWith("ASIO: "))
			{
			    numberOfBuffers = 2;
			    int bufferCalc = (n_latency * 25);
                if (bufferCalc <= 256) bufferSize = 128;
                else if (bufferCalc > 256)
                    {
                        if (bufferCalc <= 512) bufferSize = 256;
                        else if (bufferCalc > 512)
                        {
                            if (bufferCalc <= 768) bufferSize = 384;
                            else if (bufferCalc > 768)
                                {
                                    if (bufferCalc <= 1024) bufferSize = 512;
                                    else if (bufferCalc > 1024)
                                        {
                                            if (bufferCalc <= 1536) bufferSize = 768;
                                            else if (bufferCalc > 1536) bufferSize = 1024;
                                        }
                                }
                        }
                    }
			}
			else if (defaultAudio.StartsWith("Jack: "))
			{
			    numberOfBuffers = 1;
			    bufferSize=1024;
			}
            else if (defaultAudio.StartsWith("DirectSound: "))
            {
                numberOfBuffers = 2;
                int bufferCalc = (n_latency * 25);
                if (bufferCalc <= 256) bufferSize = 128;
                else if (bufferCalc > 256)
                    {
                        if (bufferCalc <= 512) bufferSize = 256;
                        else if (bufferCalc > 512)
                        {
                            if (bufferCalc <= 768) bufferSize = 384;
                            else if (bufferCalc > 768)
                                {
                                    if (bufferCalc <= 1024) bufferSize = 512;
                                    else if (bufferCalc > 1024)
                                    {
                                        numberOfBuffers = 3;
                                            {
                                                if (bufferCalc <= 1536) bufferSize = 512;
                                                else if (bufferCalc > 1536)
                                                    {
                                                    if (bufferCalc <= 2048)
                                                            {
                                                                numberOfBuffers = 4;
                                                                bufferSize = 512;
                                                            }
                                                        else if (bufferCalc > 2048)
                                                            {
                                                                numberOfBuffers = 5;
                                                                if (bufferCalc <= 2560) bufferSize = 512;
                                                                else if (bufferCalc > 2560)
                                                                    {
                                                                        numberOfBuffers = 6;
                                                                        if (bufferCalc <= 3072) bufferSize = 512;
                                                                        else if (bufferCalc > 3072) bufferSize = 1024;
                                                                    }
                                                            }

                                                    }
                                            }
                                    }
                                }
                        }
                    }
            }
			else
			{
			    bufferSize = 128;
			    numberOfBuffers = (n_latency * 25) / bufferSize;
			    if (numberOfBuffers < 2) numberOfBuffers = 2;
			}

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
		                CloseSound();
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
			CloseSound();
			return false;
		}
	}
	catch (RtError &e)
	{
		::wxSleep(1);
		if (logSoundErrors)
            e.printMessage();
		CloseSound();
		return false;
	}

	if (wait)
		::wxSleep(1);

	return true;
}

void GOrgueSound::CloseSound()
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
		MIDIAllNotesOff();
}

bool GOrgueSound::ResetSound()
{
	wxBusyCursor busy;
	bool was_active = b_active;

	int temp = volume;
	CloseSound();
	if (!OpenSound())
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

void GOrgueSound::MIDIPretend(bool on)
{
	for (int i = organfile->GetFirstManualIndex(); i <= organfile->GetManualAndPedalCount(); i++)
		organfile->GetManual(i)->MIDIPretend(on);
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

GOrgueSampler* GOrgueSound::OpenNewSampler()
{

	if (samplers_count >= polyphony)
		return NULL;

	GOrgueSampler* sampler = samplers_open[samplers_count++];
	memset(sampler, 0, sizeof(GOrgueSampler));
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

void GOrgueSound::MIDIAllNotesOff()
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
			{
				register GOrguePipe* pipe = organfile->GetPipe(organfile->GetManual(i)->GetStop(j)->pipe[k]);
				if (pipe->instances > -1)
					pipe->instances = 0;
				pipe->sampler = 0;
			}
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
		if ((f_output = fopen(dlg.GetPath(), "wb")))
			fwrite(&WAVE, sizeof(WAVE), 1, f_output);
		else
			::wxLogError("Unable to open file for writing");
	}
}

void GOrgueSound::StopRecording()
{
	if (f_output)
		b_stoprecording = true;
		return;
}

bool GOrgueSound::HasMIDIDevice()
{
	for (int i = 0; i < n_midiDevices; i++)
		if (b_midiDevices[i])
			return true;
}

bool GOrgueSound::HasMIDIListener()
{
	return (listen_evthandler) && b_listening;
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
	return m_midiDevices;
}

const wxString GOrgueSound::GetDefaultAudioDevice()
{
	return defaultAudio;
}

const RtAudioFormat GOrgueSound::GetAudioFormat()
{
	return format;
}
