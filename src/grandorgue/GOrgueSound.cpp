/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2012 GrandOrgue contributors (see AUTHORS)
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

#include "GOrgueSound.h"
#include "GOSoundThread.h"
#include "GrandOrgueFile.h"
#include "GOrgueRtHelpers.h"
#include "GOrgueMidi.h"
#include "GOrgueSettings.h"
#include "GOrgueEvent.h"

#define DELETE_AND_NULL(x) do { if (x) { delete x; x = NULL; } } while (0)

GOrgueSound* g_sound = 0;

GOrgueSound::GOrgueSound(GOrgueSettings& settings) :
	format(0),
	logSoundErrors(true),
	m_audioDevices(),
	m_AudioOutputs(),
	m_SamplesPerBuffer(0),
	m_nb_buffers(0),
	meter_counter(0),
	defaultAudio(),
	defaultAudioDevice(),
	m_organfile(0),
	m_Settings(settings)
{

	g_sound = this;
	memset(&meter_info, 0, sizeof(meter_info));

	try
	{
		m_midi = new GOrgueMidi(m_Settings);
	}
	catch (RtError &e)
	{
		wxString error = wxString::FromAscii(e.getMessage().c_str());
		wxLogError(_("RtMidi error: %s"), error.c_str());
	}

	RtAudio* audioDevice = 0;
	try
	{
		std::vector<RtAudio::Api> rtaudio_apis;
		RtAudio::getCompiledApi(rtaudio_apis);

		for (unsigned k = 0; k < rtaudio_apis.size(); k++)
		{
			audioDevice = new RtAudio(rtaudio_apis[k]);
			for (unsigned i = 0; i < audioDevice->getDeviceCount(); i++)
			{

				RtAudio::DeviceInfo info = audioDevice->getDeviceInfo(i);
				wxString dev_name = wxString::FromAscii(info.name.c_str());
				dev_name.Replace(wxT("\\"), wxT("|"));
				wxString name = wxString(GOrgueRtHelpers::GetApiName(rtaudio_apis[k])) + wxString(wxT(": ")) + dev_name;

				if (info.isDefaultOutput && defaultAudioDevice.IsEmpty())
					defaultAudioDevice = name;

				unsigned sample_rate_index = info.sampleRates.size();

				for (unsigned j = 0; j < info.sampleRates.size(); j++)
				{
					if (info.sampleRates[j] == 44100)
						sample_rate_index = j;
					if (info.sampleRates[j] == 48000)
						sample_rate_index = j;
				}

				if (
						(info.outputChannels < 2) ||
						(!info.probed) ||
						(sample_rate_index == info.sampleRates.size()) ||
						(m_audioDevices.find(name) != m_audioDevices.end())
					)
					continue;

				GO_SOUND_DEV_CONFIG cfg;
				cfg.rt_api = rtaudio_apis[k];
				cfg.rt_api_subindex = i;
				m_audioDevices[name] = cfg;

			}

			delete audioDevice;
			audioDevice = 0;

		}
	}
	catch (RtError &e)
	{
		wxString error = wxString::FromAscii(e.getMessage().c_str());
		wxLogError(_("RtAudio error: %s"), error.c_str());
		if (audioDevice)
			delete audioDevice;
	}

	Pa_Initialize();
	
	for(int i = 0; i < Pa_GetDeviceCount(); i++)
	{
		const PaDeviceInfo* info = Pa_GetDeviceInfo(i);
		const PaHostApiInfo *api = Pa_GetHostApiInfo(info->hostApi);
		if (info->maxOutputChannels < 2)
			continue;
		GO_SOUND_DEV_CONFIG cfg;
		cfg.rt_api = RTAPI_PORTAUDIO;
		cfg.rt_api_subindex = i;
		wxString name = wxGetTranslation(wxString::FromAscii(api->name)) + wxString(_(" (PA): ")) + wxString::FromAscii(info->name);
		m_audioDevices[name] = cfg;
		if (Pa_GetDefaultOutputDevice() == i)
			defaultAudioDevice = name;
	}
}

GOrgueSound::~GOrgueSound()
{

	CloseSound();

	/* dispose of midi devices */
	DELETE_AND_NULL(m_midi);

	Pa_Terminate ();
}

void GOrgueSound::StartThreads()
{
	StopThreads();

	int n_cpus = m_Settings.GetConcurrency();
	unsigned tasks = GetEngine().GetGroupCount();

	wxCriticalSectionLocker thread_locker(m_thread_lock);
	float fact = tasks / (!n_cpus ? 1 : n_cpus);
	unsigned no = 1;
	/* We don't create extra thread for last cpu */
	for(int i = 0; i < n_cpus; i++, no++)
	{
		if (no > tasks)
			break;
		unsigned end = no + (fact + 0.5);
		if (end > tasks)
			end = tasks;
		m_Threads.push_back(new GOSoundThread(&GetEngine(), no - 1, end - 1, m_SamplesPerBuffer));
		no = end;
	}

	for(unsigned i = 0; i < m_Threads.size(); i++)
		m_Threads[i]->Run();
}

void GOrgueSound::StopThreads()
{
	for(unsigned i = 0; i < m_Threads.size(); i++)
		m_Threads[i]->Delete();

	wxCriticalSectionLocker thread_locker(m_thread_lock);
	m_Threads.resize(0);
}

void GOrgueSound::OpenMidi()
{
	try
	{
		m_midi->Open();
	}
	catch (RtError &e)
	{
		wxString error = wxString::FromAscii(e.getMessage().c_str());
		throw wxString::Format(_("RtMidi error: %s"), error.c_str());
	}
}

bool GOrgueSound::OpenSound()
{
	assert(m_AudioOutputs.size() == 0);
	bool opened_ok = false;

	m_AudioOutputs.resize(1);
	for(unsigned i = 0; i < m_AudioOutputs.size(); i++)
	{
		m_AudioOutputs[i].sound = this;
		m_AudioOutputs[i].audioDevice = 0;
		m_AudioOutputs[i].audioStream = 0;
	}

	defaultAudio = m_Settings.GetDefaultAudioDevice();
	m_SoundEngine.SetPolyphonyLimiting(m_Settings.GetManagePolyphony());
	m_SoundEngine.SetHardPolyphony(m_Settings.GetPolyphonyLimit());
	m_SoundEngine.SetVolume(m_Settings.GetVolume());
	m_SoundEngine.SetScaledReleases(m_Settings.GetScaleRelease());
	m_SoundEngine.SetRandomizeSpeaking(m_Settings.GetRandomizeSpeaking());
	m_SoundEngine.SetInterpolationType(m_Settings.GetInterpolationType());
	unsigned sample_rate = m_Settings.GetSampleRate();
	m_recorder.SetBytesPerSample(m_Settings.GetWaveFormatBytesPerSample());

	PreparePlayback(NULL);

	try
	{
		OpenMidi();

		if (m_audioDevices.find(defaultAudio) == m_audioDevices.end())
		{
			defaultAudio = defaultAudioDevice;
			m_Settings.SetDefaultAudioDevice(defaultAudio);
		}

		std::map<wxString, GO_SOUND_DEV_CONFIG>::iterator it;
		it = m_audioDevices.find(defaultAudio);
		if (it != m_audioDevices.end())
		{
			GetEngine().SetSampleRate(sample_rate);
			m_recorder.SetSampleRate(sample_rate);

			unsigned try_latency = m_Settings.GetAudioDeviceLatency(defaultAudio);
			if (it->second.rt_api == RTAPI_PORTAUDIO)
			{
				format = RTAUDIO_FLOAT32;
				m_SamplesPerBuffer = BLOCKS_PER_FRAME * ceil(sample_rate * try_latency / 1000.0 / BLOCKS_PER_FRAME);
				if (m_SamplesPerBuffer > MAX_FRAME_SIZE)
					m_SamplesPerBuffer = MAX_FRAME_SIZE;
				if (m_SamplesPerBuffer < BLOCKS_PER_FRAME)
					m_SamplesPerBuffer = BLOCKS_PER_FRAME;

				PaStreamParameters stream_parameters;
				stream_parameters.device = it->second.rt_api_subindex;
				stream_parameters.channelCount = 2;
				stream_parameters.sampleFormat = paFloat32;
				stream_parameters.suggestedLatency = try_latency / 1000.0;
				stream_parameters.hostApiSpecificStreamInfo = NULL;

				PaError error;
				error = Pa_OpenStream(&m_AudioOutputs[0].audioStream, NULL, &stream_parameters, sample_rate, m_SamplesPerBuffer,
						      paNoFlag, &GOrgueSound:: PaAudioCallback, &m_AudioOutputs[0]);
				if (error != paNoError)
					throw (wxString)wxGetTranslation(wxString::FromAscii(Pa_GetErrorText(error)));
				
				error = Pa_StartStream(m_AudioOutputs[0].audioStream);
				if (error != paNoError)
					throw (wxString)wxGetTranslation(wxString::FromAscii(Pa_GetErrorText(error)));

				const struct PaStreamInfo* info = Pa_GetStreamInfo(m_AudioOutputs[0].audioStream);
				m_Settings.SetAudioDeviceActualLatency(defaultAudio, (int)(info->outputLatency * 1000));

				GetEngine().SetSampleRate(sample_rate);
				m_recorder.SetSampleRate(sample_rate);
			}
			else
			{
				m_AudioOutputs[0].audioDevice = new RtAudio(it->second.rt_api);

				GOrgueRtHelpers::GetBufferConfig
					(it->second.rt_api
					,try_latency
					,sample_rate
					,&m_nb_buffers
					,&m_SamplesPerBuffer
					);

				RtAudio::StreamParameters aOutputParam;
				aOutputParam.deviceId = it->second.rt_api_subindex;
				aOutputParam.nChannels = 2; //stereo

				RtAudio::StreamOptions aOptions;
				aOptions.numberOfBuffers = m_nb_buffers;

				format = RTAUDIO_FLOAT32;
				m_AudioOutputs[0].audioDevice->openStream
					(&aOutputParam
					 ,NULL
					 ,format
					 ,sample_rate
					 ,&m_SamplesPerBuffer
					 ,&GOrgueSound::AudioCallback
					 ,&m_AudioOutputs[0]
					 ,&aOptions
					 );

				m_nb_buffers = aOptions.numberOfBuffers;

				if (m_SamplesPerBuffer <= 1024)
				{
					m_AudioOutputs[0].audioDevice->startStream();

					int actual_latency = m_AudioOutputs[0].audioDevice->getStreamLatency();
					
					/* getStreamLatency returns zero if not supported by the API, in which
					 * case we will make a best guess.
					 */
					if (actual_latency == 0)
						actual_latency = m_SamplesPerBuffer * m_nb_buffers;

					m_Settings.SetAudioDeviceActualLatency(defaultAudio, (actual_latency * 1000) / GetEngine().GetSampleRate());
				}
				else
					throw (wxString)_("Cannot use buffer size above 1024 samples; unacceptable quantization would occur.");

				if (m_SamplesPerBuffer & (BLOCKS_PER_FRAME - 1))
				{
					wxLogWarning
						(_("Audio engine wants frame size of %u blocks which is indivisible by %u.")
						 ,m_SamplesPerBuffer
						 ,BLOCKS_PER_FRAME
						 );
				}
				GetEngine().SetSampleRate(m_AudioOutputs[0].audioDevice->getStreamSampleRate());
				m_recorder.SetSampleRate(m_AudioOutputs[0].audioDevice->getStreamSampleRate());
			}
		}
		else
			throw (wxString)_("No audio device is selected; neither MIDI input nor sound output will occur!");

		opened_ok = true;

	}
	catch (RtError &e)
	{
		if (logSoundErrors)
		{
			wxString error = wxString::FromAscii(e.getMessage().c_str());
			wxMessageBox(wxString::Format(_("RtAudio error: %s"), error.c_str()), _("Error"), wxOK | wxICON_ERROR, NULL);
		}
	}
	catch (wxString &msg)
	{
		if (logSoundErrors)
		{
			wxMessageBox(msg, _("Error"), wxOK | wxICON_ERROR, NULL);
		}
	}

	if (!opened_ok)
		CloseSound();

	return opened_ok;

}

void GOrgueSound::CloseSound()
{
	StopThreads();
	m_recorder.Close();

	for(int i = m_AudioOutputs.size() - 1; i >= 0; i--)
	{
		if (m_AudioOutputs[i].audioDevice)
		{
			try
			{
				m_AudioOutputs[i].audioDevice->abortStream();
				m_AudioOutputs[i].audioDevice->closeStream();
			}
			catch (RtError &e)
			{
				wxString error = wxString::FromAscii(e.getMessage().c_str());
				wxLogError(_("RtAudio error: %s"), error.c_str());
			}

			try
			{
				delete m_AudioOutputs[i].audioDevice;
			}
			catch (RtError &e)
			{
				wxString error = wxString::FromAscii(e.getMessage().c_str());
				wxLogError(_("RtAudio error: %s"), error.c_str());
			}
			m_AudioOutputs[i].audioDevice = 0;
		}
		if (m_AudioOutputs[i].audioStream)
		{
			Pa_AbortStream(m_AudioOutputs[i].audioStream);
			Pa_CloseStream(m_AudioOutputs[i].audioStream);
			m_AudioOutputs[i].audioStream = 0;
		}

	}

	wxCriticalSectionLocker locker(m_lock);

	if (m_organfile)
	{
		m_organfile->Abort();
		m_organfile = NULL;
	}
	ResetMeters();
	m_AudioOutputs.clear();
}

bool GOrgueSound::ResetSound()
{
	wxBusyCursor busy;
	GrandOrgueFile* organfile = m_organfile;

	if (!m_AudioOutputs.size())
		return false;

	CloseSound();
	if (!OpenSound())
		return false;
	if (organfile)
		PreparePlayback(organfile);

	return true;
}

GOrgueSettings& GOrgueSound::GetSettings()
{
	return m_Settings;
}

GrandOrgueFile* GOrgueSound::GetOrganFile()
{
	return m_organfile;
}

bool GOrgueSound::IsRecording()
{
	return m_recorder.IsOpen();
}

void GOrgueSound::StartRecording(wxString filepath)
{
	m_recorder.Open(filepath);
}

void GOrgueSound::StopRecording()
{
	m_recorder.Close();
}

void GOrgueSound::PreparePlayback(GrandOrgueFile* organfile)
{
	wxCriticalSectionLocker locker(m_lock);

	m_organfile = organfile;
	StopThreads();
	if (organfile)
	{
		m_SoundEngine.Setup(organfile, m_Settings.GetReleaseConcurrency());
		organfile->PreparePlayback(&GetEngine());
		StartThreads();
	}
	else
	{
		m_SoundEngine.Reset();
	}
}

void GOrgueSound::SetLogSoundErrorMessages(bool settingsDialogVisible)
{
	logSoundErrors = settingsDialogVisible;
}

std::map<wxString, GOrgueSound::GO_SOUND_DEV_CONFIG>& GOrgueSound::GetAudioDevices()
{
	return m_audioDevices;
}

const wxString GOrgueSound::GetDefaultAudioDevice()
{
	return defaultAudioDevice;
}

const RtAudioFormat GOrgueSound::GetAudioFormat()
{
	return format;
}

GOrgueMidi& GOrgueSound::GetMidi()
{
	return *m_midi;
}

void GOrgueSound::ResetMeters()
{
	wxCommandEvent event(wxEVT_METERS, 0);
	event.SetInt(0);
	if (wxTheApp->GetTopWindow())
		wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);
}

int GOrgueSound::AudioCallbackLocal(GO_SOUND_OUTPUT* device, float* output_buffer, unsigned int n_frames, double stream_time)
{
	assert(n_frames == m_SamplesPerBuffer);

	wxCriticalSectionLocker locker(m_lock);

	int r = m_SoundEngine.GetSamples
		(output_buffer
		,n_frames
		,stream_time
		,&meter_info
		);

	/* Write data to file if recording is enabled*/
	m_recorder.Write(output_buffer, n_frames * 2);

	/* Update meters */
	meter_counter += n_frames;
	if (meter_counter >= 6144)	// update 44100 / (N / 2) = ~14 times per second
	{

		// polyphony
		int n = (33 * meter_info.current_polyphony) / m_SoundEngine.GetHardPolyphony();
		n <<= 8;
		// right channel
		n |= lrint(32.50000000000001 * meter_info.meter_right);
		n <<= 8;
		// left  channel
		n |= lrint(32.50000000000001 * meter_info.meter_left);

		wxCommandEvent event(wxEVT_METERS, 0);
		event.SetInt(n);
		if (wxTheApp->GetTopWindow())
			wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);

		meter_counter = meter_info.current_polyphony = 0;
		meter_info.meter_left = meter_info.meter_right = 0.0;

	}

	wxCriticalSectionLocker thread_locker(m_thread_lock);
	for(unsigned i = 0; i < m_Threads.size(); i++)
		m_Threads[i]->Wakeup();

	return r;

}

int GOrgueSound::AudioCallback(void *outputBuffer, void *inputBuffer, unsigned int nFrames, double streamTime, RtAudioStreamStatus status, void *userData)
{
	assert(userData);
	GO_SOUND_OUTPUT* sound = (GO_SOUND_OUTPUT*) userData;
	if (nFrames & (BLOCKS_PER_FRAME - 1))
	{
		wxString error;
		error.Printf
			(_("Audio callback of %u blocks requested. Must be divisible by %u.")
			,nFrames
			,BLOCKS_PER_FRAME
			);
		throw error;
	}

	return sound->sound->AudioCallbackLocal(sound, static_cast<float*>(outputBuffer), nFrames, streamTime);
}

int GOrgueSound::PaAudioCallback (const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
	assert(userData);
	GO_SOUND_OUTPUT* sound = (GO_SOUND_OUTPUT*) userData;
	if (frameCount & (BLOCKS_PER_FRAME - 1))
	{
		wxString error;
		error.Printf
			(_("Audio callback of %u blocks requested. Must be divisible by %u.")
			,frameCount
			,BLOCKS_PER_FRAME
			);
		throw error;
	}

	int ret = sound->sound->AudioCallbackLocal(sound, static_cast<float*>(output), frameCount, 0*timeInfo->currentTime);

	if (!ret)
		return paContinue;
	else
		return paAbort;
}

GOSoundEngine& GOrgueSound::GetEngine()
{
	return m_SoundEngine;
}
