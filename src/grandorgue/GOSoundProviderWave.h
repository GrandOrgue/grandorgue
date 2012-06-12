/*
 * GrandOrgue - free pipe organ simulator
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

#ifndef GOSOUNDPROVIDERWAVE_H_
#define GOSOUNDPROVIDERWAVE_H_

#include "GOSoundProvider.h"
#include <wx/wx.h>
#include <vector>

class GOrgueWave;

typedef enum
{
	/* Only the first loop with the earliest endpoint is loaded. This will
	 * result in the minimal amount of memory being loaded for the sample.
	 */
	LOOP_LOAD_CONSERVATIVE = 0,

	/* Only the longest loop will be loaded. This only provides a benefit if
	 * the longest loop is not the last loop found.
	 */
	LOOP_LOAD_LONGEST = 1,

	/* Stores all samples up to the very last loop end-point. Uses the most
	 * memory and should achieve best realism.
	 */
	LOOP_LOAD_ALL = 2
} loop_load_type;

typedef struct
{
	int loop_start;
	int loop_end;
} loop_load_info;

typedef struct
{
	wxString filename;
	int sample_group;
	bool load_release;
	bool percussive;
	int max_playback_time;
	int cue_point;
	std::vector<loop_load_info> loops;
} attack_load_info;

typedef struct
{
	wxString filename;
	int sample_group;
	int max_playback_time;
	int cue_point;
} release_load_info;

class GOSoundProviderWave : public GOSoundProvider
{
	unsigned GetBytesPerSample(unsigned bits_per_sample);
       
	void CreateAttack(const char* data, GOrgueWave& wave, std::vector<GO_WAVE_LOOP> loop_list, int sample_group, unsigned bits_per_sample, unsigned channels, bool compress, loop_load_type loop_mode, bool percussive);
	void CreateRelease(const char* data, GOrgueWave& wave, int sample_group, unsigned max_playback_time, int cue_point, unsigned bits_per_sample, unsigned channels, bool compress);
	void ProcessFile(wxString filename, wxString path, std::vector<GO_WAVE_LOOP> loops, bool is_attack, bool is_release, int sample_group, unsigned max_playback_time, int cue_point, unsigned bits_per_sample, 
			 unsigned load_channels, bool compress, loop_load_type loop_mode, bool percussive);

public:
	GOSoundProviderWave(GOrgueMemoryPool& pool);

	void LoadFromFile(std::vector<attack_load_info> attacks, std::vector<release_load_info> releases, wxString path, unsigned bits_per_sample, unsigned channels, bool compress, loop_load_type loop_mode,
			  unsigned attack_load, unsigned release_load);
	void SetAmplitude(float fixed_amplitude, float gain);
};

#endif /* GOSOUNDPROVIDERWAVE_H_ */
