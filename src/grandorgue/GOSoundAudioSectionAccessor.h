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

#ifndef GOSOUNDAUDIOSECTIONACCESSOR_H_
#define GOSOUNDAUDIOSECTIONACCESSOR_H_

#include "GOSoundAudioSection.h"
#include "GOSoundCompress.h"
#include "GOrgueInt24.h"
#include <wx/wx.h>

static inline AUDIO_SECTION_TYPE GetAudioSectionType(unsigned bytes_per_sample, unsigned channels)
{
	if (channels == 1 && bytes_per_sample == sizeof(wxInt8))
		return AC_UNCOMPRESSED8_MONO;
	if (channels == 2 && bytes_per_sample == sizeof(wxInt8))
		return AC_UNCOMPRESSED8_STEREO;
	if (channels == 1 && bytes_per_sample == sizeof(wxInt16))
		return AC_UNCOMPRESSED16_MONO;
	if (channels == 2 && bytes_per_sample == sizeof(wxInt16))
		return AC_UNCOMPRESSED16_STEREO;
	if (channels == 1 && bytes_per_sample == sizeof(Int24))
		return AC_UNCOMPRESSED24_MONO;
	if (channels == 2 && bytes_per_sample == sizeof(Int24))
		return AC_UNCOMPRESSED24_STEREO;

	assert(0 && "Invalid sample block");
	return AC_UNCOMPRESSED8_MONO;
}

static inline int GetAudioSectionSample(const GOAudioSection& release, unsigned position, unsigned channel, DecompressionCache* cache = NULL)
{
	switch (release.type)
	{
		case AC_UNCOMPRESSED8_STEREO:
		{
			typedef wxInt8 stereoSample[0][2];
	
			stereoSample& samples = (stereoSample&)*(wxInt8*)release.data; 
			return samples[position][channel];
		}

		case AC_UNCOMPRESSED8_MONO:
		{
			wxInt8* samples = ((wxInt8*)release.data);
			return samples[position];
		}
		case AC_UNCOMPRESSED16_STEREO:
		{
			typedef wxInt16 stereoSample[0][2];
	
			stereoSample& samples = (stereoSample&)*(wxInt16*)release.data; 
			return samples[position][channel];
		}

		case AC_UNCOMPRESSED16_MONO:
		{
			wxInt16* samples = ((wxInt16*)release.data);
			return samples[position];
		}
		case AC_UNCOMPRESSED24_STEREO:
		{
			typedef Int24 stereoSample[0][2];
	
			stereoSample& samples = (stereoSample&)*(Int24*)release.data; 
			return samples[position][channel];
		}

		case AC_UNCOMPRESSED24_MONO:
		{
			Int24* samples = ((Int24*)release.data);
			return samples[position];
		}

		case AC_COMPRESSED8_MONO:
		{
			DecompressionCache tmp;
			if (!cache)
			{
				cache = &tmp;
				InitDecompressionCache(*cache);
			}
			DecompressTo(*cache, position, release.data, 1, false);
			return cache->value[0];
		}

		case AC_COMPRESSED8_STEREO:
		{
			DecompressionCache tmp;
			if (!cache)
			{
				cache = &tmp;
				InitDecompressionCache(*cache);
			}
			DecompressTo(*cache, position, release.data, 2, false);
			return cache->value[channel];
		}

		case AC_COMPRESSED16_MONO:
		{
			DecompressionCache tmp;
			if (!cache)
			{
				cache = &tmp;
				InitDecompressionCache(*cache);
			}
			DecompressTo(*cache, position, release.data, 1, true);
			return cache->value[0];
		}

		case AC_COMPRESSED16_STEREO:
		{
			DecompressionCache tmp;
			if (!cache)
			{
				cache = &tmp;
				InitDecompressionCache(*cache);
			}
			DecompressTo(*cache, position, release.data, 2, true);
			return cache->value[channel];
		}

		default:
			assert(0 && "broken sampler type");
			return 1;
	}
}


#endif /* GOSOUNDAUDIOSECTIONACCESSOR_H_ */
