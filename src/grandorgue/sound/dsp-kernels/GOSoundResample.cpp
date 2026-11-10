/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
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

#include "GOSoundResample.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

void GOSoundResample::ResamplingPosition::Init(
  unsigned startIndex, unsigned startFraction) {
  assert(startFraction < UPSAMPLE_FACTOR);
  m_index = startIndex;
  m_fraction = startFraction;
}

unsigned GOSoundResample::scaleResamplerPosIncrement(
  unsigned resamplerPosIncrement, float ratio) {
  assert(resamplerPosIncrement > 0);
  assert(ratio > 0.0f);

  return (unsigned)std::clamp<float>(
    std::round(ratio * resamplerPosIncrement),
    MIN_FRACTION_INCREMENT,
    MAX_FRACTION_INCREMENT);
}

static double sinc(const double arg) {
  return (arg == 0) ? 1.0 : std::sin(M_PI * arg) / (M_PI * arg);
}

namespace {

constexpr unsigned N_BANDLIMITED_BANKS = 64;
constexpr double KAISER_BETA = 7.86;
// Half of the transition width in cycles per source frame.
constexpr double HALF_TRANSITION_WIDTH = 0.035;

// I0 for the Kaiser window. The argument is at most KAISER_BETA.
double kaiser_bessel(double value) {
  const double square = value * value / 4.0;
  double sum = 1.0;
  double term = 1.0;

  for (unsigned termN = 1; termN <= 14; ++termN) {
    term *= square / (termN * termN);
    sum += term;
  }
  return sum;
}

struct BandlimitedTables {
  static constexpr unsigned N_PHASES
    = GOSoundResample::BandlimitedResampler::N_PHASES;
  static constexpr unsigned N_TAPS = GOSoundResample::BANDLIMITED_POINTS;

  float m_Coefs[N_BANDLIMITED_BANKS][N_PHASES + 1][N_TAPS];

  BandlimitedTables() {
    for (unsigned phaseI = 0; phaseI <= N_PHASES; ++phaseI) {
      const double fraction = double(phaseI) / N_PHASES;
      double distances[N_TAPS];
      double window[N_TAPS];

      for (unsigned tapI = 0; tapI < N_TAPS; ++tapI) {
        const double distance
          = double(tapI) - GOSoundResample::BANDLIMITED_LEFT - fraction;
        const double relativeDistance = 2.0 * distance / N_TAPS;

        distances[tapI] = distance;
        window[tapI] = kaiser_bessel(
          KAISER_BETA * std::sqrt(1.0 - relativeDistance * relativeDistance));
      }

      for (unsigned bankN = 1; bankN <= N_BANDLIMITED_BANKS; ++bankN) {
        const double stopband = 0.5 * bankN / N_BANDLIMITED_BANKS;
        const double cutoff
          = stopband - std::min(HALF_TRANSITION_WIDTH, stopband / 2.0);
        float(&coefs)[N_TAPS] = m_Coefs[bankN - 1][phaseI];
        double sum = 0.0;

        for (unsigned tapI = 0; tapI < N_TAPS; ++tapI) {
          const double coef
            = sinc(2.0 * cutoff * distances[tapI]) * window[tapI];

          coefs[tapI] = float(coef);
          sum += coef;
        }
        // Keep the gain at DC constant as the fractional position changes.
        for (float &coef : coefs)
          coef = float(coef / sum);
      }
    }
  }
};

// Shared by all voices and ready before the audio callback can run.
const BandlimitedTables BANDLIMITED_TABLES;

} // namespace

GOSoundResample::BandlimitedResampler::BandlimitedResampler(
  unsigned increment) {
  assert(increment);

  // Round the bandwidth down to the next available bank.
  const unsigned bankN = std::clamp(
    N_BANDLIMITED_BANKS * UPSAMPLE_FACTOR / increment, 1u, N_BANDLIMITED_BANKS);

  p_Coefs = BANDLIMITED_TABLES.m_Coefs[bankN - 1];
}

#if 0
static
void
create_sinc_filter
	(float          *filter_buffer
	,const unsigned  filter_buffer_length
	,const float     f0
	,const float     band_width
	,const unsigned  sample_rate
	,const float     gain
	)
{
	for (unsigned i = 0; i < filter_buffer_length; i++)
	{
		double t = ((double)i - (filter_buffer_length * 0.5)) / sample_rate;
		filter_buffer[i] = gain * (float)(2.0 * band_width * cos(2.0 * M_PI * f0 * t) * sinc(band_width * t) / sample_rate);
	}
}
#endif

static void apply_lanczos_window(float *buffer, const unsigned length) {
  const int ldec = ((int)length) - 1;
  for (int i = 0; i <= ldec; i++) {
    buffer[i] *= sinc((2 * i - ldec) / (double)ldec);
  }
}

static void create_nyquist_filter(
  float *output, unsigned subfilter_taps, unsigned nb_subfilters) {
  int i;
  const int filter_length = (int)(subfilter_taps * nb_subfilters);
  const double inv_taps = 1.0 / nb_subfilters;
  for (i = 0; i < filter_length; i++) {
    double a = (i - filter_length / 2) * inv_taps * M_PI;
    double f = (i - filter_length / 2) ? (std::sin(a) / a) : 1.0;
    output[i] = f;
  }
}

GOSoundResample::GOSoundResample() {
  float temp[UPSAMPLE_FACTOR * POLYPHASE_POINTS];

  create_nyquist_filter(temp, POLYPHASE_POINTS, UPSAMPLE_FACTOR);
  apply_lanczos_window(temp, UPSAMPLE_FACTOR * POLYPHASE_POINTS);

  /* Split up the filter into the sub-filters and reverse the coefficient
   * arrays. */
  for (unsigned i = 0; i < UPSAMPLE_FACTOR; i++) {
    for (unsigned j = 0; j < POLYPHASE_POINTS; j++) {
      m_PolyphaseCoefs[i][(POLYPHASE_POINTS - 1) - j]
        = temp[j * UPSAMPLE_FACTOR + i];
    }
  }
  for (unsigned i = 0; i < UPSAMPLE_FACTOR; i++) {
    m_LinearCoefs[i][0] = 1 - (i / (float)UPSAMPLE_FACTOR);
    m_LinearCoefs[i][1] = i / (float)UPSAMPLE_FACTOR;
  }
}

unsigned GOSoundResample::getVectorLength(
  GOSoundResample::InterpolationType interpolation) {
  unsigned res;

  switch (interpolation) {
  case GO_LINEAR_INTERPOLATION:
    res = LinearResampler::VECTOR_LENGTH;
    break;
  case GO_POLYPHASE_INTERPOLATION:
    res = PolyphaseResampler::VECTOR_LENGTH;
    break;
  case GO_BANDLIMITED_INTERPOLATION:
    res = BANDLIMITED_POINTS;
    break;

  /* A compiler cannot guarantee that interpolation has a valid value.
   * So for avoiding the "res may be used uninitialized" warning */
  default:
    assert(false);
    res = 0;
  }
  return res;
}

float *GOSoundResample::NewResampledMono(
  const float *data,
  unsigned &len,
  unsigned from_samplerate,
  unsigned to_samplerate) {
  ResamplingPosition resamplingPos;
  const unsigned resamplingPosIncrement
    = rateToFractionIncrement((float)from_samplerate / to_samplerate);

  resamplingPos.Init();

  unsigned new_len
    = resamplingPos.AvailableTargetSamples(len, resamplingPosIncrement);

  if (!new_len)
    return NULL;

  float *out = (float *)malloc(sizeof(float) * new_len);

  if (!out)
    return NULL;

  BoundedPtrFrameVector<float, float, 1> w(data, len);
  PolyphaseResampler resampler(*this);

  resampler.ResampleBlock<BoundedPtrFrameVector<float, float, 1>, 1>(
    resamplingPosIncrement, resamplingPos, w, out, new_len);
  len = new_len;
  return out;
}
