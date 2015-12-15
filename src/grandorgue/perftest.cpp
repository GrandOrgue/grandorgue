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

#include "ptrvector.h"
#include "GOSoundEngine.h"
#include "GOSoundProviderWave.h"
#include "GOSoundRecorder.h"
#include "GOrgueSettings.h"
#include "GOrgueWindchest.h"
#include "GrandOrgueFile.h"
#include <wx/app.h>
#include <wx/image.h>
#include <wx/stopwatch.h>
#include <iostream>

#ifdef __linux__
#include <sys/time.h>
#include <sys/resource.h>
#endif

class TestApp : public wxApp
{
        wxMilliClock_t getCPUTime();

public:
	TestApp();
	bool OnInit();
	int OnRun();
	void RunTest(unsigned bits_per_sample, bool compress, unsigned sample_instances, unsigned sample_rate, unsigned interpolation, unsigned samples_per_frame);
};

DECLARE_APP(TestApp)
IMPLEMENT_APP_CONSOLE(TestApp)

TestApp::TestApp()
{
}

wxMilliClock_t TestApp::getCPUTime()
{
#ifdef __linux__
	struct rusage usage;
	if (!getrusage(RUSAGE_SELF, &usage))
	{
		wxMilliClock_t t = usage.ru_utime.tv_sec;
		t *= 1000; 
		t += usage.ru_utime.tv_usec / 1000;
		return t;
	}
#endif
	return wxGetLocalTimeMillis();
}

void TestApp::RunTest(unsigned bits_per_sample, bool compress, unsigned sample_instances, unsigned sample_rate, unsigned interpolation, unsigned samples_per_frame)
{
	try
	{
		GOrgueSettings settings(wxT("perftest"));
		GrandOrgueFile* organfile = new GrandOrgueFile(NULL, settings);
		organfile->SetODFPath(argv[1]);
		organfile->AddWindchest(new GOrgueWindchest(organfile));
		GOSoundEngine* engine = new GOSoundEngine();
		GOSoundRecorder recorder;

		try
		{
			ptr_vector<GOSoundProvider> pipes;
			for(unsigned i = 0; i < sample_instances; i++)
			{
				GOSoundProviderWave* w = new GOSoundProviderWave(organfile->GetMemoryPool());
				w->SetAmplitude(102, 0);
				std::vector<release_load_info> release;
				std::vector<attack_load_info> attack;
				attack_load_info ainfo;
				ainfo.filename.Assign(wxString::Format(wxT("%02d.wav"), i % 3), organfile);
				ainfo.sample_group = -1;
				ainfo.load_release = true;
				ainfo.percussive = false;
				ainfo.min_attack_velocity = 0;
				ainfo.max_playback_time = -1;
				ainfo.attack_start = 0;
				ainfo.cue_point = -1;
				ainfo.release_end = -1;
				ainfo.loops.clear();
				attack.push_back(ainfo);
				w->LoadFromFile(attack, release, bits_per_sample, 2, compress, LOOP_LOAD_ALL, 1, 1, -1, 0);
				pipes.push_back(w);
			}
			engine->SetSamplesPerBuffer(samples_per_frame);
			engine->SetVolume(10);
			engine->SetSampleRate(sample_rate);
			engine->SetPolyphonyLimiting(false);
			engine->SetHardPolyphony(10000);
			engine->SetScaledReleases(true);
			engine->SetAudioGroupCount(1);
			engine->SetInterpolationType(interpolation);

			std::vector<GOAudioOutputConfiguration> engine_config;
			engine_config.resize(1);
			engine_config[0].channels = 2;
			engine_config[0].scale_factors.resize(2);
			engine_config[0].scale_factors[0].resize(2);
			engine_config[0].scale_factors[0][0] = 0;
			engine_config[0].scale_factors[0][1] = -121;
			engine_config[0].scale_factors[1].resize(2);
			engine_config[0].scale_factors[1][0] = -121;
			engine_config[0].scale_factors[1][1] = 0;
			engine->SetAudioOutput(engine_config);
			engine->SetAudioRecorder(&recorder, false);

			engine->Setup(organfile);

			std::vector<GO_SAMPLER*> handles;
			float output_buffer[samples_per_frame * 2];

			for(unsigned i = 0; i < pipes.size(); i++)
			{
				GO_SAMPLER* handle = engine->StartSample(pipes[i], 1, 0, 127, 0);
				if (handle)
					handles.push_back(handle);
			}

			wxMilliClock_t start = getCPUTime();
			wxMilliClock_t end;
			wxMilliClock_t diff;
			unsigned batch_size = 1 * engine->GetSampleRate() / samples_per_frame;
			unsigned blocks = 0;
			do
			{
				for(unsigned i = 0; i < batch_size; i++)
				{
					engine->GetAudioOutput(output_buffer, samples_per_frame, 0);
					engine->NextPeriod();
					blocks++;
				}
				end = getCPUTime();
				diff = end - start;
			}
			while(diff < 30000);
			
			float playback_time = blocks * (double)samples_per_frame / engine->GetSampleRate();
			wxLogError(wxT("%d sampler, %f seconds, %d bits, %d, %s, %s, %d block: %d ms cpu time, limit: %f"), pipes.size(), playback_time, 
				   bits_per_sample, sample_rate, compress ? wxT("Y") : wxT("N"), interpolation == 0 ? wxT("Linear") : wxT("Polyphase"), 
				   samples_per_frame, diff.ToLong(), playback_time * 1000.0 * pipes.size() / diff.ToLong());

			pipes.clear();
		}
		catch(wxString msg)
		{
			wxLogError(wxT("Error: %s"), msg.c_str());
		}

		delete engine;
		delete organfile;
	}
	catch(wxString msg)
	{
		wxLogError(wxT("Error: %s"), msg.c_str());
	}
}

bool TestApp::OnInit()
{
	wxLog *logger=new wxLogStream(&std::cout);
	wxLog::SetActiveTarget(logger);
	wxImage::AddHandler(new wxJPEGHandler);
	wxImage::AddHandler(new wxGIFHandler);
	wxImage::AddHandler(new wxPNGHandler);
	wxImage::AddHandler(new wxBMPHandler);
	wxImage::AddHandler(new wxICOHandler);

	if (argc != 2)
	{
		wxLogError(wxT("Usage: perftest test-data-directory"));
		return false;
	}
	return true;
}

int TestApp::OnRun()
{
	const int samplers = 300;
	RunTest(8, true, samplers, 44100, 0, 128);
	RunTest(8, false, samplers, 44100, 0, 128);
	RunTest(16, true, samplers, 44100, 0, 128);
	RunTest(16, false, samplers, 44100, 0, 128);
	RunTest(24, true, samplers, 44100, 0, 128);
	RunTest(24, false, samplers, 44100, 0, 128);
	RunTest(8, true, samplers, 48000, 1, 1024);
	RunTest(8, false, samplers, 48000, 1, 1024);
	RunTest(16, true, samplers, 48000, 1, 1024);
	RunTest(16, false, samplers, 48000, 1, 1024);
	RunTest(24, true, samplers, 48000, 1, 1024);
	RunTest(24, false, samplers, 48000, 1, 1024);
	RunTest(8, true, samplers, 48000, 0, 1024);
	RunTest(8, false, samplers, 48000, 0, 1024);
	RunTest(16, true, samplers, 48000, 0, 1024);
	RunTest(16, false, samplers, 48000, 0, 1024);
	RunTest(24, true, samplers, 48000, 0, 1024);
	RunTest(24, false, samplers, 48000, 0, 1024);
	return 0;
}
