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

#ifndef GOSOUNDPROVIDERWAVE_H_
#define GOSOUNDPROVIDERWAVE_H_

#include "GOSoundProvider.h"
#include <wx/wx.h>

typedef enum
{
	/* Only the first loop with the earliest endpoint is loaded. This will
	 * result in the minimal amount of memory being loaded for the sample.
	 */
	LOOP_LOAD_CONSERVATIVE,

	/* Only the longest loop will be loaded. This only provides a benefit if
	 * the longest loop is not the last loop found.
	 */
	LOOP_LOAD_LONGEST,

	/* Stores all samples up to the very last loop end-point. Uses the most
	 * memory and should achieve best realism.
	 */
	LOOP_LOAD_ALL
} loop_load_type;

class GOSoundProviderWave : public GOSoundProvider
{
	void Compress(GOAudioSection& section, bool format16);

public:
	GOSoundProviderWave(GOrgueMemoryPool& pool);

	void LoadFromFile(wxString filename, wxString path, unsigned bits_per_sample, bool stereo, bool compress, loop_load_type loop_mode);
	void SetAmplitude(int fixed_amplitude);
};

#endif /* GOSOUNDPROVIDERWAVE_H_ */
