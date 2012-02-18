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

void GOSoundProviderWave::SetAmplitude(int fixed_amplitude)
{
	/* Amplitude is the combination of global amplitude volume and the stop
	 * volume. 1000000 would correspond to sample playback at normal volume.
	 */
	m_Gain                     = fixed_amplitude / 1000000.0f;
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

void GOSoundProviderWave::CreateAttack(const char* data, GOrgueWave& wave, unsigned bits_per_sample, unsigned channels, bool compress, loop_load_type loop_mode, bool percussive)
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

void GOSoundProviderWave::CreateRelease(const char* data, GOrgueWave& wave, unsigned bits_per_sample, unsigned channels, bool compress)
{
	const unsigned release_offset = wave.HasReleaseMarker() ? wave.GetReleaseMarkerPosition() : 0;
	const unsigned release_samples = wave.GetLength() - release_offset;
			
	if (release_offset >= wave.GetLength())
		throw (wxString)_("Invalid release position");

	release_section_info release_info;
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

void GOSoundProviderWave::ProcessFile(wxString filename, wxString path, bool is_attack, bool is_release, unsigned bits_per_sample, unsigned load_channels, 
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
			CreateAttack(data, wave, bits_per_sample, channels, compress, loop_mode, !wave.HasReleaseMarker());

		if (is_release && (!is_attack || (wave.GetNbLoops() > 0 && wave.HasReleaseMarker() && !percussive)))
			CreateRelease(data, wave, bits_per_sample, channels, compress);

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
	(wxString       filename
	,wxString       path
	,unsigned       bits_per_sample
	,unsigned       load_channels
	,bool           compress
	,loop_load_type loop_mode
	)
{

	ClearData();
	if (!load_channels)
		return;

	try
	{
		ProcessFile(filename, path, true, true, bits_per_sample, load_channels, compress, loop_mode, false);

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
