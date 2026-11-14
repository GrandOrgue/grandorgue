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
#include <mutex>
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

// The beta of the Kaiser window of the bandlimited filters
static constexpr double BANDLIMITED_KAISER_BETA = 7.86;
/*
  The maximum distance from the cutoff to the nominal stopband edge, in
  cycles per source frame
*/
static constexpr double BANDLIMITED_HALF_TRANSITION_BAND = 0.035;
// The number of terms of the I0 series after the initial constant
static constexpr unsigned N_KAISER_BESSEL_TERMS = 14;

/**
  Computes the modified Bessel function of the first kind and order 0 by its
  power series, for the Kaiser window
  @param value the argument, from 0 to BANDLIMITED_KAISER_BETA
  @return I0(value)
*/
static double kaiser_bessel(double value) {
  assert(value >= 0.0 && value <= BANDLIMITED_KAISER_BETA);

  const double square = value * value / 4.0;
  double sum = 1.0;
  double term = 1.0;

  for (unsigned termN = 1; termN <= N_KAISER_BESSEL_TERMS; ++termN) {
    term *= square / (termN * termN);
    sum += term;
  }
  return sum;
}

/**
  Rounds the block's bandwidth down to the widest bank whose stopband edge
  fits the output Nyquist limit in source units. Uses the narrowest bank if
  the limit falls below its edge
  @param maxIncrement the largest source position advance per output frame
    in the block, in 1/UPSAMPLE_FACTOR units, from 0 to MAX_FRACTION_INCREMENT
  @return the bank index, from 0 to N_BANDLIMITED_BANKS - 1
*/
static unsigned get_bandlimited_bank(unsigned maxIncrement) {
  constexpr unsigned UNITY = GOSoundResample::UPSAMPLE_FACTOR;
  constexpr unsigned N_BANKS = GOSoundResample::N_BANDLIMITED_BANKS;

  assert(maxIncrement <= GOSoundResample::MAX_FRACTION_INCREMENT);
  return std::max(N_BANKS * UNITY / std::max(maxIncrement, UNITY), 1u) - 1;
}

/**
  Calculates a Kaiser-windowed sinc row for each phase of each cutoff bank,
  with each row normalised to unit gain at DC
  @param outCoefs receives the coefficients, indexed by bank, phase row
    (the fraction phaseI / N_BANDLIMITED_PHASES) and window point
*/
static void create_bandlimited_filters(
  float (&outCoefs)[GOSoundResample::N_BANDLIMITED_BANKS]
                   [GOSoundResample::N_BANDLIMITED_ROWS]
                   [GOSoundResample::BANDLIMITED_POINTS]) {
  constexpr unsigned N_POINTS = GOSoundResample::BANDLIMITED_POINTS;
  constexpr unsigned N_ROWS = GOSoundResample::N_BANDLIMITED_ROWS;
  constexpr unsigned N_BANKS = GOSoundResample::N_BANDLIMITED_BANKS;

  for (unsigned phaseI = 0; phaseI < N_ROWS; ++phaseI) {
    const double fraction
      = double(phaseI) / GOSoundResample::N_BANDLIMITED_PHASES;
    double distances[N_POINTS];
    double kaiserWindow[N_POINTS];

    for (unsigned pointI = 0; pointI < N_POINTS; ++pointI) {
      const double distance
        = double(pointI) - GOSoundResample::BANDLIMITED_LEFT - fraction;
      const double relativeDistance = 2.0 * distance / N_POINTS;

      distances[pointI] = distance;
      kaiserWindow[pointI] = kaiser_bessel(
        BANDLIMITED_KAISER_BETA
        * std::sqrt(1.0 - relativeDistance * relativeDistance));
    }

    for (unsigned bankN = 1; bankN <= N_BANKS; ++bankN) {
      const double stopband = 0.5 * bankN / N_BANKS;
      const double cutoff
        = stopband - std::min(BANDLIMITED_HALF_TRANSITION_BAND, stopband / 2.0);
      auto &coefs = outCoefs[bankN - 1][phaseI];
      double sum = 0.0;

      for (unsigned pointI = 0; pointI < N_POINTS; ++pointI) {
        const double coef
          = sinc(2.0 * cutoff * distances[pointI]) * kaiserWindow[pointI];

        coefs[pointI] = float(coef);
        sum += coef;
      }
      // Keep the gain at DC constant as the fractional position changes.
      assert(sum > 0.0);
      for (float &coef : coefs)
        coef = float(coef / sum);
    }
  }
}

GOSoundResample::BandlimitedResampler::BandlimitedResampler(
  const GOSoundResample &resample, unsigned maxIncrement)
  : ScalarProductionResampler(
    resample.m_BandlimitedCoefs[get_bandlimited_bank(maxIncrement)]) {}

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

float GOSoundResample::m_LinearCoefs[UPSAMPLE_FACTOR][LINEAR_POINTS];
float GOSoundResample::m_PolyphaseCoefs[UPSAMPLE_FACTOR][POLYPHASE_POINTS];
float GOSoundResample::m_BandlimitedCoefs[N_BANDLIMITED_BANKS]
                                         [N_BANDLIMITED_ROWS]
                                         [BANDLIMITED_POINTS];

// Guards the single filling of the coefficient tables in the process
static std::once_flag coefs_filled_flag;

void GOSoundResample::fillCoefs() {
  float nyquistFilter[UPSAMPLE_FACTOR * POLYPHASE_POINTS];

  create_nyquist_filter(nyquistFilter, POLYPHASE_POINTS, UPSAMPLE_FACTOR);
  apply_lanczos_window(nyquistFilter, UPSAMPLE_FACTOR * POLYPHASE_POINTS);

  /* Split up the filter into the sub-filters and reverse the coefficient
   * arrays. At phase zero the peak at the middle point of the Nyquist
   * filter moves to the reversed row's centre point. */
  static_assert(
    PolyphaseResampler::CENTRE_POINT
    == POLYPHASE_POINTS - 1 - POLYPHASE_POINTS / 2);
  for (unsigned phaseI = 0; phaseI < UPSAMPLE_FACTOR; phaseI++) {
    for (unsigned pointI = 0; pointI < POLYPHASE_POINTS; pointI++) {
      m_PolyphaseCoefs[phaseI][(POLYPHASE_POINTS - 1) - pointI]
        = nyquistFilter[pointI * UPSAMPLE_FACTOR + phaseI];
    }
  }
  for (unsigned phaseI = 0; phaseI < UPSAMPLE_FACTOR; phaseI++) {
    m_LinearCoefs[phaseI][0] = 1 - (phaseI / (float)UPSAMPLE_FACTOR);
    m_LinearCoefs[phaseI][1] = phaseI / (float)UPSAMPLE_FACTOR;
  }

  create_bandlimited_filters(m_BandlimitedCoefs);
}

GOSoundResample::GOSoundResample() {
  std::call_once(coefs_filled_flag, &GOSoundResample::fillCoefs);
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
    res = BandlimitedResampler::VECTOR_LENGTH;
    break;

  /* A compiler cannot guarantee that interpolation has a valid value.
   * So for avoiding the "res may be used uninitialized" warning */
  default:
    assert(false);
    res = 0;
  }
  return res;
}

unsigned GOSoundResample::getCentrePoint(InterpolationType interpolation) {
  unsigned centrePoint;

  switch (interpolation) {
  case GO_LINEAR_INTERPOLATION:
    centrePoint = LinearResampler::CENTRE_POINT;
    break;
  case GO_POLYPHASE_INTERPOLATION:
    centrePoint = PolyphaseResampler::CENTRE_POINT;
    break;
  case GO_BANDLIMITED_INTERPOLATION:
    centrePoint = BandlimitedResampler::CENTRE_POINT;
    break;
  default:
    assert(false);
    centrePoint = 0;
  }
  return centrePoint;
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
