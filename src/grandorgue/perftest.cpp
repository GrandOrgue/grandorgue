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

#include <iostream>
#include "GOSoundEngine.h"
#include "GOSoundProviderWave.h"
#include "GOrgueSettings.h"
#include "GOrgueWindchest.h"
#include "GrandOrgueFile.h"
#include "ptrvector.h"

class TestApp : public wxApp
{
public:
	TestApp();
	bool OnInit();
	int OnRun();
	void RunTest(unsigned bits_per_sample, bool compress, unsigned sample_instances, unsigned sample_rate, unsigned interpolation, unsigned seconds, unsigned samples_per_frame);
};

DECLARE_APP(TestApp)
IMPLEMENT_APP_CONSOLE(TestApp)

TestApp::TestApp()
{
}

void TestApp::RunTest(unsigned bits_per_sample, bool compress, unsigned sample_instances, unsigned sample_rate, unsigned interpolation, unsigned seconds, unsigned samples_per_frame)
{
	try
	{
		GOrgueSettings settings;
		GrandOrgueFile* organfile = new GrandOrgueFile(NULL, settings);
		organfile->AddWindchest(new GOrgueWindchest(organfile));
		GOSoundEngine* engine = new GOSoundEngine();

		ptr_vector<GOSoundProvider> pipes;
		for(unsigned i = 0; i < sample_instances; i++)
		{
			GOSoundProviderWave* w = new GOSoundProviderWave(organfile->GetMemoryPool());
			w->SetAmplitude(102, 0);
 			std::vector<release_load_info> release;
 			std::vector<attack_load_info> attack;
			attack_load_info ainfo;
			ainfo.cue_point = -1;
			ainfo.filename = wxString::Format(wxT("%02d.wav"), i % 3);
			ainfo.sample_group = -1;
			ainfo.load_release = true;
			ainfo.percussive = false;
			ainfo.max_playback_time = -1;
			attack.push_back(ainfo);
			w->LoadFromFile(attack, release, argv[1], bits_per_sample, 2, compress, LOOP_LOAD_ALL, 1, 1);
			pipes.push_back(w);
		}
		engine->SetVolume(10);
		engine->SetSampleRate(sample_rate);
		engine->SetPolyphonyLimiting(false);
		engine->SetHardPolyphony(10000);
		engine->SetScaledReleases(true);
		engine->SetInterpolationType(interpolation);
		engine->Setup(organfile);

		std::vector<SAMPLER_HANDLE> handles;
		METER_INFO info;
		float output_buffer[GO_SOUND_BUFFER_SIZE];

		for(unsigned i = 0; i < pipes.size(); i++)
		{
			SAMPLER_HANDLE handle = engine->StartSample(pipes[i], 1, 0);
			if (handle)
				handles.push_back(handle);
		}

		wxMilliClock_t start = wxGetLocalTimeMillis();
		unsigned blocks = seconds * engine->GetSampleRate() / samples_per_frame;
		for(unsigned i = 0; i < blocks; i++)
		{
			engine->GetSamples(output_buffer, samples_per_frame, 0, &info);
		}
		wxMilliClock_t end = wxGetLocalTimeMillis();
		wxMilliClock_t diff = end - start;

		float playback_time = blocks * (double)samples_per_frame / engine->GetSampleRate();
		wxLogError(wxT("%d sampler, %f seconds, %d bits, %d, %s, %s, %d block: %d ms cpu time, limit: %f"), pipes.size(), playback_time, 
			   bits_per_sample, sample_rate, compress ? wxT("Y") : wxT("N"), interpolation == 0 ? wxT("Linear") : wxT("Polyphase"), 
			   samples_per_frame, diff.ToLong(), playback_time * 1000.0 * pipes.size() / diff.ToLong());

		pipes.clear();
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
	RunTest(8, true, 300, 44100, 0, 10, 128);
	RunTest(8, false, 300, 44100, 0, 10, 128);
	RunTest(16, true, 300, 44100, 0, 10, 128);
	RunTest(16, false, 300, 44100, 0, 10, 128);
	RunTest(24, true, 300, 44100, 0, 10, 128);
	RunTest(24, false, 300, 44100, 0, 10, 128);
	RunTest(8, true, 300, 48000, 1, 10, 1024);
	RunTest(8, false, 300, 48000, 1, 10, 1024);
	RunTest(16, true, 300, 48000, 1, 10, 1024);
	RunTest(16, false, 300, 48000, 1, 10, 1024);
	RunTest(24, true, 300, 48000, 1, 10, 1024);
	RunTest(24, false, 300, 48000, 1, 10, 1024);
	RunTest(8, true, 300, 48000, 0, 10, 1024);
	RunTest(8, false, 300, 48000, 0, 10, 1024);
	RunTest(16, true, 300, 48000, 0, 10, 1024);
	RunTest(16, false, 300, 48000, 0, 10, 1024);
	RunTest(24, true, 300, 48000, 0, 10, 1024);
	RunTest(24, false, 300, 48000, 0, 10, 1024);
	return 0;
}
