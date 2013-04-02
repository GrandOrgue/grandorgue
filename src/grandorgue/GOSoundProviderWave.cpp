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

#include "GOSoundProviderWave.h"
#include "GOrgueMemoryPool.h"
#include "GOrguePath.h"
#include "GOrgueWave.h"

GOSoundProviderWave::GOSoundProviderWave(GOrgueMemoryPool& pool) :
	GOSoundProvider(pool)
{
}

void GOSoundProviderWave::SetAmplitude(float fixed_amplitude, float gain)
{
	/* Amplitude is the combination of global amplitude volume and the stop
	 * volume. 1000000 would correspond to sample playback at normal volume.
	 */
	m_Gain = fixed_amplitude / 1000000.0f * powf(10.0f, gain * 0.05f);
}

unsigned GOSoundProviderWave::GetBytesPerSample(unsigned bits_per_sample)
{
	if (bits_per_sample <= 8)
		return 1;
	else if (bits_per_sample <= 16)
		return 2;
	else
		return 3;
}

void GOSoundProviderWave::CreateAttack(const char* data, GOrgueWave& wave, std::vector<GO_WAVE_LOOP> loop_list, int sample_group, unsigned bits_per_sample, unsigned channels, bool compress, loop_load_type loop_mode, 
				       bool percussive, unsigned min_attack_velocity)
{
	std::vector<GO_WAVE_LOOP> loops;
	if (loop_list.size() == 0)
		for(unsigned i = 0; i < wave.GetNbLoops(); i++)
			loop_list.push_back(wave.GetLoop(i));

	for(unsigned i = 0; i < loop_list.size(); i++)
		if (loop_list[i].start_sample >= wave.GetLength() ||
		    loop_list[i].start_sample >= loop_list[i].end_sample ||
		    loop_list[i].end_sample >= wave.GetLength())
			throw (wxString)_("Invalid loop defintion");

	if ((loop_list.size() > 0) && !percussive)
	{
		switch (loop_mode)
		{
		case LOOP_LOAD_ALL:
			for (unsigned i = 0; i < loop_list.size(); i++)
				loops.push_back(loop_list[i]);
			break;
		case LOOP_LOAD_CONSERVATIVE:
			{
				unsigned cidx = 0;
				for (unsigned i = 1; i < loop_list.size(); i++)
					if (loop_list[i].end_sample < loop_list[i].end_sample)
						cidx = i;
				loops.push_back(loop_list[cidx]);
			}
			break;
		default:
			{
				assert(loop_mode == LOOP_LOAD_LONGEST);

				unsigned lidx = 0;
				for (unsigned int i = 1; i < loop_list.size(); i++)
				{
					assert(loop_list[i].end_sample > loop_list[i].start_sample);
					if ((loop_list[i].end_sample - loop_list[i].start_sample) >
					    (loop_list[lidx].end_sample - loop_list[lidx].start_sample))
						lidx = i;
				}
				loops.push_back(loop_list[lidx]);
			}
		}
	}

	attack_section_info attack_info;
	attack_info.sample_group = sample_group;
	attack_info.min_attack_velocity = min_attack_velocity;
	m_AttackInfo.push_back(attack_info);
	GOAudioSection* section = new GOAudioSection(m_pool);
	m_Attack.push_back(section);
	section->Setup
		(data
		 ,(GOrgueWave::SAMPLE_FORMAT)bits_per_sample
		 ,channels
		 ,wave.GetSampleRate()
		 ,wave.GetLength()
		 ,&loops
		 ,compress
		 );
}

void GOSoundProviderWave::CreateRelease(const char* data, GOrgueWave& wave, int sample_group, unsigned max_playback_time, int cue_point, unsigned bits_per_sample, unsigned channels, bool compress)
{
	unsigned release_offset = wave.HasReleaseMarker() ? wave.GetReleaseMarkerPosition() : 0;
	if (cue_point != -1)
		release_offset = cue_point;
	unsigned release_samples = wave.GetLength() - release_offset;
			
	if (release_offset >= wave.GetLength())
		throw (wxString)_("Invalid release position");

	release_section_info release_info;
	release_info.sample_group = sample_group;
	release_info.max_playback_time = max_playback_time;
	m_ReleaseInfo.push_back(release_info);
	GOAudioSection* section = new GOAudioSection(m_pool);
	m_Release.push_back(section);
	section->Setup
		(data + release_offset * GetBytesPerSample(bits_per_sample) * channels
		 ,(GOrgueWave::SAMPLE_FORMAT)bits_per_sample
		 ,channels
		 ,wave.GetSampleRate()
		 ,release_samples
		 ,NULL
		 ,compress
		 );
}

#define FREE_AND_NULL(x) do { if (x) { free(x); x = NULL; } } while (0)
#define DELETE_AND_NULL(x) do { if (x) { delete x; x = NULL; } } while (0)

void GOSoundProviderWave::ProcessFile(wxString filename, wxString path, std::vector<GO_WAVE_LOOP> loops, bool is_attack, bool is_release, int sample_group, unsigned max_playback_time, 
				      int cue_point, unsigned bits_per_sample, int load_channels, bool compress, loop_load_type loop_mode, bool percussive, unsigned min_attack_velocity)
{
	wxLogDebug(_("Loading file %s"), filename.c_str());

	GOrgueWave wave;
	wave.Open(GOCreateFilename(path, filename));

	/* allocate data to work with */
	unsigned totalDataSize = wave.GetLength() * GetBytesPerSample(bits_per_sample) * wave.GetChannels();
	char* data = (char*)malloc(totalDataSize);
	if (data == NULL)
		throw GOrgueOutOfMemory();

	m_MidiKeyNumber = wave.GetMidiNote();
	m_MidiPitchFract = wave.GetPitchFract();

	unsigned channels = wave.GetChannels();
	if (load_channels == 1)
		channels = 1;
	unsigned wave_channels = channels;
	if (load_channels < 0 && (unsigned)-load_channels <= wave.GetChannels())
	{
		wave_channels = load_channels;
		channels = 1;
	}
	if (bits_per_sample > wave.GetBitsPerSample())
		bits_per_sample = wave.GetBitsPerSample();

	try
	{
		wave.ReadSamples(data, (GOrgueWave::SAMPLE_FORMAT)bits_per_sample, wave.GetSampleRate(), wave_channels);

		if (is_attack)
			CreateAttack(data, wave, loops, sample_group, bits_per_sample, channels, compress, loop_mode, percussive, min_attack_velocity);

		if (is_release && (!is_attack || (wave.GetNbLoops() > 0 && wave.HasReleaseMarker() && !percussive)))
			CreateRelease(data, wave, sample_group, max_playback_time, cue_point, bits_per_sample, channels, compress);

		/* data is no longer needed */
		FREE_AND_NULL(data);
	}
	catch (...)
	{
		FREE_AND_NULL(data);
		throw;
	}
}

void GOSoundProviderWave::LoadFromFile
	(std::vector<attack_load_info> attacks
	,std::vector<release_load_info> releases
	,wxString       path
	,unsigned       bits_per_sample
	,int            load_channels
	,bool           compress
	,loop_load_type loop_mode
	,unsigned attack_load
	,unsigned release_load
	,int midi_key_number
	)
{

	ClearData();
	if (!load_channels)
		return;

	if (!release_load)
		for(int k = -1; k < 2; k++)
		{
			unsigned longest = 0;
			for(unsigned i = 0; i < attacks.size(); i++)
				if (attacks[i].load_release && (unsigned)attacks[i].max_playback_time > longest && attacks[i].sample_group == k)
					longest = attacks[i].max_playback_time;
			for(unsigned i = 0; i < releases.size(); i++)
				if ((unsigned)releases[i].max_playback_time > longest && releases[i].sample_group == k)
					longest = releases[i].max_playback_time;

			bool found = false;
			for(unsigned i = 0; i < attacks.size(); i++)
				if (attacks[i].load_release && attacks[i].sample_group == k)
				{
					if ((unsigned)attacks[i].max_playback_time == longest && !found)
					{
						found = true;
						continue;
					}
					else
						attacks[i].load_release = false;
				}
			for(unsigned i = 0; i < releases.size(); i++)
				if (releases[i].sample_group == k)
				{
					if ((unsigned)releases[i].max_playback_time == longest && !found)
					{
						found = true;
						continue;
					}
					else
					{
						releases[i] = releases[releases.size() - 1];
						releases.resize(releases.size() - 1);
						i--;
					}
				}
		}

	if (!attack_load)
		for(int k = -1; k < 2; k++)
		{
			int best_idx = -1;
			unsigned min_velocity = 0xff;
			for(unsigned i = 0; i < attacks.size(); i++)
			{
				if (attacks[i].sample_group != k)
					continue;
				if (attacks[i].min_attack_velocity < min_velocity)
					min_velocity = attacks[i].min_attack_velocity;
				if (best_idx == -1)
				{
					best_idx = i;
					continue;
				}
				if (attacks[i].load_release && !attacks[best_idx].load_release)
					best_idx = i;
			}
			for(unsigned i = 0; i < attacks.size(); i++)
			{
				if (attacks[i].sample_group == k)
					attacks[i].min_attack_velocity = min_velocity;
				if (attacks[i].sample_group != k || best_idx == -1 || best_idx == (int)i)
					continue;
				for(unsigned j = i + 1; j < attacks.size(); j++)
					attacks[j - 1] = attacks[j];
				if ((int)i < best_idx)
					best_idx --;
				attacks.resize(attacks.size() - 1);
				i--;
			}
		}

	try
	{
		for(unsigned i = 0; i < attacks.size(); i++)
		{
			std::vector<GO_WAVE_LOOP> loops;
			for(unsigned j = 0; j < attacks[i].loops.size(); j++)
			{
				GO_WAVE_LOOP loop;
				loop.start_sample = attacks[i].loops[j].loop_start;
				loop.end_sample = attacks[i].loops[j].loop_end;
				loops.push_back(loop);
			}
			ProcessFile(attacks[i].filename, path, loops, true, attacks[i].load_release, attacks[i].sample_group, attacks[i].max_playback_time, attacks[i].cue_point,
				    bits_per_sample, load_channels, compress, loop_mode, attacks[i].percussive, attacks[i].min_attack_velocity);
		}

		for(unsigned i = 0; i < releases.size(); i++)
		{
			std::vector<GO_WAVE_LOOP> loops;
			ProcessFile(releases[i].filename, path, loops, false, true, releases[i].sample_group, releases[i].max_playback_time, releases[i].cue_point, 
				    bits_per_sample, load_channels, compress, loop_mode, true, 0);
		}

		ComputeReleaseAlignmentInfo();
		if (midi_key_number != -1)
		{
			m_MidiKeyNumber = midi_key_number;
			m_MidiPitchFract = 0;
		}
	}
	catch (wxString error)
	{
		wxLogError(_("caught exception: %s\n"), error.c_str());
		ClearData();
		throw;
	}
	catch (...)
	{
		ClearData();
		throw;
	}

}
