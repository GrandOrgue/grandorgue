/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
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

#include <math.h>
#include <stdlib.h>

#include "GOSoundDefs.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

void GOSoundResample::ResamplingPosition::Init(
  float factor, unsigned startIndex, const ResamplingPosition *pOld) {
  m_index = startIndex;
  m_fraction = pOld ? pOld->m_fraction : 0;
  m_FractionIncrement
    = roundf(factor * (pOld ? pOld->m_FractionIncrement : UPSAMPLE_FACTOR));
}

static double sinc(const double arg) {
  return (arg == 0) ? 1.0 : sin(M_PI * arg) / (M_PI * arg);
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
    double f = (i - filter_length / 2) ? (sin(a) / a) : 1.0;
    output[i] = f;
  }
}

void GOSoundResample::Init(
  const unsigned input_sample_rate,
  GOSoundResample::InterpolationType interpolation) {
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
    m_LinearCoeffs[i][0] = i / (float)UPSAMPLE_FACTOR;
    m_LinearCoeffs[i][1] = 1 - (i / (float)UPSAMPLE_FACTOR);
  }
  m_interpolation = interpolation;
}

float *GOSoundResample::newResampledMono(
  const float *data,
  unsigned &len,
  unsigned from_samplerate,
  unsigned to_samplerate) {
  struct GOSoundResample resample;
  float factor = ((float)from_samplerate) / to_samplerate;

  resample.Init(to_samplerate, GO_POLYPHASE_INTERPOLATION);

  unsigned new_len = ceil(len / factor);
  unsigned position_index = 0;
  unsigned position_fraction = 0;
  unsigned increment_fraction = factor * UPSAMPLE_FACTOR;
  if (!new_len)
    return NULL;
  float *out = (float *)malloc(sizeof(float) * new_len);
  if (!out)
    return NULL;

  for (unsigned i = 0; i < new_len;
       ++i, position_fraction += increment_fraction) {
    position_index += position_fraction >> UPSAMPLE_BITS;
    position_fraction = position_fraction & (UPSAMPLE_FACTOR - 1);
    float out1 = 0.0f;
    float out2 = 0.0f;
    float out3 = 0.0f;
    float out4 = 0.0f;
    const float *coef_set = resample.m_PolyphaseCoefs[position_fraction];
    const float *in_set = &data[position_index];
    unsigned max_i = len - position_index;

    for (unsigned j = 0; j < POLYPHASE_POINTS; j += 4) {
      unsigned k = j;

      if (k < max_i)
        out1 += in_set[k] * coef_set[k];
      if (++k < max_i)
        out2 += in_set[k] * coef_set[k];
      if (++k < max_i)
        out3 += in_set[k] * coef_set[k];
      if (++k < max_i)
        out4 += in_set[k] * coef_set[k];
    }
    out[i] = out1 + out2 + out3 + out4;
  }
  len = new_len;
  return out;
}
