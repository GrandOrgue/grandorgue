/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2014 GrandOrgue contributors (see AUTHORS)
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

#include "GOrgueWave.h"

#include "GOrgueMemoryPool.h"
#include "GOrgueInt24.h"
#include "GOrgueWaveTypes.h"
#include "GOrgueWavPack.h"
#include <wx/file.h>
#include <wx/intl.h>

bool inline CompareFourCC(GO_FOURCC fcc, const char* text)
{
	assert(text != NULL);
	assert(strlen(text) == 4);
	return ((fcc[0] == text[0]) &&
			(fcc[1] == text[1]) &&
			(fcc[2] == text[2]) &&
			(fcc[3] == text[3]));
}

void GOrgueWave::SetInvalid()
{
	m_Content = NULL;
	m_SampleData = NULL;
	m_SampleDataSize = 0;
	m_Channels = 0;
	m_BytesPerSample = 0;
	m_SampleRate = 0;
	m_MidiNote = 0;
	m_PitchFract = 0;
	m_CuePoint = -1;
	m_hasRelease = false;
	m_isPacked = false;
	m_Loops.clear();
}

GOrgueWave::GOrgueWave() :
	m_Loops()
{
	/* Start up the waveform in an invalid state */
	SetInvalid();
}

GOrgueWave::~GOrgueWave()
{
	/* Close and free any currently open wave data */
	Close();
}

void GOrgueWave::LoadFormatChunk(char* ptr, unsigned long length)
{
	/* FIXME: This could be done much more elequently */
	/* Ensure format chunk size is 16 (basic wave
	 * format chunk... no extensible data... and
	 * that the format tag is 1 */
	if (length < sizeof(GO_WAVEFORMATPCM))
		throw (wxString)_("< Invalid WAVE format chunk");

	GO_WAVEFORMATPCMEX* format = (GO_WAVEFORMATPCMEX*)ptr;
	unsigned formatCode = wxUINT16_SWAP_ON_BE(format->wf.wf.wFormatTag);
	if (formatCode != 1 && formatCode != 3)
		throw (wxString)_("< Unsupported PCM format");

	m_Channels = wxUINT16_SWAP_ON_BE(format->wf.wf.nChannels);
	m_SampleRate = wxUINT32_SWAP_ON_BE(format->wf.wf.nSamplesPerSec);

	unsigned bitsPerSample = wxUINT16_SWAP_ON_BE(format->wf.wBitsPerSample);
	if (bitsPerSample % 8)
		throw (wxString)_("< Bits per sample must be a multiple of 8 in this implementation");
	m_BytesPerSample = bitsPerSample / 8;

	if (formatCode == 3 && m_BytesPerSample != 4)
		throw (wxString)_("< Only 32bit IEEE float samples supported");
	else if (formatCode == 1 && m_BytesPerSample > 3)
		throw (wxString)_("< Unsupport PCM bit size");
}

void GOrgueWave::LoadCueChunk(char* ptr, unsigned long length)
{
	if (length < sizeof(GO_WAVECUECHUNK))
		throw (wxString)_("< Invalid CUE chunk in");

	GO_WAVECUECHUNK* cue = (GO_WAVECUECHUNK*)ptr;
	unsigned nbPoints = wxUINT32_SWAP_ON_BE(cue->dwCuePoints);
	if (length < sizeof(GO_WAVECUECHUNK) + sizeof(GO_WAVECUEPOINT) * nbPoints)
		throw (wxString)_("< Invalid CUE chunk in");

	GO_WAVECUEPOINT* points = (GO_WAVECUEPOINT*)(ptr + sizeof(GO_WAVECUECHUNK));
	m_hasRelease = (nbPoints > 0);
	if (m_hasRelease)
		m_CuePoint = 0;
	for (unsigned k = 0; k < nbPoints; k++)
	{
		assert(m_Channels != 0);
		unsigned position = wxUINT32_SWAP_ON_BE(points[k].dwSampleOffset);
		if (position > m_CuePoint)
			m_CuePoint = position;
	}
}

void GOrgueWave::LoadSamplerChunk(char* ptr, unsigned long length)
{
	if (length < sizeof(GO_WAVESAMPLERCHUNK))
		throw (wxString)_("< Invalid SMPL chunk in");

	GO_WAVESAMPLERCHUNK* sampler = (GO_WAVESAMPLERCHUNK*)ptr;
	unsigned numberOfLoops = wxUINT32_SWAP_ON_BE(sampler->cSampleLoops);
	if (length < sizeof(GO_WAVESAMPLERCHUNK) + sizeof(GO_WAVESAMPLERLOOP) * numberOfLoops)
		throw (wxString)_("<Invalid SMPL chunk in");

	GO_WAVESAMPLERLOOP* loops = (GO_WAVESAMPLERLOOP*)(ptr + sizeof(GO_WAVESAMPLERCHUNK));
	m_Loops.clear();
	for (unsigned k = 0; k < numberOfLoops; k++)
	{
		GO_WAVE_LOOP l;
		l.start_sample = wxUINT32_SWAP_ON_BE(loops[k].dwStart);
		l.end_sample = wxUINT32_SWAP_ON_BE(loops[k].dwEnd);
		m_Loops.push_back(l);
	}

	m_MidiNote = wxUINT32_SWAP_ON_BE(sampler->dwMIDIUnityNote);
	m_PitchFract = wxUINT32_SWAP_ON_BE(sampler->dwMIDIPitchFraction) / (double)UINT_MAX * 100.0;
}

void GOrgueWave::Open(const wxString& filename)
{
	/* Close any currently open wave data */
	Close();

	wxFile file;
	if (!file.Open(filename, wxFile::read))
	{
		wxString message;
		message.Printf(_("Failed to open file '%s'"), filename.c_str());
		throw message;
	}
	unsigned length = file.Length();
	
	// Allocate memory for wave and read it.
	m_Content = (char*)malloc(length);
	char* ptr = m_Content;

	unsigned offset = 0;
	unsigned start = 0;
	try
	{
		if (!ptr)
			throw GOrgueOutOfMemory();

		if (length < 12)
			throw (wxString)_("< Not a RIFF file");

		if (file.Read(ptr, length) != (ssize_t)length)
		{
			wxString message;
			message.Printf(_("Failed to read file '%s'\n"), filename.c_str());
			throw message;
		}
		file.Close();

		GOrgueWavPack pack(m_Content, length);
		if (pack.IsWavPack())
		{
			if (!pack.Unpack())
				throw (wxString)_("Failed to decode WavePack data");
			free(m_Content);
			m_Content = NULL;

			pack.GetSamples(m_SampleData, m_SampleDataSize);
			m_Content = m_SampleData;

			pack.GetWrapper(ptr, length);
			offset = 0;
			m_isPacked = true;
		}

		/* Read the header, get it's size and make sure that it makes sense. */
		GO_WAVECHUNKHEADER* riffHeader = (GO_WAVECHUNKHEADER*)(ptr + offset);
		unsigned long riffChunkSize = wxUINT32_SWAP_ON_BE(riffHeader->dwSize);

		/*
		if (!CompareFourCC(riffHeader->fccChunk, "RIFF") || (riffChunkSize > length - 8))
			throw (wxString)_("< Invalid RIFF file");
		*/

		/* Pribac compatibility */
		if (!CompareFourCC(riffHeader->fccChunk, "RIFF"))
		{
			throw (wxString)_("< Invalid RIFF file");
		}
		offset += sizeof(GO_WAVECHUNKHEADER);

		/* Make sure this is a RIFF/WAVE file */
		GO_FOURCC* riffIdent = (GO_FOURCC*)(ptr + offset);
		if (!CompareFourCC(*riffIdent, "WAVE"))
			throw (wxString)_("< Invalid RIFF/WAVE file");
		offset += sizeof(GO_FOURCC);

		if (m_isPacked)
		{
			if (riffChunkSize < pack.GetOrigDataLen())
				throw (wxString)_("Inconsitant WavPack file");
			riffChunkSize -= pack.GetOrigDataLen();
		}

		/* This is a bit more leaniant than the original code... it will
		 * truncate the usable size of the file if the size on disk is larger
		 * than the size of the RIFF chunk */
		if ((unsigned long)length > riffChunkSize + 8 + start)
			length = riffChunkSize + 8 + start;

		bool hasFormat = false;
		/* Find required chunks... */
		for (; offset + 8 <= length;)
		{

			/* Check for word alignment as per RIFF spec */
			assert((offset & 1) == 0);

			/* Read chunk header */
			GO_WAVECHUNKHEADER* header = (GO_WAVECHUNKHEADER*)(ptr + offset);
			unsigned long size = wxUINT32_SWAP_ON_BE(header->dwSize);
			offset += sizeof(GO_WAVECHUNKHEADER);

			if (CompareFourCC(header->fccChunk, "data"))
			{
				if (!hasFormat)
					throw (wxString)_("< Malformed wave file. Format chunk must preceed data chunk.");

				if (m_isPacked)
					size = 0;
				else
				{
					m_SampleData = ptr + offset;
					m_SampleDataSize = size;
				}
			}
			if (CompareFourCC(header->fccChunk, "fmt "))
			{
				hasFormat = true;
				LoadFormatChunk(ptr + offset, size);
			}
			if (CompareFourCC(header->fccChunk, "cue ")) /* This used to only work if !load m_pipe_percussive[i] */
				LoadCueChunk(ptr + offset, size);
			if (CompareFourCC(header->fccChunk, "smpl")) /* This used to only work if !load m_pipe_percussive[i] */
				LoadSamplerChunk(ptr + offset, size);

			/* Move to next chunk respecting word alignment */
			offset += size + (size & 1);

		}

		if (offset != length)
			throw (wxString)_("<Invalid WAV file");
		if (!m_SampleData || !m_SampleDataSize)
			throw (wxString)_("No samples found");

		// learning lesson: never ever trust the range values of outside sources to be correct!
		for (unsigned int i = 0; i < m_Loops.size(); i++)
		{
			if ((m_Loops[i].start_sample >= m_Loops[i].end_sample) ||
			    (m_Loops[i].start_sample >= GetLength()) ||
			    (m_Loops[i].end_sample >= GetLength()) ||
				(m_Loops[i].end_sample == 0))
				m_Loops.erase(m_Loops.begin() + i);
		}
	}
	catch (wxString msg)
	{
		wxLogError(_("unhandled exception: %s\n"), msg.c_str());

		/* Free any memory that was allocated by chunk loading procedures */
		Close();

		/* Rethrow the exception */
		throw;
	}
	catch (...)
	{
		/* Free any memory that was allocated by chunk loading procedures */
		Close();

		/* Rethrow the exception */
		throw;
	}
}

void GOrgueWave::Close()
{
	/* Free the wave data if it has been alloc'ed */
	if (m_Content != NULL)
	{
		free(m_Content);
		m_Content = NULL;
	}

	/* Set the wave to the invalid state.  */
	SetInvalid();
}

unsigned GOrgueWave::GetBitsPerSample() const
{
	return m_BytesPerSample * 8;
}

unsigned GOrgueWave::GetChannels() const
{
	return m_Channels;
}

bool GOrgueWave::HasReleaseMarker() const
{
	return m_hasRelease;
}

unsigned GOrgueWave::GetReleaseMarkerPosition() const
{
	/* release = dwSampleOffset from cue chunk. This is a byte offset into
	 * the data chunk. Compute this to a block start
	 */
	return m_CuePoint;
}

const GO_WAVE_LOOP& GOrgueWave::GetLongestLoop() const
{
	if (m_Loops.size() < 1)
		throw (wxString)_("wave does not contain loops");

	assert(m_Loops[0].end_sample > m_Loops[0].start_sample);
	unsigned lidx = 0;
	for (unsigned int i = 1; i < m_Loops.size(); i++)
	{
		assert(m_Loops[i].end_sample > m_Loops[i].start_sample);
		if ((m_Loops[i].end_sample - m_Loops[i].start_sample) >
			(m_Loops[lidx].end_sample - m_Loops[lidx].start_sample))
			lidx = i;
	}

	return m_Loops[lidx];
}

unsigned GOrgueWave::GetLength() const
{
	if (m_isPacked)
		return m_SampleDataSize / (4 * m_Channels);
	/* return number of samples in the stream */
	assert((m_SampleDataSize % (m_BytesPerSample * m_Channels)) == 0);
	return m_SampleDataSize / (m_BytesPerSample * m_Channels);
}

void GOrgueWave::ReadSamples
	(void* dest_buffer                        /** Pointer to received sample data */
	,GOrgueWave::SAMPLE_FORMAT read_format    /** Format of the above buffer */
	,unsigned sample_rate                     /** Sample rate to read data at */
	,int return_channels                      /** number of channels to return or if negative, specific channel as mono*/
	) const
{
	if (m_SampleRate != sample_rate)
		throw (wxString)_("bad format!");

	if (m_BytesPerSample < 1 || m_BytesPerSample > 4)
		throw (wxString)_("Unsupported format");

	unsigned select_channel = 0;
	if (return_channels < 0)
	{
		if ((unsigned)-return_channels > m_Channels)
			throw (wxString)_("Unsupported channel number");
		select_channel = -return_channels;
	}
	else if (m_Channels != (unsigned)return_channels && return_channels != 1)
		throw (wxString)_("Unsupported channel count");

	unsigned merge_count = 1;
	/* need reduce stereo to mono ? */
	if (m_Channels != (unsigned)return_channels && return_channels == 1)
		merge_count = m_Channels;
	if (select_channel != 0)
		merge_count = m_Channels;

	char* input  = m_SampleData;
	char* output = (char*)dest_buffer;

	unsigned len = m_Channels * GetLength() / merge_count;
	for(unsigned i = 0; i < len; i++)
	{
		int value = 0; /* Value will be stored with 24 fractional bits of precision */
		for (unsigned j = 0; j < merge_count; j++)
		{
			int val; /* Value will be stored with 24 fractional bits of precision */
			if (m_isPacked && m_BytesPerSample != 4)
			{
				val = (*((int32_t*)input));
				switch(m_BytesPerSample)
				{
				case 1:
					val <<= 16;
					break;
				case 2:
					val <<= 8;
					break;
				}
				input += 4;
			}
			else
			{
				switch(m_BytesPerSample)
				{
				case 1:
					val = (*((unsigned char*)input) - 0x80);
					val <<= 16;
					break;
				case 2:
					val = wxINT16_SWAP_ON_BE(*((wxInt16*)input));
					val <<= 8;
					break;
				case 3:
					val = GOInt24ToInt(*((GO_Int24*)input));
					break;
				case 4:
					val = (*(float*)input) * (float)(1 << 23);
					break;
				default:
					throw (wxString)_("bad format!");
				}
				input += m_BytesPerSample;
			}

			if (select_channel && select_channel != j + 1)
				continue;
			value += val;
		}
		if (select_channel == 0 && merge_count > 1)
			value = value / (int)merge_count;

		switch (read_format)
		{
		case SF_SIGNEDBYTE_8:
			*(wxInt8*)output = value >> 16;
			output += sizeof(wxInt8);
			break;
		case SF_SIGNEDSHORT_12:
			*(wxInt16*)output = value >> 12;
			output += sizeof(wxInt16);
			break;
		case SF_SIGNEDSHORT_16:
			*(wxInt16*)output = value >> 8;
			output += sizeof(wxInt16);
			break;
		case SF_SIGNEDINT24_20:
			*(Int24*)output = value >> 4;
			output += sizeof(Int24);
			break;
		case SF_SIGNEDINT24_24:
			*(Int24*)output = value;
			output += sizeof(Int24);
			break;
		case SF_IEEE_FLOAT:
			*(float*)output = value / (float)(1 << 23);
			output += sizeof(float);
			break;
		default:
			throw (wxString)_("bad return format!");
		}
	}
}

unsigned GOrgueWave::GetNbLoops() const
{
	return m_Loops.size();
}

const GO_WAVE_LOOP& GOrgueWave::GetLoop(unsigned idx) const
{
	assert(idx < m_Loops.size());
	return m_Loops[idx];
}

unsigned GOrgueWave::GetSampleRate() const
{
	return m_SampleRate;
}

unsigned GOrgueWave::GetMidiNote() const
{
	return m_MidiNote;
}

float GOrgueWave::GetPitchFract() const
{
	return m_PitchFract;
}
