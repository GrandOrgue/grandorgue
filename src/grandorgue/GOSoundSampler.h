/*
 * GrandOrgue - free pipe organ simulator
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

#ifndef GOSOUNDSAMPLER_H_
#define GOSOUNDSAMPLER_H_

#include "GOSoundAudioSection.h"
#include "GOSoundDefs.h"
#include "GOSoundFader.h"

class GOSoundProvider;

typedef struct GO_SAMPLER_T
{
	struct GO_SAMPLER_T       *next;
	const GOSoundProvider     *pipe;
	int                        sampler_group_id;
	unsigned                   audio_group_id;
	audio_section_stream       stream;
	GOSoundFader               fader;
	uint64_t                   time;
	unsigned                   velocity;
	unsigned                   delay;
	/* current index of the current block into this sample */
	volatile unsigned long     stop;
	volatile unsigned long     new_attack;
	bool                       is_release;
} GO_SAMPLER;

#endif /* GOSOUNDSAMPLER_H_ */
