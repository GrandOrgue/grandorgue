/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundReverbProcessor.h"

#include <cassert>

#include <wx/intl.h>
#include <wx/log.h>

#include "sound/buffer/GOSoundBufferMutableMono.h"
#include "sound/buffer/GOSoundBufferPlanarMutable.h"

#include "zita-convolver.h"

GOSoundReverbProcessor::GOSoundReverbProcessor(
  const GOSoundReverb::ReverbConfig &config)
  : m_config(config) {}

void GOSoundReverbProcessor::EnsureSetup(
  unsigned nChannels, unsigned nFrames, unsigned sampleRate) {
  // The impulse response depends only on m_config and sampleRate (it is
  // resampled to sampleRate by loadIRData()) - nChannels/nFrames only affect
  // CreateTypedState()'s Convproc construction, so a channel- or
  // buffer-size-only change must not force a reload from disk.
  const bool isSampleRateChanged
    = !m_IsSetupCalled || sampleRate != m_SampleRate;

  if (isSampleRateChanged || !m_IsIRLoaded) {
    try {
      m_irData = GOSoundReverb::loadIRData(m_config, sampleRate);
      m_IsIRLoaded = true;
    } catch (wxString error) {
      wxLogError(_("Reverb load error: %s"), error.c_str());
      m_irData = GOSoundReverb::IRData();
      m_IsIRLoaded = false;
    }
  }
  m_NChannels = nChannels;
  m_NFrames = nFrames;
  m_SampleRate = sampleRate;
  m_IsSetupCalled = true;
}

std::unique_ptr<GOSoundReverbProcessorState> GOSoundReverbProcessor::
  CreateTypedState() const {
  assert(
    m_IsSetupCalled && "EnsureSetup() must be called before CreateState()");

  return std::make_unique<GOSoundReverbProcessorState>(
    m_irData, m_NChannels, m_NFrames);
}

void GOSoundReverbProcessor::Process(
  GOSoundReverbProcessorState &state,
  GOSoundBufferPlanarMutable &buffer) const {
  // An empty mp_ConvprocsByChannel means the state's constructor found
  // nothing valid to convolve (no impulse response loaded, or configure()
  // failed) and built a no-op state instead - mirrors
  // GOSoundReverb::Process()'s own `if (!m_engine.size()) return;` for the
  // same case.
  if (!state.mp_ConvprocsByChannel.empty()) {
    const unsigned nFrames = buffer.GetNFrames();
    const unsigned nChannels = buffer.GetNChannels();

    assert(nChannels == state.mp_ConvprocsByChannel.size());

    for (unsigned channelI = 0; channelI < nChannels; channelI++) {
      Convproc *const pConvProc = state.mp_ConvprocsByChannel[channelI].get();

      if (pConvProc->state() != Convproc::ST_WAIT)
        pConvProc->check_stop();

      if (pConvProc->state() == Convproc::ST_PROC) {
        GOSoundBufferMutableMono channelBuffer
          = buffer.GetChannelBuffer(channelI);
        GOSoundBufferMutableMono convInBuffer(pConvProc->inpdata(0), nFrames);
        GOSoundBufferMutableMono convOutBuffer(pConvProc->outdata(0), nFrames);

        convInBuffer.CopyFrom(channelBuffer);
        pConvProc->process(false);
        channelBuffer.CopyFrom(convOutBuffer);
      }
    }
  }
}
