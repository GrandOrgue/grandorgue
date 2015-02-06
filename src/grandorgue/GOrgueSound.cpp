/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2015 GrandOrgue contributors (see AUTHORS)
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

#include "GOSoundDefs.h"
#include "GOSoundThread.h"
#include "GOrgueEvent.h"
#include "GOrgueMidi.h"
#include "GOrgueSettings.h"
#include "GOrgueSoundPort.h"
#include "GrandOrgueFile.h"
#include "portaudio.h"
#include <wx/app.h>
#include <wx/intl.h>
#include <wx/window.h>

#define DELETE_AND_NULL(x) do { if (x) { delete x; x = NULL; } } while (0)

GOrgueSound::GOrgueSound(GOrgueSettings& settings) :
	logSoundErrors(true),
	m_AudioOutputs(),
	m_SamplesPerBuffer(0),
	meter_counter(0),
	m_defaultAudioDevice(),
	m_organfile(0),
	m_Settings(settings)
{
	m_midi = new GOrgueMidi(m_Settings);

	Pa_Initialize();
	GetAudioDevices();

	m_SoundEngine.SetVolume(m_Settings.GetVolume());
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

	GOMutexLocker thread_locker(m_thread_lock);
	for(unsigned i = 0; i < n_cpus; i++)
		m_Threads.push_back(new GOSoundThread(&GetEngine().GetScheduler()));

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
	m_midi->Open();
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
		m_AudioOutputs[i].port = NULL;
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
	m_SoundEngine.SetSamplesPerBuffer(m_SamplesPerBuffer);
	m_SoundEngine.SetPolyphonyLimiting(m_Settings.GetManagePolyphony());
	m_SoundEngine.SetHardPolyphony(m_Settings.GetPolyphonyLimit());
	m_SoundEngine.SetScaledReleases(m_Settings.GetScaleRelease());
	m_SoundEngine.SetRandomizeSpeaking(m_Settings.GetRandomizeSpeaking());
	m_SoundEngine.SetInterpolationType(m_Settings.GetInterpolationType());
	m_SoundEngine.SetAudioGroupCount(audio_group_count);
	unsigned sample_rate = m_Settings.GetSampleRate();
	m_AudioRecorder.SetBytesPerSample(m_Settings.GetWaveFormatBytesPerSample());
	GetEngine().SetSampleRate(sample_rate);
	m_AudioRecorder.SetSampleRate(sample_rate);
	m_SoundEngine.SetAudioOutput(engine_config);
	m_SoundEngine.SetupReverb(m_Settings);
	m_SoundEngine.SetAudioRecorder(&m_AudioRecorder, m_Settings.GetRecordDownmix());

	if (m_organfile)
		m_SoundEngine.Setup(m_organfile, m_Settings.GetReleaseConcurrency());
	else
		m_SoundEngine.ClearSetup();

	try
	{
		for(unsigned i = 0; i < m_AudioOutputs.size(); i++)
		{
			wxString name = audio_config[i].name;
			if (name == wxEmptyString)
				name = m_defaultAudioDevice;

			m_AudioOutputs[i].port = GOrgueSoundPort::create(this, name);
			if (!m_AudioOutputs[i].port)
				throw wxString::Format(_("Output device %s not found - no sound output will occure"), name.c_str());

			m_AudioOutputs[i].port->Init(audio_config[i].channels, GetEngine().GetSampleRate(), m_SamplesPerBuffer, i);
		}

		OpenMidi();
		StartStreams();
		StartThreads();
		opened_ok = true;

		if (m_organfile)
			m_organfile->PreparePlayback(&GetEngine(), &GetMidi());
	}
	catch (wxString &msg)
	{
		if (logSoundErrors)
		{
			GOMessageBox(msg, _("Error"), wxOK | wxICON_ERROR, NULL);
		}
	}

	if (!opened_ok)
		CloseSound();

	return opened_ok;

}

void GOrgueSound::StartStreams()
{
	for(unsigned i = 0; i < m_AudioOutputs.size(); i++)
		m_AudioOutputs[i].port->Open();

	if (m_SamplesPerBuffer > MAX_FRAME_SIZE)
		throw wxString::Format(_("Cannot use buffer size above %d samples; unacceptable quantization would occur."), MAX_FRAME_SIZE);

	for(unsigned i = 0; i < m_AudioOutputs.size(); i++)
		m_AudioOutputs[i].port->StartStream();

	for(unsigned i = 0; i < m_AudioOutputs.size(); i++)
	{
		GOMutexLocker dev_lock(m_AudioOutputs[i].mutex);
		m_AudioOutputs[i].wait = true;
	}
}

void GOrgueSound::CloseSound()
{
	StopMidiPlaying();
	StopThreads();
	StopAudioRecording();
	StopMidiRecording();

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
		if (m_AudioOutputs[i].port)
		{
			m_AudioOutputs[i].port->Close();
			delete m_AudioOutputs[i].port;
			m_AudioOutputs[i].port = 0;
		}
	}

	if (m_organfile)
		m_organfile->Abort();
	ResetMeters();
	m_AudioOutputs.clear();
}

bool GOrgueSound::ResetSound(bool force)
{
	wxBusyCursor busy;
	if (!m_AudioOutputs.size() && !force)
		return false;

	CloseSound();
	if (!OpenSound())
		return false;

	return true;
}

void GOrgueSound::AssignOrganFile(GrandOrgueFile* organfile)
{
	if (organfile == m_organfile)
		return;

	GOMutexLocker locker(m_lock);
	GOMultiMutexLocker multi;
	for(unsigned i = 0; i < m_AudioOutputs.size(); i++)
		multi.Add(m_AudioOutputs[i].mutex);
	
	if (m_organfile)
	{
		m_organfile->Abort();
		m_SoundEngine.ClearSetup();
	}

	m_organfile = organfile;
	m_midi->GetMidiRecorder().SetOrganFile(m_organfile);

	if (m_organfile && m_AudioOutputs.size())
	{
		m_SoundEngine.Setup(organfile, m_Settings.GetReleaseConcurrency());
		m_organfile->PreparePlayback(&GetEngine(), &GetMidi());
	}
}

GOrgueSettings& GOrgueSound::GetSettings()
{
	return m_Settings;
}

GrandOrgueFile* GOrgueSound::GetOrganFile()
{
	return m_organfile;
}

bool GOrgueSound::IsAudioRecording()
{
	return m_AudioRecorder.IsOpen();
}

void GOrgueSound::StartAudioRecording(wxString filepath)
{
	m_AudioRecorder.Open(filepath);
}

void GOrgueSound::StopAudioRecording()
{
	m_AudioRecorder.Close();
}

bool GOrgueSound::IsMidiRecording()
{
	return m_midi->GetMidiRecorder().IsRecording();
}

void GOrgueSound::StartMidiRecording(wxString filename)
{
	m_midi->GetMidiRecorder().StartRecording(filename);
}

void GOrgueSound::StopMidiRecording()
{
	m_midi->GetMidiRecorder().StopRecording();
}

bool GOrgueSound::IsMidiPlaying()
{
	return m_midi->GetMidiPlayer().IsPlaying();
}

void GOrgueSound::StartMidiPlaying(wxString filename)
{
	m_midi->GetMidiPlayer().Load(filename, m_organfile ? (m_organfile->GetODFManualCount() - 1): 2, m_organfile ? (m_organfile->GetFirstManualIndex() == 0) : true);
	m_midi->GetMidiPlayer().Play();
}

void GOrgueSound::StopMidiPlaying()
{
	m_midi->GetMidiPlayer().StopPlayer();
}

void GOrgueSound::SetLogSoundErrorMessages(bool settingsDialogVisible)
{
	logSoundErrors = settingsDialogVisible;
}

std::vector<GOrgueSoundDevInfo> GOrgueSound::GetAudioDevices()
{
	std::vector<GOrgueSoundDevInfo> list = GOrgueSoundPort::getDeviceList();
	for(unsigned i = 0; i < list.size(); i++)
		if (list[i].isDefault)
		{
			m_defaultAudioDevice = list[i].name;
			break;
		}
	return list;
}

const wxString GOrgueSound::GetDefaultAudioDevice()
{
	return m_defaultAudioDevice;
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

bool GOrgueSound::AudioCallback(unsigned dev_index, float* output_buffer, unsigned int n_frames)
{
	if (n_frames != m_SamplesPerBuffer)
	{
		wxLogError(_("No sound output will happen. Samples per buffer has been changed by the sound driver to %d"), n_frames);
		return 1;
	}
	GO_SOUND_OUTPUT* device = &m_AudioOutputs[dev_index];
	GOMutexLocker locker(device->mutex);

	m_SoundEngine.GetAudioOutput(output_buffer, n_frames, dev_index);

	if (dev_index == 0)
	{
		m_SoundEngine.NextPeriod();

		/* Update meters */
		meter_counter += n_frames;
		if (meter_counter >= 6144)	// update 44100 / (N / 2) = ~14 times per second
		{
			METER_INFO meter_info;
			m_SoundEngine.GetMeterInfo(&meter_info);

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

	return true;
}

GOSoundEngine& GOrgueSound::GetEngine()
{
	return m_SoundEngine;
}
