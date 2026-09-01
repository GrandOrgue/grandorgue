/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundReverb.h"

#include <algorithm>
#include <memory>
#include <new>

#include <wx/intl.h>
#include <wx/log.h>

#include "config/GOConfig.h"
#include "files/GOStandardFile.h"
#include "sound/playing/GOSoundResample.h"

#include "GOWave.h"
#include "zita-convolver.h"

const GOSoundReverb::ReverbConfig GOSoundReverb::CONFIG_REVERB_DISABLED
  = {false, false, 0, 0, 0, 0, 0.0f, std::string()};

GOSoundReverb::ReverbConfig GOSoundReverb::createReverbConfig(
  const GOConfig &config) {
  return {
    .isEnabled = config.ReverbEnabled(),
    .isDirect = config.ReverbDirect(),
    .channel = config.ReverbChannel(),
    .startOffset = config.ReverbStartOffset(),
    .len = config.ReverbLen(),
    .delay = config.ReverbDelay(),
    .gain = config.ReverbGain(),
    .file = std::string(config.ReverbFile().ToUTF8().data()),
  };
}

GOSoundReverb::GOSoundReverb(unsigned channels)
  : m_channels(channels), m_engine() {}

GOSoundReverb::~GOSoundReverb() { Cleanup(); }

void GOSoundReverb::Cleanup() {
  for (unsigned i = 0; i < m_engine.size(); i++) {
    m_engine[i]->stop_process();
    m_engine[i]->cleanup();
  }
}

GOSoundReverb::IRData GOSoundReverb::loadIRData(
  const ReverbConfig &config, unsigned sampleRate) {
  GOWave wav;
  unsigned offset = config.startOffset;
  const float gain = config.gain;
  unsigned len;

  GOStandardFile reverb_file(wxString::FromUTF8(config.file.c_str()));
  wav.Open(&reverb_file);
  if (offset > wav.GetLength())
    throw(wxString) _("Invalid reverb start offset");
  len = wav.GetLength();

  // Read directly into the vector that (modulo a resample below) becomes
  // IRData::data, rather than a malloc'd scratch buffer copied into it
  // afterwards: at MAX_SAMPLE_LENGTH, a second full-size copy would nearly
  // double peak memory right when a large IR is most likely to be declined
  // as out of memory.
  std::vector<float> data;

  try {
    data.resize(len);
  } catch (const std::bad_alloc &) {
    throw(wxString) _("Out of memory");
  }
  wav.ReadSamples(
    data.data(), GOWave::SF_IEEE_FLOAT, wav.GetSampleRate(), -config.channel);

  for (float &sample : data)
    sample *= gain;
  if (len >= offset + config.len && config.len)
    len = offset + config.len;
  if (wav.GetSampleRate() != sampleRate) {
    GOSoundResample resample;
    // Mallocs its own buffer and updates len to the resampled length.
    float *const resampled = resample.NewResampledMono(
      data.data(), len, wav.GetSampleRate(), sampleRate);

    if (!resampled)
      throw(wxString) _("Resampling failed");

    // Owns resampled for this scope, so it is freed even if the code below
    // throws.
    std::unique_ptr<float, decltype(&free)> resampledData(resampled, &free);

    if (len <= data.capacity()) {
      // The resampled data still fits in the existing buffer: reuse it in
      // place instead of allocating a second one.
      std::copy(resampled, resampled + len, data.begin());
      data.resize(len);
    } else {
      // The existing buffer is too small to hold the resampled data (e.g.
      // when upsampling). Release it before allocating the bigger one, so
      // the resampled data and the new vector are the only large buffers
      // alive at once, instead of three.
      data.clear();
      data.shrink_to_fit();
      try {
        data.assign(resampled, resampled + len);
      } catch (const std::bad_alloc &) {
        throw(wxString) _("Out of memory");
      }
    }
    offset = (offset * sampleRate) / (float)wav.GetSampleRate();
  }
  wav.Close();

  IRData irData;

  irData.delay = (sampleRate * config.delay) / 1000;
  irData.isDirect = config.isDirect;
  // Shift the kept region to the front of the same buffer and shrink,
  // instead of assign()-ing a second buffer for the trimmed copy: shrinking
  // a vector never reallocates, so this is the zero-extra-memory version of
  // the offset trim.
  std::copy(data.begin() + offset, data.begin() + len, data.begin());
  data.resize(len - offset);
  irData.data = std::move(data);

  return irData;
}

void GOSoundReverb::Setup(
  const ReverbConfig &config, unsigned nSamplesPerBuffer, unsigned sampleRate) {
  Cleanup();

  if (!config.isEnabled)
    return;

  m_engine.clear();
  for (unsigned i = 0; i < m_channels; i++) {
    Convproc *pConvProc = new Convproc();

    // Disable stopping the reverb engine when the system is overloaded
    pConvProc->set_options(Convproc::OPT_LATE_CONTIN);
    m_engine.push_back(pConvProc);
  }
  unsigned val = nSamplesPerBuffer;
  if (val < Convproc::MINPART)
    val = Convproc::MINPART;
  if (val > Convproc::MAXPART)
    val = Convproc::MAXPART;
  try {
    for (unsigned i = 0; i < m_engine.size(); i++)
      if (m_engine[i]->configure(
            1, 1, 1000000, nSamplesPerBuffer, val, Convproc::MAXPART, 1))
        throw(wxString) _("Invalid reverb configuration (samples per buffer)");

    const IRData irData = loadIRData(config, sampleRate);
    const unsigned block = 0x4000;
    float *const d = const_cast<float *>(irData.data.data());
    const unsigned l = (unsigned)irData.data.size();

    for (unsigned i = 0; i < m_channels; i++) {
      float g = 1;
      if (irData.isDirect)
        m_engine[i]->impdata_create(0, 0, 0, &g, 0, 1);
      for (unsigned j = 0; j < l; j += block) {
        m_engine[i]->impdata_create(
          0,
          0,
          1,
          d + j,
          irData.delay + j,
          irData.delay + j + std::min(l - j, block));
      }
    }
    for (unsigned i = 0; i < m_engine.size(); i++)
      m_engine[i]->start_process(0, 0);
  } catch (wxString error) {
    wxLogError(_("Reverb load error: %s"), error.c_str());
    m_engine.clear();
  }
}

void GOSoundReverb::Reset() {
  for (unsigned i = 0; i < m_engine.size(); i++)
    m_engine[i]->reset();
}

void GOSoundReverb::Process(float *output_buffer, unsigned n_frames) {
  if (!m_engine.size())
    return;

  for (unsigned i = 0; i < m_channels; i++) {
    float *const pGoData = output_buffer + i;
    // because output_buffer is interleaved
    Convproc *const pConvProc = m_engine[i];

    if (pConvProc->state() != Convproc::ST_WAIT)
      pConvProc->check_stop();

    if (pConvProc->state() == Convproc::ST_PROC) {
      // fill the convolver input buffer with the GO data
      float *pGoFrom = pGoData;
      float *pConvTo = pConvProc->inpdata(0);

      for (unsigned j = 0; j < n_frames; j++) {
        *(pConvTo++) = *pGoFrom;
        pGoFrom += m_channels;
      }
      pConvProc->process(false);

      // fill the GO buffer with the convolver output
      float *pGoTo = pGoData;
      float *pConvFrom = pConvProc->outdata(0);

      for (unsigned j = 0; j < n_frames; j++) {
        *pGoTo = *(pConvFrom++);
        pGoTo += m_channels;
      }
    }
  }
}
