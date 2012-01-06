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

#define FREE_AND_NULL(x) do { if (x) { free(x); x = NULL; } } while (0)
#define DELETE_AND_NULL(x) do { if (x) { delete x; x = NULL; } } while (0)

void GOSoundProviderWave::LoadFromFile
	(wxString       filename
	,wxString       path
	,unsigned       bits_per_sample
	,bool           stereo
	,bool           compress
	,loop_load_type loop_mode
	)
{

	ClearData();

	wxLogDebug(_("Loading file %s"), filename.c_str());

	/* Translate directory seperator from ODF(\) to native format */
	wxString temp = filename;
	temp.Replace(wxT("\\"), wxString(wxFileName::GetPathSeparator()));
	temp = path + wxFileName::GetPathSeparator() + temp;

	GOrgueWave wave;
	wave.Open(temp);

	unsigned bytes_per_sample;
	if (bits_per_sample <= 8)
		bytes_per_sample = 1;
	else if (bits_per_sample <= 16)
		bytes_per_sample = 2;
	else
		bytes_per_sample = 3;

	/* allocate data to work with */
	unsigned totalDataSize = wave.GetLength() * bytes_per_sample * wave.GetChannels();
	char* data = (char*)malloc(totalDataSize);
	if (data == NULL)
		throw (wxString)_("< out of memory allocating wave");

	m_MidiKeyNumber = wave.GetMidiNote();
	m_MidiPitchFract = wave.GetPitchFract();

	m_SampleRate = wave.GetSampleRate();
	m_Gain = 1.0f;
	unsigned channels = wave.GetChannels();
	if (!stereo)
		channels = 1;

	try
	{
		wave.ReadSamples(data, (GOrgueWave::SAMPLE_FORMAT)bits_per_sample, m_SampleRate, channels);

		std::vector<GO_WAVE_LOOP> loops;
		if ((wave.GetNbLoops() > 0) && (wave.HasReleaseMarker()))
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

		m_Attack.Setup
			(data
			,(GOrgueWave::SAMPLE_FORMAT)bits_per_sample
			,channels
			,m_SampleRate
			,wave.GetLength()
			,&loops
			,compress
			);

		if (wave.HasReleaseMarker() && loops.size())
		{
			const unsigned release_offset = wave.GetReleaseMarkerPosition();
			const unsigned release_samples = wave.GetLength() - release_offset;
			m_Release.Setup
				(data + release_offset * bytes_per_sample * channels
				,(GOrgueWave::SAMPLE_FORMAT)bits_per_sample
				,channels
				,m_SampleRate
				,release_samples
				,NULL
				,compress
				);
		}

		/* data is no longer needed */
		FREE_AND_NULL(data);

		if (wave.HasReleaseMarker())
			ComputeReleaseAlignmentInfo();

	}
	catch (wxString error)
	{
		wxLogError(_("caught exception: %s\n"), error.c_str());
		FREE_AND_NULL(data);
		ClearData();
		throw;
	}
	catch (...)
	{
		FREE_AND_NULL(data);
		ClearData();
		throw;
	}

}
