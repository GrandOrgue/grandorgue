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
 * MA 02111-1307, USA.
 */

#include "GOSoundProviderWave.h"
#include "GOrgueMemoryPool.h"
#include "GOrgueWave.h"
#include <wx/filename.h>

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

void GOSoundProviderWave::CreateAttack(const char* data, GOrgueWave& wave, int sample_group, unsigned bits_per_sample, unsigned channels, bool compress, loop_load_type loop_mode, bool percussive)
{
	std::vector<GO_WAVE_LOOP> loops;
	if ((wave.GetNbLoops() > 0) && !percussive)
	{
		switch (loop_mode)
		{
		case LOOP_LOAD_ALL:
			for (unsigned i = 0; i < wave.GetNbLoops(); i++)
				loops.push_back(wave.GetLoop(i));
			break;
		case LOOP_LOAD_CONSERVATIVE:
		{
			unsigned cidx = 0;
			for (unsigned i = 1; i < wave.GetNbLoops(); i++)
				if (wave.GetLoop(i).end_sample < wave.GetLoop(cidx).end_sample)
					cidx = i;
			loops.push_back(wave.GetLoop(cidx));
		}
		break;
		default:
			assert(loop_mode == LOOP_LOAD_LONGEST);
			loops.push_back(wave.GetLongestLoop());
		}
	}

	attack_section_info attack_info;
	attack_info.sample_group = sample_group;
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

void GOSoundProviderWave::CreateRelease(const char* data, GOrgueWave& wave, int sample_group, unsigned max_playback_time, unsigned bits_per_sample, unsigned channels, bool compress)
{
	/* force release_offset = 0 when wave has no release marker or when wave has no loops as it assumes wave is a release.*/
	const unsigned release_offset = (wave.HasReleaseMarker() && wave.GetNbLoops()>0 ) ? wave.GetReleaseMarkerPosition() : 0;
	const unsigned release_samples = wave.GetLength() - release_offset;
			
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

void GOSoundProviderWave::ProcessFile(wxString filename, wxString path, bool is_attack, bool is_release, int sample_group, unsigned max_playback_time, unsigned bits_per_sample, unsigned load_channels, 
				      bool compress, loop_load_type loop_mode, bool percussive)
{
	wxLogDebug(_("Loading file %s"), filename.c_str());

	/* Translate directory seperator from ODF(\) to native format */
	wxString temp = filename;
	temp.Replace(wxT("\\"), wxString(wxFileName::GetPathSeparator()));
	temp = path + wxFileName::GetPathSeparator() + temp;

	GOrgueWave wave;
	wave.Open(temp);

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

	try
	{
		wave.ReadSamples(data, (GOrgueWave::SAMPLE_FORMAT)bits_per_sample, wave.GetSampleRate(), channels);

		if (is_attack)
			CreateAttack(data, wave, sample_group, bits_per_sample, channels, compress, loop_mode, percussive);

		if (is_release && (!is_attack || (wave.GetNbLoops() > 0 && wave.HasReleaseMarker() && !percussive)))
			CreateRelease(data, wave, sample_group, max_playback_time, bits_per_sample, channels, compress);

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
	,unsigned       load_channels
	,bool           compress
	,loop_load_type loop_mode
	,unsigned attack_load
	,unsigned release_load
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
			for(unsigned i = 0; i < attacks.size(); i++)
			{
				if (attacks[i].sample_group != k)
					continue;
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
			ProcessFile(attacks[i].filename, path, true, attacks[i].load_release, attacks[i].sample_group, attacks[i].max_playback_time, 
				    bits_per_sample, load_channels, compress, loop_mode, attacks[i].percussive);

		for(unsigned i = 0; i < releases.size(); i++)
			ProcessFile(releases[i].filename, path, false, true, releases[i].sample_group, releases[i].max_playback_time, bits_per_sample, load_channels, compress, loop_mode, true);

		ComputeReleaseAlignmentInfo();
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
