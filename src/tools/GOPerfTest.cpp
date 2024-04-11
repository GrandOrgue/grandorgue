/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include <iostream>

#include <wx/app.h>
#include <wx/filename.h>
#include <wx/image.h>
#include <wx/stopwatch.h>

#include "ptrvector.h"

#include "config/GOConfig.h"
#include "model/GOWindchest.h"
#include "sound/GOSoundEngine.h"
#include "sound/GOSoundProviderWave.h"
#include "sound/GOSoundRecorder.h"

#include "GOOrganController.h"
#include "GOStdPath.h"

#ifdef __linux__
#include <sys/resource.h>
#include <sys/time.h>
#endif

class GOPerfTestApp : public wxApp {
  wxMilliClock_t getCPUTime();

public:
  GOPerfTestApp();
  bool OnInit();
  int OnRun();
  void RunTest(
    unsigned bits_per_sample,
    bool compress,
    unsigned sample_instances,
    unsigned sample_rate,
    unsigned interpolation,
    unsigned samples_per_frame);
};

DECLARE_APP(GOPerfTestApp)
IMPLEMENT_APP_CONSOLE(GOPerfTestApp)

GOPerfTestApp::GOPerfTestApp() {}

wxMilliClock_t GOPerfTestApp::getCPUTime() {
#ifdef __linux__
  struct rusage usage;
  if (!getrusage(RUSAGE_SELF, &usage)) {
    wxMilliClock_t t = usage.ru_utime.tv_sec;
    t *= 1000;
    t += usage.ru_utime.tv_usec / 1000;
    return t;
  }
#endif
  return wxGetLocalTimeMillis();
}

void GOPerfTestApp::RunTest(
  unsigned bits_per_sample,
  bool compress,
  unsigned sample_instances,
  unsigned sample_rate,
  unsigned interpolation,
  unsigned samples_per_frame) {
  try {
    GOConfig settings(wxT("perftest"));
    GOOrganController *organController = new GOOrganController(settings);
    const wxString testsDir = argc >= 2 ? argv[1]
                                        : GOStdPath::GetResourceDir()
        + wxFileName::GetPathSeparator() + "perftests";

    organController->InitOrganDirectory(testsDir);
    organController->AddWindchest(new GOWindchest(*organController));
    GOSoundEngine *engine = new GOSoundEngine();
    GOSoundRecorder recorder;

    try {
      ptr_vector<GOSoundProvider> pipes;
      for (unsigned i = 0; i < sample_instances; i++) {
        GOSoundProviderWave *w = new GOSoundProviderWave();

        w->SetAmplitude(102, 0);

        std::vector<GOSoundProviderWave::AttackFileInfo> attacks;
        std::vector<GOSoundProviderWave::ReleaseFileInfo> releases;
        GOSoundProviderWave::AttackFileInfo ainfo;

        ainfo.filename.Assign(wxString::Format(wxT("%02d.wav"), i % 3));
        ainfo.m_WaveTremulantStateFor = BOOL3_DEFAULT;
        ainfo.load_release = true;
        ainfo.percussive = false;
        ainfo.min_attack_velocity = 0;
        ainfo.max_playback_time = -1;
        ainfo.attack_start = 0;
        ainfo.cue_point = -1;
        ainfo.release_end = -1;
        ainfo.loops.clear();
        ainfo.m_LoopCrossfadeLength = 0;
        ainfo.m_ReleaseCrossfadeLength = 0;
        attacks.push_back(ainfo);
        w->LoadFromMultipleFiles(
          organController->GetFileStore(),
          organController->GetMemoryPool(),
          attacks,
          releases,
          bits_per_sample,
          2,
          compress,
          GOSoundProviderWave::LOOP_LOAD_ALL,
          true,
          true);
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

      engine->Setup(organController);

      std::vector<GOSoundSampler *> handles;
      float output_buffer[samples_per_frame * 2];

      for (unsigned i = 0; i < pipes.size(); i++) {
        GOSoundSampler *handle
          = engine->StartPipeSample(pipes[i], 1, 0, 127, 0, 0);
        if (handle)
          handles.push_back(handle);
      }

      wxMilliClock_t start = getCPUTime();
      wxMilliClock_t end;
      wxMilliClock_t diff;
      unsigned batch_size = 1 * engine->GetSampleRate() / samples_per_frame;
      unsigned blocks = 0;
      do {
        for (unsigned i = 0; i < batch_size; i++) {
          engine->GetAudioOutput(output_buffer, samples_per_frame, 0, false);
          engine->NextPeriod();
          blocks++;
        }
        end = getCPUTime();
        diff = end - start;
      } while (diff < 30000);

      float playback_time
        = blocks * (double)samples_per_frame / engine->GetSampleRate();
      wxLogMessage(
        wxT("%u sampler, %f seconds, %u bits, %u, %s, %s, %u block: "
            "%ld ms cpu time, limit: %f"),
        pipes.size(),
        playback_time,
        bits_per_sample,
        sample_rate,
        wxString(compress ? wxT("Y") : wxT("N")),
        wxString(interpolation == 0 ? wxT("Linear") : wxT("Polyphase")),
        samples_per_frame,
        diff.ToLong(),
        playback_time * 1000.0 * pipes.size() / diff.ToLong());

      pipes.clear();
    } catch (wxString msg) {
      wxLogError(wxT("Error: %s"), msg.c_str());
    }

    delete engine;
    delete organController;
  } catch (wxString msg) {
    wxLogError(wxT("Error: %s"), msg.c_str());
  }
}

bool GOPerfTestApp::OnInit() {
  wxLog *logger = new wxLogStream(&std::cout);
  wxLog::SetActiveTarget(logger);
  wxLog::SetLogLevel(wxLOG_Status);
  wxImage::AddHandler(new wxJPEGHandler);
  wxImage::AddHandler(new wxGIFHandler);
  wxImage::AddHandler(new wxPNGHandler);
  wxImage::AddHandler(new wxBMPHandler);
  wxImage::AddHandler(new wxICOHandler);

  if (argc > 2) {
    wxLogError(wxT("Usage: perftest [test-data-directory]"));
    return false;
  }
  return true;
}

int GOPerfTestApp::OnRun() {
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
