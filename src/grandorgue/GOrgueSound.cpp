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
 */

#include "GOrgueSound.h"
#include "GOSoundThread.h"
#include "GrandOrgueFile.h"
#include "GOrgueRtHelpers.h"
#include "GOLock.h"
#include "GOrgueMidi.h"
#include "GOrgueSettings.h"
#include "GOrgueEvent.h"

#define DELETE_AND_NULL(x) do { if (x) { delete x; x = NULL; } } while (0)

GOrgueSound* g_sound = 0;

GOrgueSound::GOrgueSound(GOrgueSettings& settings) :
	logSoundErrors(true),
	m_audioDevices(),
	m_AudioOutputs(),
	m_SamplesPerBuffer(0),
	meter_counter(0),
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
				cfg.channels = info.outputChannels;
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
		cfg.channels = info->maxOutputChannels;
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

	unsigned n_cpus = m_Settings.GetConcurrency();
	unsigned tasks = GetEngine().GetGroupCount();

	GOMutexLocker thread_locker(m_thread_lock);
	for(unsigned i = 0; i < n_cpus; i++)
	{
		if (i >= tasks)
			break;
		m_Threads.push_back(new GOSoundThread(&GetEngine(), m_SamplesPerBuffer));
	}

	for(unsigned i = 0; i < m_Threads.size(); i++)
		m_Threads[i]->Run();
}

void GOrgueSound::StopThreads()
{
	for(unsigned i = 0; i < m_Threads.size(); i++)
		m_Threads[i]->Delete();

	GOMutexLocker thread_locker(m_thread_lock);
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

	unsigned audio_group_count = m_Settings.GetAudioGroups().size();
	std::vector<GOAudioDeviceConfig> audio_config = m_Settings.GetAudioDeviceConfig();
	std::vector<GOAudioOutputConfiguration> engine_config;

	m_AudioOutputs.resize(audio_config.size());
	for(unsigned i = 0; i < m_AudioOutputs.size(); i++)
	{
		m_AudioOutputs[i].name = audio_config[i].name;
		m_AudioOutputs[i].audioStream = 0;
		m_AudioOutputs[i].audioDevice = 0;
		m_AudioOutputs[i].channels = audio_config[i].channels;
	}
	engine_config.resize(audio_config.size());
	for(unsigned i = 0; i < engine_config.size(); i++)
	{
		engine_config[i].channels = audio_config[i].channels;
		engine_config[i].scale_factors.resize(engine_config[i].channels);
		for(unsigned j = 0; j <  engine_config[i].channels; j++)
		{
			engine_config[i].scale_factors[j].resize(audio_group_count * 2);
			for(unsigned k = 0; k < audio_group_count * 2; k++)
				engine_config[i].scale_factors[j][k] = -121;

			if (j >= audio_config[i].scale_factors.size())
				continue;
			for(unsigned k = 0; k < audio_config[i].scale_factors[j].size(); k++)
			{
				int id = m_Settings.GetStrictAudioGroupId(audio_config[i].scale_factors[j][k].name);
				if (id == -1)
					continue;
				if (audio_config[i].scale_factors[j][k].left >= -120)
					engine_config[i].scale_factors[j][id * 2] = audio_config[i].scale_factors[j][k].left;
				if (audio_config[i].scale_factors[j][k].right >= -120)
					engine_config[i].scale_factors[j][id * 2 + 1] = audio_config[i].scale_factors[j][k].right;
			}
		}
	}

	m_SamplesPerBuffer = m_Settings.GetSamplesPerBuffer();
	m_SoundEngine.SetPolyphonyLimiting(m_Settings.GetManagePolyphony());
	m_SoundEngine.SetHardPolyphony(m_Settings.GetPolyphonyLimit());
	m_SoundEngine.SetVolume(m_Settings.GetVolume());
	m_SoundEngine.SetScaledReleases(m_Settings.GetScaleRelease());
	m_SoundEngine.SetRandomizeSpeaking(m_Settings.GetRandomizeSpeaking());
	m_SoundEngine.SetInterpolationType(m_Settings.GetInterpolationType());
	m_SoundEngine.SetAudioGroupCount(audio_group_count);
	unsigned sample_rate = m_Settings.GetSampleRate();
	m_recorder.SetBytesPerSample(m_Settings.GetWaveFormatBytesPerSample());
	GetEngine().SetSampleRate(sample_rate);
	m_recorder.SetSampleRate(sample_rate);
	m_SoundEngine.SetAudioOutput(engine_config);

	PreparePlayback(NULL);

	try
	{
		OpenMidi();
		InitStreams();
		OpenStreams();
		StartStreams();
		opened_ok = true;

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

void GOrgueSound::InitStreams()
{
	for(unsigned i = 0; i < m_AudioOutputs.size(); i++)
	{
		m_AudioOutputs[i].sound = this;
		m_AudioOutputs[i].index = i;

		if (m_AudioOutputs[i].name == wxEmptyString)
			m_AudioOutputs[i].name = defaultAudioDevice;

		std::map<wxString, GO_SOUND_DEV_CONFIG>::iterator it;
		it = m_audioDevices.find(m_AudioOutputs[i].name);
		if (it == m_audioDevices.end())
			throw wxString::Format(_("Output device %s not found - no sound output will occure"), m_AudioOutputs[i].name.c_str());

		m_AudioOutputs[i].rt_api = it->second.rt_api;
		m_AudioOutputs[i].rt_api_subindex = it->second.rt_api_subindex;
		m_AudioOutputs[i].try_latency = m_Settings.GetAudioDeviceLatency(m_AudioOutputs[i].name);
	}
}

void GOrgueSound::OpenStreams()
{
	for(unsigned i = 0; i < m_AudioOutputs.size(); i++)
	{
		if (m_AudioOutputs[i].rt_api == RTAPI_PORTAUDIO)
		{
			PaStreamParameters stream_parameters;
			stream_parameters.device = m_AudioOutputs[i].rt_api_subindex;
			stream_parameters.channelCount = m_AudioOutputs[i].channels;
			stream_parameters.sampleFormat = paFloat32;
			stream_parameters.suggestedLatency = m_AudioOutputs[i].try_latency / 1000.0;
			stream_parameters.hostApiSpecificStreamInfo = NULL;

			PaError error;
			error = Pa_OpenStream(&m_AudioOutputs[i].audioStream, NULL, &stream_parameters, GetEngine().GetSampleRate(), m_SamplesPerBuffer,
					      paNoFlag, &GOrgueSound:: PaAudioCallback, &m_AudioOutputs[i]);
			if (error != paNoError)
				throw wxString::Format(_("Open of the audio stream for %s failed: %s"), m_AudioOutputs[i].name.c_str(), wxGetTranslation(wxString::FromAscii(Pa_GetErrorText(error))));
		}
		else
		{
			try
			{
				m_AudioOutputs[i].audioDevice = new RtAudio(m_AudioOutputs[i].rt_api);

				RtAudio::StreamParameters aOutputParam;
				aOutputParam.deviceId = m_AudioOutputs[i].rt_api_subindex;
				aOutputParam.nChannels = m_AudioOutputs[i].channels;

				RtAudio::StreamOptions aOptions;
				aOptions.flags = RTAUDIO_MINIMIZE_LATENCY;
				aOptions.numberOfBuffers = 0;

				unsigned samples_per_buffer = m_SamplesPerBuffer;
				m_AudioOutputs[i].audioDevice->openStream(&aOutputParam, NULL, RTAUDIO_FLOAT32, GetEngine().GetSampleRate(), &samples_per_buffer,
									  &GOrgueSound::AudioCallback, &m_AudioOutputs[i], &aOptions);
				m_AudioOutputs[i].nb_buffers = aOptions.numberOfBuffers;
				if (samples_per_buffer != m_SamplesPerBuffer)
				{
					if (i == 0)
					{
						m_SamplesPerBuffer = samples_per_buffer;
						m_Settings.SetSamplesPerBuffer(m_SamplesPerBuffer);
					}
					else
						throw wxString::Format(_("Device %s wants a different frame count: %d"), m_AudioOutputs[i].name.c_str(), samples_per_buffer);
				}
			}
			catch (RtError &e)
			{
				wxString error = wxString::FromAscii(e.getMessage().c_str());
				throw wxString::Format(_("RtAudio error: %s"), error.c_str());
			}
		}
	}
}

void GOrgueSound::StartStreams()
{
	if (m_SamplesPerBuffer & (BLOCKS_PER_FRAME - 1))
		throw wxString::Format(_("Audio engine wants frame size of %u blocks which is indivisible by %u."), m_SamplesPerBuffer, BLOCKS_PER_FRAME);

	if (m_SamplesPerBuffer > MAX_FRAME_SIZE)
		throw wxString::Format(_("Cannot use buffer size above %d samples; unacceptable quantization would occur."), MAX_FRAME_SIZE);

	for(unsigned i = 0; i < m_AudioOutputs.size(); i++)
	{
		double latency = 0;

		if (m_AudioOutputs[i].audioDevice)
		{
			try
			{
				m_AudioOutputs[i].audioDevice->startStream();
				double actual_latency = m_AudioOutputs[i].audioDevice->getStreamLatency();

				/* getStreamLatency returns zero if not supported by the API, in which
				 * case we will make a best guess.
				 */
				if (actual_latency == 0)
					actual_latency = m_SamplesPerBuffer * m_AudioOutputs[i].nb_buffers;

				latency = actual_latency / GetEngine().GetSampleRate();
			}
			catch (RtError &e)
			{
				wxString error = wxString::FromAscii(e.getMessage().c_str());
				throw wxString::Format(_("RtAudio error: %s"), error.c_str());
			}

			if (m_AudioOutputs[i].audioDevice->getStreamSampleRate() != GetEngine().GetSampleRate())
				throw wxString::Format(_("Sample rate of device %s changed"), m_AudioOutputs[i].name.c_str());
		}
		if (m_AudioOutputs[i].audioStream)
		{
			PaError error;
			error = Pa_StartStream(m_AudioOutputs[i].audioStream);
			if (error != paNoError)
				throw wxString::Format(_("Start of audio stream of %s failed: %s"), m_AudioOutputs[i].name.c_str(),
						       wxGetTranslation(wxString::FromAscii(Pa_GetErrorText(error))));

			const struct PaStreamInfo* info = Pa_GetStreamInfo(m_AudioOutputs[i].audioStream);
			latency = info->outputLatency;
		}
		if (latency < m_SamplesPerBuffer / GetEngine().GetSampleRate())
			latency = m_SamplesPerBuffer / GetEngine().GetSampleRate();
		if (latency < 2 * m_SamplesPerBuffer / GetEngine().GetSampleRate())
			latency += m_SamplesPerBuffer / GetEngine().GetSampleRate();
		m_Settings.SetAudioDeviceActualLatency(m_AudioOutputs[i].name, (int)(latency * 1000));
	}

	for(unsigned i = 0; i < m_AudioOutputs.size(); i++)
	{
		GOMutexLocker dev_lock(m_AudioOutputs[i].mutex);
		m_AudioOutputs[i].wait = true;
	}
}

void GOrgueSound::CloseSound()
{
	StopThreads();
	m_recorder.Close();

	for(unsigned i = 0; i < m_AudioOutputs.size(); i++)
	{
		m_AudioOutputs[i].wait = false;
		m_AudioOutputs[i].condition.Signal();
	}

	for(unsigned i = 1; i < m_AudioOutputs.size(); i++)
	{
		GOMutexLocker dev_lock(m_AudioOutputs[i].mutex);
		m_AudioOutputs[i].condition.Signal();
	}

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

	{
		GOMutexLocker locker(m_lock);
		GOMultiMutexLocker multi;
		for(unsigned i = 0; i < m_AudioOutputs.size(); i++)
			multi.Add(m_AudioOutputs[i].mutex);

		if (m_organfile)
		{
			m_organfile->Abort();
			m_organfile = NULL;
		}
		ResetMeters();
	}
	m_AudioOutputs.clear();
}

bool GOrgueSound::ResetSound(bool force)
{
	wxBusyCursor busy;
	GrandOrgueFile* organfile = m_organfile;

	if (!m_AudioOutputs.size() && !force)
		return false;

	CloseSound();
	if (!OpenSound())
	{
		m_organfile = organfile;
		return false;
	}
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
	GOMutexLocker locker(m_lock);
	GOMultiMutexLocker multi;
	for(unsigned i = 0; i < m_AudioOutputs.size(); i++)
		multi.Add(m_AudioOutputs[i].mutex);

	m_organfile = organfile;
	StopThreads();
	if (organfile)
	{
		m_SoundEngine.Setup(organfile, m_Settings.GetReleaseConcurrency());
		organfile->PreparePlayback(&GetEngine(), &GetMidi());
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

const std::map<wxString, GOrgueSound::GO_SOUND_DEV_CONFIG>& GOrgueSound::GetAudioDevices()
{
	return m_audioDevices;
}

const wxString GOrgueSound::GetDefaultAudioDevice()
{
	return defaultAudioDevice;
}

GOrgueMidi& GOrgueSound::GetMidi()
{
	return *m_midi;
}

void GOrgueSound::ResetMeters()
{
	wxCommandEvent event(wxEVT_METERS, 0);
	event.SetInt(0xf0000000);
	if (wxTheApp->GetTopWindow())
		wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);
}

int GOrgueSound::AudioCallbackLocal(GO_SOUND_OUTPUT* device, float* output_buffer, unsigned int n_frames, double stream_time)
{
	assert(n_frames == m_SamplesPerBuffer);

	GOMutexLocker locker(device->mutex);

	int r;
	r = m_SoundEngine.GetAudioOutput(output_buffer, n_frames, device->index);

	if (device->index == 0)
	{
		float buffer[GO_SOUND_BUFFER_SIZE];
		r = m_SoundEngine.GetSamples
			(buffer
			 ,n_frames
			 ,stream_time
			 ,&meter_info
			 );

		/* Write data to file if recording is enabled*/
		m_recorder.Write(buffer, n_frames * 2);

		/* Update meters */
		meter_counter += n_frames;
		if (meter_counter >= 6144)	// update 44100 / (N / 2) = ~14 times per second
		{

			// polyphony
			int n = 0xff & ((33 * meter_info.current_polyphony) / m_SoundEngine.GetHardPolyphony());
			n <<= 8;
			// right channel
			n |= 0xff & (lrint(32.50000000000001 * meter_info.meter_right));
			n <<= 8;
			// left  channel
			n |= 0xff & (lrint(32.50000000000001 * meter_info.meter_left));

			wxCommandEvent event(wxEVT_METERS, 0);
			event.SetInt(n);
			if (wxTheApp->GetTopWindow())
				wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);

			meter_counter = meter_info.current_polyphony = 0;
			meter_info.meter_left = meter_info.meter_right = 0.0;
		}

		GOMutexLocker thread_locker(m_thread_lock);
		for(unsigned i = 0; i < m_Threads.size(); i++)
			m_Threads[i]->Wakeup();

		for(unsigned i = 1; i < m_AudioOutputs.size(); i++)
		{
			GOMutexLocker dev_lock(m_AudioOutputs[i].mutex);
			if (m_AudioOutputs[i].wait)
			{
				if (m_AudioOutputs[i].waiting)
				{
					dev_lock.Unlock();
					m_AudioOutputs[i].condition.Signal();
				}
				else
				{
					m_AudioOutputs[i].waiting = true;
					m_AudioOutputs[i].condition.Wait();
					m_AudioOutputs[i].waiting = false;
				}
			}
		}
	}
	else
	{
		if (device->wait)
		{
			if (device->waiting)
			{
				locker.Unlock();
				device->condition.Signal();
			}
			else
			{
				device->waiting = true;
				device->condition.Wait();
				device->waiting = false;
			}
		}
	}

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
