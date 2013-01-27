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

#ifndef GOSOUNDDEFS_H_
#define GOSOUNDDEFS_H_

/* Number of samples to store from previous block decode as history. This may
 * be needed for features such as release alignment and compression. */
#define BLOCK_HISTORY          (2)

/* BLOCKS_PER_FRAME specifies the number of mono samples or stereo sample
 * pairs which are decoded for each iteration of the audio engines main loop.
 * Setting this value too low will result in inefficiencies or certain
 * features (compression) failing to work. */
#define BLOCKS_PER_FRAME       (16)

/* Number of extra samples to avoid out of array accesses */
#define EXTRA_FRAMES           (128)

/* Maximum number of blocks (1 block is nChannels samples) per frame */
#define MAX_FRAME_SIZE         (1024)

/* Maximum number of channels the engine supports. This value can not be
 * changed at present.
 */
#define MAX_OUTPUT_CHANNELS    (2)

#endif /* GOSOUNDDEFS_H_ */
