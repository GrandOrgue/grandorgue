/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundTestSignals.h"

#include <algorithm>
#include <cmath>

#include "sound/buffer/GOSoundBufferPlanarMutable.h"

#include "GOInt.h"

void GOSoundTestSignals::fillWithRamp(std::vector<float> &outData) {
  for (unsigned n = outData.size(), i = 0; i < n; i++)
    outData[i] = (float)i;
}

void GOSoundTestSignals::fillWithRamp(std::vector<GOInt24> &outData) {
  for (unsigned n = outData.size(), i = 0; i < n; i++)
    outData[i] = (int)(i % 0x7fffff);
}

void GOSoundTestSignals::fillWithAlternatingSignal(
  GOSoundBufferPlanarMutable &outBuffer) {
  float *pData = outBuffer.GetData();

  for (unsigned itemI = 0, n = outBuffer.GetNItems(); itemI < n; itemI++)
    pData[itemI] = (itemI % 2 == 0) ? 0.5f : -0.5f;
}

std::vector<float> GOSoundTestSignals::makeSinePeriod(
  unsigned nFrames, float amplitude, float periodFrames) {
  std::vector<float> signal(nFrames);

  for (unsigned frameI = 0; frameI < nFrames; ++frameI)
    signal[frameI]
      = amplitude * std::sin(2.0f * (float)M_PI * (float)frameI / periodFrames);
  return signal;
}

std::vector<float> GOSoundTestSignals::makeSine(
  unsigned nFrames, double freqHz, unsigned sampleRate, float amplitude) {
  std::vector<float> signal(nFrames);

  for (unsigned frameI = 0; frameI < nFrames; ++frameI)
    signal[frameI]
      = amplitude * (float)std::sin(2.0 * M_PI * freqHz * frameI / sampleRate);
  return signal;
}

void GOSoundTestSignals::fillWithRamp(GOSoundBufferPlanarMutable &outBuffer) {
  float *pData = outBuffer.GetData();

  for (unsigned itemI = 0, n = outBuffer.GetNItems(); itemI < n; itemI++)
    pData[itemI] = (float)itemI;
}

std::vector<double> GOSoundTestSignals::findRisingZeroCrossings(
  const std::vector<float> &signal, unsigned beginFrame, unsigned endFrame) {
  std::vector<double> crossings;

  for (unsigned frameI = beginFrame; frameI + 1 < endFrame; ++frameI) {
    const double cur = signal[frameI];
    const double next = signal[frameI + 1];

    if (cur < 0.0 && next >= 0.0)
      crossings.push_back(frameI + (-cur) / (next - cur));
  }
  return crossings;
}

double GOSoundTestSignals::maxPitchErrorCents(
  const std::vector<float> &signal,
  unsigned beginFrame,
  double carrierHz,
  unsigned sampleRate,
  const std::vector<double> &rates,
  unsigned &outNPeriods) {
  const std::vector<double> crossings
    = findRisingZeroCrossings(signal, beginFrame, signal.size());
  double maxErrorCents = 0.0;

  outNPeriods = crossings.size() > 0 ? crossings.size() - 1 : 0;
  for (unsigned crossingI = 0; crossingI < outNPeriods; ++crossingI) {
    const double beginPos = crossings[crossingI];
    const double endPos = crossings[crossingI + 1];
    const unsigned beginFrameI = (unsigned)std::ceil(beginPos);
    const unsigned endFrameI = (unsigned)std::ceil(endPos);
    double sumPeriodRates = 0.0;

    for (unsigned frameI = beginFrameI; frameI < endFrameI; ++frameI)
      sumPeriodRates += rates[frameI];

    const double expectedHz
      = carrierHz * sumPeriodRates / (endFrameI - beginFrameI);
    const double measuredHz = sampleRate / (endPos - beginPos);
    const double errorCents = 1200.0 * std::log2(measuredHz / expectedHz);

    maxErrorCents = std::max(maxErrorCents, std::fabs(errorCents));
  }
  return maxErrorCents;
}
