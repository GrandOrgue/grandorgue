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

#include "GOSoundAudioSectionAccessor.h"
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

void GOSoundProviderWave::Compress(GOAudioSection& section, bool format16)
{
	unsigned char* data = (unsigned char*)malloc(section.m_AllocSize);
	unsigned output_len = 0;
	if (!data)
		return;

	int diff[MAX_OUTPUT_CHANNELS];
	int last[MAX_OUTPUT_CHANNELS];
	memset(diff, 0, sizeof(diff));
	memset(last, 0, sizeof(last));

	unsigned channels = section.GetChannels();
	for(unsigned i = 0; i < section.m_SampleCount + EXTRA_FRAMES; i++)
		for(unsigned j = 0; j < channels; j++)
		{
			int val = GetAudioSectionSample(section, i, j);
			int encode = val - last[j];
			diff[j] = (diff[j] + val - last[j]) / 2;
			last[j] = val + diff[j];

			if (format16)
				AudioWriteCompressed16(data, output_len, encode);
			else
				AudioWriteCompressed8(data, output_len, encode);

			if (output_len + 10 >= section.m_AllocSize)
			{
				free(data);
				return;
			}
		}

	AUDIO_SECTION_TYPE type;
	if (channels == 2)
		type = format16 ? AC_COMPRESSED16_STEREO : AC_COMPRESSED8_STEREO;
	else
		type = format16 ? AC_COMPRESSED16_MONO : AC_COMPRESSED8_MONO;

	if (false) 	/* Verifcation of compression code */
	{
		GOAudioSection new_section = section;
		new_section.m_Data = data;
		new_section.m_Size = output_len;
		new_section.m_AllocSize = output_len;
		DecompressionCache tmp;
		InitDecompressionCache(tmp);
		new_section.m_Type = type;

		for(unsigned i = 0; i < section.m_SampleCount + EXTRA_FRAMES; i++)
			for(unsigned j = 0; j < channels; j++)
			{
				int old_value = GetAudioSectionSample(section, i, j);
				int new_value = GetAudioSectionSample(new_section, i, j, &tmp);
				if (old_value != new_value)
					wxLogError(wxT("%d %d: %d %d"), i, j, old_value, new_value);
			}
	
		wxLogError(wxT("Compress: %d %d"), section.m_AllocSize, output_len);
	}

	section.m_Data = (unsigned char*)m_pool.Realloc(section.m_Data, section.m_AllocSize, output_len);
	memcpy(section.m_Data, data, output_len);
	section.m_Size = output_len;
	section.m_AllocSize = output_len;
	section.m_Type = type;
	free(data);
}

void GOSoundProviderWave::LoadFromFile
	(wxString filename
	,wxString path
	,unsigned bits_per_sample
	,bool stereo
	,bool compress
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
	unsigned channels = wave.GetChannels();
	if (!stereo)
		channels = 1;

	m_Attack.m_SampleFracBits    = bits_per_sample - 1;
	m_Attack.m_Stage             = GSS_ATTACK;
	m_Attack.m_Type              = GetAudioSectionType(bytes_per_sample, channels);
	m_Loop.m_SampleFracBits      = bits_per_sample - 1;
	m_Loop.m_Stage               = GSS_LOOP;
	m_Loop.m_Type                = GetAudioSectionType(bytes_per_sample, channels);
	m_Release.m_SampleFracBits   = bits_per_sample - 1;
	m_Release.m_Stage            = GSS_RELEASE;
	m_Release.m_Type             = GetAudioSectionType(bytes_per_sample, channels);

	try
	{
		wave.ReadSamples(data, (GOrgueWave::SAMPLE_FORMAT)bits_per_sample, m_SampleRate, channels);

		if (channels < 1 || channels > 2)
			throw (wxString)_("< More than 2 channels in");

		/* Basically, sample playback reads BLOCKS_PER_FRAME * 2 samples at a
		 * time (because the engine always plays back in stereo at present),
		 * which means that if the loop ranges, attack segment length or
		 * release segment length is not a multiple of BLOCKS_PER_FRAME, the
		 * sound will crackle. The following code is used to ensure that each
		 * data range will always have enough samples for their intended
		 * purposes.
		 */
		unsigned attackSamples = wave.GetLength();
		if ((wave.GetNbLoops() > 0) && wave.HasReleaseMarker())
		{

			/* The wave has loops and a release marker so truncate the samples
			 * stored in the attack portion to the beginning of the loop.
			 */
			attackSamples = wave.GetLongestLoop().start_sample;;

			/* Get the loop parameters */
			unsigned loopStart = attackSamples;
			unsigned loopSamples = wave.GetLongestLoop().end_sample - loopStart + 1;
			unsigned loopSamplesInMem = loopSamples + EXTRA_FRAMES;
			assert(loopStart > 0);
			assert(wave.GetLongestLoop().end_sample > loopStart);

			/* Allocate memory for the loop, copy the loop into it and then
			 * copy some slack samples from the beginning of the loop onto
			 * the end to ensure correct operation of the sampler.
			 */
			m_Loop.m_Size = loopSamples * bytes_per_sample * channels;
			m_Loop.m_AllocSize = loopSamplesInMem * bytes_per_sample * channels;
			m_Loop.m_Data = (unsigned char*)m_pool.Alloc(m_Loop.m_AllocSize);
			if (m_Loop.m_Data == NULL)
				throw (wxString)_("< out of memory allocating loop");
			m_Loop.m_SampleRate = m_SampleRate;
			m_Loop.m_SampleCount = loopSamples;

			memcpy
				(m_Loop.m_Data
				,data + bytes_per_sample * loopStart * channels
				,m_Loop.m_Size
				);
			memcpy
				(&m_Loop.m_Data[m_Loop.m_Size]
				,data + bytes_per_sample * loopStart * channels
				,loopSamplesInMem * bytes_per_sample * channels - m_Loop.m_Size
				);

			if (compress && bytes_per_sample == 3)
				Compress(m_Loop, true);
			if (compress && bytes_per_sample == 2)
				Compress(m_Loop, false);

			/* Get the release parameters from the wave file. */
			unsigned releaseOffset = wave.GetReleaseMarkerPosition();
			unsigned releaseSamples = wave.GetLength() - releaseOffset;
			unsigned releaseSamplesInMem = releaseSamples + EXTRA_FRAMES;

			/* Allocate memory for the release, copy the release into it and
			 * pad the slack samples with zeroes to ensure correct operation
			 * of the sampler.
			 */
			m_Release.m_Size = releaseSamples * bytes_per_sample * channels;
			m_Release.m_AllocSize = releaseSamplesInMem * bytes_per_sample * channels;
			m_Release.m_Data = (unsigned char*)m_pool.Alloc(m_Release.m_AllocSize);
			if (m_Release.m_Data == NULL)
				throw (wxString)_("< out of memory allocating release");
			m_Release.m_SampleRate = m_SampleRate;
			m_Release.m_SampleCount = releaseSamples;

			memcpy
				(m_Release.m_Data
				,data + bytes_per_sample * (wave.GetLength() - releaseSamples) * channels
				,m_Release.m_Size
				);
			memset
				(&m_Release.m_Data[m_Release.m_Size]
				,0
				,releaseSamplesInMem * bytes_per_sample * channels - m_Release.m_Size
				);

			if (compress && bytes_per_sample == 3)
				Compress(m_Release, true);
			if (compress && bytes_per_sample == 2)
				Compress(m_Release, false);
		}

		/* Allocate memory for the attack. */
		assert(attackSamples != 0);
		unsigned attackSamplesInMem = attackSamples + EXTRA_FRAMES;
		m_Attack.m_Size = attackSamples * bytes_per_sample * channels;
		m_Attack.m_AllocSize = attackSamplesInMem * bytes_per_sample * channels;
		assert((unsigned)m_Attack.m_Size <= totalDataSize); /* can be equal for percussive samples */
		m_Attack.m_Data = (unsigned char*)m_pool.Alloc(m_Attack.m_AllocSize);
		if (m_Attack.m_Data == NULL)
			throw (wxString)_("< out of memory allocating attack");
		m_Attack.m_SampleRate = m_SampleRate;
		m_Attack.m_SampleCount = attackSamples;

		if (attackSamplesInMem <= wave.GetLength())
		{
			memcpy
				(m_Attack.m_Data
				,data
				,attackSamplesInMem * bytes_per_sample * channels
				);
		}
		else
		{
			memset
				(m_Attack.m_Data
				,0
				,(attackSamplesInMem - wave.GetLength()) * bytes_per_sample * channels
				);
			memcpy
				(&m_Attack.m_Data[(attackSamplesInMem - wave.GetLength()) * bytes_per_sample * channels]
				,data
				,totalDataSize
				);
		}

		if (compress && bytes_per_sample == 3)
			Compress(m_Attack, true);
		if (compress && bytes_per_sample == 2)
			Compress(m_Attack, false);

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
