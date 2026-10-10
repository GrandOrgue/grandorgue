/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundReverbProcessorState.h"

#include <algorithm>

#include "zita-convolver.h"

GOSoundReverbProcessorState::GOSoundReverbProcessorState(
  const GOSoundReverb::IRData &irData, unsigned nChannels, unsigned nFrames) {
  // An empty irData means there is nothing to convolve at all - e.g.
  // EnsureSetup() failed to load the file - so build no engines and keep
  // this state a safe no-op instead (Process()/Reset() below already treat
  // an empty mp_ConvprocsByChannel as such), mirroring
  // GOSoundReverb::Setup()'s own m_engine.clear() on a load failure.
  if (!irData.data.empty()) {
    unsigned val = nFrames;

    if (val < Convproc::MINPART)
      val = Convproc::MINPART;
    if (val > Convproc::MAXPART)
      val = Convproc::MAXPART;

    const unsigned block = 0x4000;
    float *const d = const_cast<float *>(irData.data.data());
    const unsigned l = (unsigned)irData.data.size();
    bool isConfigOk = true;

    for (unsigned channelI = 0; channelI < nChannels && isConfigOk;
         channelI++) {
      auto pConvProc = std::make_unique<Convproc>();

      // Disable stopping the reverb engine when the system is overloaded
      pConvProc->set_options(Convproc::OPT_LATE_CONTIN);
      isConfigOk = pConvProc->configure(
                     1, 1, 1000000, nFrames, val, Convproc::MAXPART, 1)
        == 0;
      if (isConfigOk) {
        float g = 1;

        if (irData.isDirect)
          pConvProc->impdata_create(0, 0, 0, &g, 0, 1);
        for (unsigned j = 0; j < l; j += block)
          pConvProc->impdata_create(
            0,
            0,
            1,
            d + j,
            irData.delay + j,
            irData.delay + j + std::min(l - j, block));

        pConvProc->start_process(0, 0);
        mp_ConvprocsByChannel.push_back(std::move(pConvProc));
      }
    }
    if (!isConfigOk) {
      // configure() failed partway through (mirrors GOSoundReverb::Setup(),
      // which throws and lets its catch block m_engine.clear()): stop and
      // clean up whichever engines already had start_process() called, the
      // same explicit sequence the destructor below uses, then abandon the
      // whole state rather than leave a half-running, half-missing set of
      // channels.
      for (auto &pConvProc : mp_ConvprocsByChannel) {
        pConvProc->stop_process();
        pConvProc->cleanup();
      }
      mp_ConvprocsByChannel.clear();
    }
  }
}

// Must be defined here (not in the header) so that
// std::vector<std::unique_ptr<Convproc>> can call the complete destructor of
// its managed type (Convproc), which is only forward-declared in the header
// file. Stops and cleans up every engine explicitly, the same order
// GOSoundReverb::Cleanup() uses before its own m_engine is torn down, rather
// than leaving it to each Convproc's own destructor (which makes the same
// two calls).
//
// Tearing a state down right after building it - before zita-convolver has
// scheduled its worker threads - used to hang here forever, because the stop
// command was lost and Convproc::cleanup()'s `while (!check_stop())` poll
// never terminated. Fixed in the library
// (https://github.com/GrandOrgue/ZitaConvolver/issues/1, released as 4.0.4),
// so this destructor needs no precautions of its own.
GOSoundReverbProcessorState::~GOSoundReverbProcessorState() {
  for (auto &pConvProc : mp_ConvprocsByChannel) {
    pConvProc->stop_process();
    pConvProc->cleanup();
  }
}

void GOSoundReverbProcessorState::Reset() {
  for (auto &pConvProc : mp_ConvprocsByChannel)
    pConvProc->reset();
}
