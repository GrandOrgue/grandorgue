/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2013 GrandOrgue contributors (see AUTHORS)
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
#include "GOSoundDefs.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

static
inline
double
sinc
	(const double arg
	)
{
	return (arg == 0) ? 1.0 : sin(M_PI * arg) / (M_PI * arg);
}

static
inline
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

static
inline
void
apply_lanczos_window
	(float          *buffer
	,const unsigned  length
	)
{
	const int ldec = ((int)length) - 1;
	for (int i = 0; i <= ldec; i++)
	{
		buffer[i] *= sinc((2 * i - ldec) / (double)ldec);
	}
}

void
resampler_coefs_init
	(struct resampler_coefs_s   *resampler_coefs
	,const unsigned              input_sample_rate
	,interpolation_type          interpolation
	)
{
	static const double generalised_max_frequency = ((double)UPSAMPLE_FACTOR / (double)MAX_POSITIVE_FACTOR);
	double cutoff_frequency = ((double)input_sample_rate / 2.0) * generalised_max_frequency;
	float temp[UPSAMPLE_FACTOR * SUBFILTER_TAPS];
	create_sinc_filter
		(temp
		,UPSAMPLE_FACTOR * SUBFILTER_TAPS
		,cutoff_frequency / 2
		,cutoff_frequency
		,input_sample_rate * UPSAMPLE_FACTOR
		,UPSAMPLE_FACTOR
		);

	apply_lanczos_window
		(temp
		,UPSAMPLE_FACTOR * SUBFILTER_TAPS
		);

	/* Split up the filter into the sub-filters and reverse the coefficient
	 * arrays. */
	for (unsigned i = 0; i < UPSAMPLE_FACTOR; i++)
	{
		for (unsigned j = 0; j < SUBFILTER_TAPS; j++)
		{
			resampler_coefs->coefs[i * SUBFILTER_TAPS + ((SUBFILTER_TAPS - 1) - j)] = temp[j * UPSAMPLE_FACTOR + i];
		}
	}
	for (unsigned i = 0; i < UPSAMPLE_FACTOR; i++)
	{
		resampler_coefs->linear[i][0] = i /  (float)UPSAMPLE_FACTOR;
		resampler_coefs->linear[i][1] = 1 - (i /  (float)UPSAMPLE_FACTOR);
	}
	resampler_coefs->interpolation = interpolation;
}

float* 
resample_block(float* data, unsigned& len, unsigned from_samplerate, unsigned to_samplerate)
{
	struct resampler_coefs_s coefs;
	float factor = ((float)from_samplerate) / to_samplerate;
	resampler_coefs_init(&coefs, to_samplerate, GO_POLYPHASE_INTERPOLATION);
	const float* coef   = coefs.coefs;
	unsigned new_len = ceil(len / factor);
	unsigned position_index = 0;
	unsigned position_fraction = 0;
	unsigned increment_fraction = factor * UPSAMPLE_FACTOR;
	if (!new_len)
		return 0;
	float* out = new float[new_len];

	for (unsigned i = 0; i < new_len; ++i, position_fraction += increment_fraction)
	{
		position_index += position_fraction >> UPSAMPLE_BITS;
		position_fraction = position_fraction & (UPSAMPLE_FACTOR - 1);
		float out1 = 0.0f;
		float out2 = 0.0f;
		float out3 = 0.0f;
		float out4 = 0.0f;
		const float* coef_set = &coef[position_fraction << SUBFILTER_BITS];
		float* in_set = &data[position_index];
		for (unsigned j = 0; j < SUBFILTER_TAPS; j += 4)
		{
			out1 += in_set[j]   * coef_set[j];
			out2 += in_set[j+1] * coef_set[j+1];
			out3 += in_set[j+2] * coef_set[j+2];
			out4 += in_set[j+3] * coef_set[j+3];
		}
		out[i] = out1 + out2 + out3 + out4;
	}
	len = new_len;
	return out;
}
