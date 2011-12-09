/*
 * GrandOrgue - free pipe organ simulator
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef GOSOUNDSAMPLER_H_
#define GOSOUNDSAMPLER_H_

#include "GOSoundFader.h"
#include "GOSoundDefs.h"

class GOSoundProvider;
struct AUDIO_SECTION_T;
typedef struct AUDIO_SECTION_T AUDIO_SECTION;

typedef struct GO_SAMPLER_T
{
	struct GO_SAMPLER_T       *next;
	const GOSoundProvider     *pipe;
	int                        sampler_group_id;
	const AUDIO_SECTION       *pipe_section;
	GOSoundFader               fader;
	int                        history[BLOCK_HISTORY][MAX_OUTPUT_CHANNELS];
	unsigned                   time;
	/* current index of the current block into this sample */
	float                      position;
	float                      increment;
	bool                       stop;
	/* for decoding compressed format */
	unsigned                   last_position;
	const unsigned char*       ptr;
	int                        last_value[MAX_OUTPUT_CHANNELS];
	int                        diff_value[MAX_OUTPUT_CHANNELS];
	int                        curr_value[MAX_OUTPUT_CHANNELS];
	int                        next_value[MAX_OUTPUT_CHANNELS];
} GO_SAMPLER;

#endif /* GOSOUNDSAMPLER_H_ */
