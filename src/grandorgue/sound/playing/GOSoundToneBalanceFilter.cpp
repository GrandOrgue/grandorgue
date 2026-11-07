/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundToneBalanceFilter.h"

#include <cmath>

void GOSoundToneBalanceFilter::Init(int8_t value) {
  GOSoundOnePoleFilter::Type type;
  double hz = 0;

  if (value == 0)
    type = GOSoundOnePoleFilter::Type::TYPE_NONE;
  else if (value < 0) {
    type = GOSoundOnePoleFilter::Type::TYPE_LPF;
    hz = 16000 * pow(20.0 / 16000.0, abs(value) / 99.0);
  } else {
    type = GOSoundOnePoleFilter::Type::TYPE_HPF;
    hz = 20 * pow(800.0, value / 99.0);
  }

  GOSoundOnePoleFilter::Coeffs newCoeffs;

  GOSoundOnePoleFilter::computeCoeffs(type, hz, 0, m_samplerate, newCoeffs);
  Publish(newCoeffs);
}

void GOSoundToneBalanceFilter::SetSamplerate(unsigned samplerate) {
  m_samplerate = samplerate;
  Publish(GOSoundOnePoleFilter::Coeffs());
}

void GOSoundToneBalanceFilter::Publish(
  const GOSoundOnePoleFilter::Coeffs &newCoeffs) {
  // the writers are never concurrent: they are called from the GUI thread
  // or from the engine build, which does not run together with the GUI
  // changes of the same filter
  const unsigned startVersion = m_Version.load(std::memory_order_relaxed);

  m_Version.store(startVersion + 1, std::memory_order_relaxed);
  std::atomic_thread_fence(std::memory_order_release);
  m_coeffs = newCoeffs;
  m_Version.store(startVersion + 2, std::memory_order_release);
}

void GOSoundToneBalanceFilter::State::Init(
  const GOSoundToneBalanceFilter *filter) {
  p_filter = filter;
  m_coeffs = GOSoundOnePoleFilter::Coeffs();
  m_Version = 1; // odd: the coefficients are not copied yet
  for (int i = 0; i < 2; i++)
    m_state[i] = 0;
}

void GOSoundToneBalanceFilter::State::EnsureCoeffs() {
  if (p_filter) {
    const unsigned version
      = p_filter->m_Version.load(std::memory_order_acquire);

    if (version != m_Version && !(version & 1)) {
      const GOSoundOnePoleFilter::Coeffs copy = p_filter->m_coeffs;

      std::atomic_thread_fence(std::memory_order_acquire);
      // use the copy only if it was not written during copying
      if (p_filter->m_Version.load(std::memory_order_relaxed) == version) {
        m_Version = version;
        m_coeffs = copy;
        if (!copy.isActive)
          for (int i = 0; i < 2; i++)
            m_state[i] = 0;
      }
    }
  }
}
