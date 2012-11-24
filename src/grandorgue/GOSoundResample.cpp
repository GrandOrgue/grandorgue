/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2012 GrandOrgue contributors (see AUTHORS)
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

#ifndef RESAMPLE_USE_LANCZOS

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

#else

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

#endif

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
#ifndef RESAMPLE_USE_LANCZOS
	apply_gaussian_window
		(temp
		,UPSAMPLE_FACTOR * SUBFILTER_TAPS
		);
#else
	apply_lanczos_window
		(temp
		,UPSAMPLE_FACTOR * SUBFILTER_TAPS
		);
#endif
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

