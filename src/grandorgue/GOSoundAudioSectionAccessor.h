/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
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

#ifndef GOSOUNDAUDIOSECTIONACCESSOR_H_
#define GOSOUNDAUDIOSECTIONACCESSOR_H_

#include "GOSoundAudioSection.h"
#include <wx/wx.h>

static inline AUDIO_SECTION_TYPE GetAudioSectionType(unsigned bytes_per_sample, unsigned channels)
{
	if (channels == 1 && bytes_per_sample == sizeof(wxInt16))
		return AC_UNCOMPRESSED_MONO;
	if (channels == 2 && bytes_per_sample == sizeof(wxInt16))
		return AC_UNCOMPRESSED_STEREO;

	assert(0 && "Invalid sample block");
	return AC_UNCOMPRESSED_MONO;
}

static inline unsigned GetAudioSectionChannelCount(const AUDIO_SECTION& release)
{
	switch (release.type)
	{
		case AC_COMPRESSED_STEREO:
		case AC_UNCOMPRESSED_STEREO:
			return 2;
		case AC_COMPRESSED_MONO:
		case AC_UNCOMPRESSED_MONO:
			return 1;
		default:
			assert(0 && "broken sampler type");
			return 1;
	}
}

static inline int GetAudioSectionSample(const AUDIO_SECTION& release, unsigned position, unsigned channel)
{
	switch (release.type)
	{
		case AC_UNCOMPRESSED_STEREO:
		{
			typedef wxInt16 stereoSample[0][2];
	
			stereoSample& samples = (stereoSample&)*(wxInt16*)release.data; 
			return samples[position][channel];
		}

		case AC_UNCOMPRESSED_MONO:
		{
			wxInt16* samples = ((wxInt16*)release.data);
			return samples[position];
		}

		case AC_COMPRESSED_STEREO:
		case AC_COMPRESSED_MONO:
			assert(0 && "not supported sampler type");
			return 1;
		default:
			assert(0 && "broken sampler type");
			return 1;
	}
}


#endif /* GOSOUNDAUDIOSECTIONACCESSOR_H_ */
