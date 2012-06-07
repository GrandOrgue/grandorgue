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

#ifndef GOSOUNDRESAMPLE_H_
#define GOSOUNDRESAMPLE_H_

#define RESAMPLE_USE_LANCZOS      (1)

#define SUBFILTER_BITS            (3U)
#define SUBFILTER_TAPS            (1U << SUBFILTER_BITS)
#define UPSAMPLE_BITS             (11U)
#define UPSAMPLE_FACTOR           (1U << UPSAMPLE_BITS)

/* This factor must not be exceeded in the downsampler and it MUST be
 * greater than UPSAMPLE_FACTOR. */
#define MAX_POSITIVE_FACTOR       (2300U)

typedef enum
{
	GO_LINEAR_INTERPOLATION = 0,
	GO_POLYPHASE_INTERPOLATION = 1,
} interpolation_type;

struct resampler_coefs_s
{
	float coefs[UPSAMPLE_FACTOR * SUBFILTER_TAPS];
	interpolation_type interpolation;
};

void
resampler_coefs_init
	(struct resampler_coefs_s   *resampler_coefs
	,const unsigned              input_sample_rate
	,interpolation_type          interpolation
	);

#endif /* GOSOUNDRESAMPLE_H_ */
