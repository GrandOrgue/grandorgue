/*
 * GrandOrgue - free pipe organ simulator
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

#ifndef GOSOUNDAUDIOSECTION_H_
#define GOSOUNDAUDIOSECTION_H_

#include "GOSoundDefs.h"
#include "GOrgueInt24.h"
#include <assert.h>
#include <wx/wx.h>

typedef enum
{
	AC_UNCOMPRESSED8_MONO = 0,
	AC_UNCOMPRESSED8_STEREO,
	AC_UNCOMPRESSED16_MONO,
	AC_UNCOMPRESSED16_STEREO,
	AC_UNCOMPRESSED24_MONO,
	AC_UNCOMPRESSED24_STEREO,
	AC_COMPRESSED8_MONO,
	AC_COMPRESSED8_STEREO,
	AC_COMPRESSED16_MONO,
	AC_COMPRESSED16_STEREO,
} AUDIO_SECTION_TYPE;

typedef enum {
	GSS_ATTACK = 0,
	GSS_LOOP,
	GSS_RELEASE
} AUDIO_SECTION_STAGE;

class GOrgueReleaseAlignTable;

class GOAudioSection
{

public:
	/* Size of the section in BYTES */
	unsigned                   size;
	unsigned                   alloc_size;
	unsigned                   sample_count;
	unsigned                   sample_rate;

	/* Type of the data which is stored in the data pointer */
	AUDIO_SECTION_TYPE         type;
	AUDIO_SECTION_STAGE        stage; /*overflowing,*/

	/* The starting sample and derivatives for each channel (used in the
	 * compression and release-alignment schemes */
	int                        history[BLOCK_HISTORY][MAX_OUTPUT_CHANNELS];

	/* Pointer to (size) bytes of data encoded in the format (type) */
	unsigned char             *data;

	/* If this is a release section, it may contain an alignment table */
	GOrgueReleaseAlignTable   *release_aligner;

	/* Number of significant bits in the decoded sample data */
	unsigned                   sample_frac_bits;

	unsigned GetChannels() const;
	unsigned GetBytesPerSample() const;

};

inline
unsigned GOAudioSection::GetChannels() const
{
	switch (type)
	{
		case AC_COMPRESSED8_STEREO:
		case AC_COMPRESSED16_STEREO:
		case AC_UNCOMPRESSED8_STEREO:
		case AC_UNCOMPRESSED16_STEREO:
		case AC_UNCOMPRESSED24_STEREO:
			return 2;
		case AC_COMPRESSED8_MONO:
		case AC_COMPRESSED16_MONO:
		case AC_UNCOMPRESSED8_MONO:
		case AC_UNCOMPRESSED16_MONO:
		case AC_UNCOMPRESSED24_MONO:
			return 1;
		default:
			assert(0 && "broken sampler type");
			return 1;
	}
}

inline
unsigned GOAudioSection::GetBytesPerSample() const
{
	switch (type)
	{
		case AC_UNCOMPRESSED8_STEREO:
		case AC_UNCOMPRESSED8_MONO:
			return sizeof(wxInt8);
		case AC_UNCOMPRESSED16_STEREO:
		case AC_UNCOMPRESSED16_MONO:
			return sizeof(wxInt16);
		case AC_UNCOMPRESSED24_STEREO:
		case AC_UNCOMPRESSED24_MONO:
			return sizeof(Int24);

		case AC_COMPRESSED8_STEREO:
		case AC_COMPRESSED8_MONO:
		case AC_COMPRESSED16_STEREO:
		case AC_COMPRESSED16_MONO:
			return 0;

		default:
			assert(0 && "broken sampler type");
			return 1;
	}
}

#endif /* GOSOUNDAUDIOSECTION_H_ */
