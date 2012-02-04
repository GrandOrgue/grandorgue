/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2011 GrandOrgue contributors (see AUTHORS)
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
 * MA 02111-1307, USA.

 */

#ifndef GOSOUNDRESAMPLE_H_
#define GOSOUNDRESAMPLE_H_

#include <xmmintrin.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "GOSoundDefs.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#define SUBFILTER_BITS            (3U)
#define SUBFILTER_TAPS            (1U << SUBFILTER_BITS)
#define UPSAMPLE_BITS             (11U)
#define UPSAMPLE_FACTOR           (1U << UPSAMPLE_BITS)

/* This factor must not be exceeded in the downsampler and it MUST be
 * greater than UPSAMPLE_FACTOR. */
#define MAX_POSITIVE_FACTOR       (2300U)

struct resampler_coefs_s
{
	float coefs[UPSAMPLE_FACTOR * SUBFILTER_TAPS];
};

struct resampler_s
{
	unsigned                         input_index;
	unsigned                         filter_index;
	const struct resampler_coefs_s  *coefs;
};

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
		double t = ((double)i - ((filter_buffer_length - 1) * 0.5)) / sample_rate;
		filter_buffer[i] = gain * (float)(2.0 * band_width * cos(2.0 * M_PI * f0 * t) * sinc(band_width * t) / sample_rate);
	}
}

static
inline
void
apply_gaussian_window
	(float          *buffer
	,const unsigned  length
	)
{
	static const double gauss_sigma = 0.4;
	const double ldec_2 = (length - 1) / 2.0;
	for (unsigned i = 0; i < length; i++)
	{
		const double fac = (i - ldec_2) / (gauss_sigma * ldec_2);
		buffer[i] *= exp(-(fac * fac) / 2.0);
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


static
inline
void
resampler_coefs_init
	(struct resampler_coefs_s   *resampler_coefs
	,const unsigned              input_sample_rate
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
		,UPSAMPLE_FACTOR / 2
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
}

static
inline
void
resampler_init
	(struct resampler_s              *resampler
	,const struct resampler_coefs_s  *coefs
	)
{
	resampler->coefs        = coefs;
	resampler->input_index  = 0;
	resampler->filter_index = 0;
}

static
inline
unsigned
resampler_go_fo
	(struct resampler_s    *state
	,float                 *out_data
	,const float           *in_data
	,unsigned               out_rate
	,unsigned               interleaved_channels
	)
{
	unsigned filter_index = state->filter_index;
	unsigned input_index  = state->input_index;
	const float* coef     = state->coefs->coefs;
	unsigned ins_read;
	for (unsigned i = 0; i < BLOCKS_PER_FRAME; ++i)
	{
		float out1 = 0.0f;
		float out2 = 0.0f;
		float out3 = 0.0f;
		float out4 = 0.0f;
		const float *coef_set = &coef[filter_index << SUBFILTER_BITS];
		const float *in_set   = &in_data[input_index >> UPSAMPLE_BITS];
		for (unsigned j = 0; j < SUBFILTER_TAPS; j += 4)
		{
			out1 += coef_set[j] * in_set[j];
			out2 += coef_set[j+1] * in_set[j+1];
			out3 += coef_set[j+2] * in_set[j+2];
			out4 += coef_set[j+3] * in_set[j+3];
		}
		input_index += out_rate;
		filter_index = (filter_index + out_rate) & (UPSAMPLE_FACTOR - 1);
		out_data[i] = out1 + out2 + out3 + out4;
	}
	ins_read            = input_index >> UPSAMPLE_BITS;
	state->filter_index = filter_index;
	state->input_index  = input_index & (UPSAMPLE_FACTOR - 1);
	return ins_read;
}


#endif /* GOSOUNDRESAMPLE_H_ */
