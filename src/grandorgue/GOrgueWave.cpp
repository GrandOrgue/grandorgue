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

#include <wx/wx.h>
#include <wx/file.h>
#include "GOrgueMemoryPool.h"
#include "GOrgueInt24.h"
#include "GOrgueWave.h"
#include "GOrgueWaveTypes.h"

void GOrgueWave::SetInvalid()
{
	data = NULL;
	dataSize = 0;
	channels = 0;
	bytesPerSample = 0;
	sampleRate = 0;
	m_MidiNote = 0;
	m_PitchFract = 0;
	hasFormat = false;
	release = 0;
	hasRelease = false;
	loops.clear();
}

GOrgueWave::GOrgueWave() :
	loops()
{
	/* Start up the waveform in an invalid state */
	SetInvalid();
}

GOrgueWave::~GOrgueWave()
{
	/* Close and free any currently open wave data */
	Close();
}

void GOrgueWave::LoadDataChunk(char* ptr, unsigned long length)
{
	if (!hasFormat)
		throw (wxString)_("< Malformed wave file. Format chunk must preceed data chunk.");

	data = (char*)malloc(length);
	if (data == NULL)
		throw GOrgueOutOfMemory();

	dataSize = length;
	memcpy(data, ptr, dataSize);
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

	/* get channels and ensure only mono or stereo */
	channels = wxUINT16_SWAP_ON_BE(format->wf.wf.nChannels);

	/* get sample rate and ensure only 44.1 kHz */
	sampleRate = wxUINT32_SWAP_ON_BE(format->wf.wf.nSamplesPerSec);

	unsigned bitsPerSample = wxUINT16_SWAP_ON_BE(format->wf.wBitsPerSample);
	if (bitsPerSample % 8)
		throw (wxString)_("< Bits per sample must be a multiple of 8 in this implementation");
	bytesPerSample = bitsPerSample / 8;

	if (formatCode == 3 && bytesPerSample != 4)
		throw (wxString)_("< Only 32bit IEEE float samples supported");
	else if (formatCode == 1 && bytesPerSample > 3)
		throw (wxString)_("< Unsupport PCM bit size");

	hasFormat = true;
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
	hasRelease = (nbPoints > 0);
	for (unsigned k = 0; k < nbPoints; k++)
	{
		assert(channels != 0);
		unsigned position = wxUINT32_SWAP_ON_BE(points[k].dwSampleOffset);
		if (position > release)
			release = position;
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
	this->loops.clear();
	for (unsigned k = 0; k < numberOfLoops; k++)
	{
		GO_WAVE_LOOP l;
		l.start_sample = wxUINT32_SWAP_ON_BE(loops[k].dwStart);
		l.end_sample = wxUINT32_SWAP_ON_BE(loops[k].dwEnd);
		this->loops.push_back(l);
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
		message.Printf(_("Failed to open file '%s'\n"), filename.c_str());
		throw message;
	}
	wxFileOffset length = file.Length();
	
	// Allocate memory for wave and read it.
	char* ptr = (char*)malloc(length);
	if (!ptr)
		throw (wxString)_(" Out of memory loading");

	unsigned offset = 0;
	try
	{

		if (length < 12)
			throw (wxString)_("< Not a RIFF file");

		if (file.Read(ptr, length) != length)
		{
			wxString message;
			message.Printf(_("Failed to read file '%s'\n"), filename.c_str());
			throw message;
		}
		file.Close();

		/* Read the header, get it's size and make sure that it makes sense. */
		GO_WAVECHUNKHEADER* riffHeader = (GO_WAVECHUNKHEADER*)(ptr + offset);
		unsigned long riffChunkSize = wxUINT32_SWAP_ON_BE(riffHeader->dwSize);

		/*
		if (!CompareFourCC(riffHeader->fccChunk, "RIFF") || (riffChunkSize > length - 8))
			throw (wxString)_("< Invalid RIFF file");
		*/

		/* Pribac compatibility */
		if (!CompareFourCC(riffHeader->fccChunk, "RIFF"))
			throw (wxString)_("< Invalid RIFF file");
		offset += sizeof(GO_WAVECHUNKHEADER);

		/* Make sure this is a RIFF/WAVE file */
		GO_FOURCC* riffIdent = (GO_FOURCC*)(ptr + offset);
		if (!CompareFourCC(*riffIdent, "WAVE"))
			throw (wxString)_("< Invalid RIFF/WAVE file");
		offset += sizeof(GO_FOURCC);

		/* This is a bit more leaniant than the original code... it will
		 * truncate the usable size of the file if the size on disk is larger
		 * than the size of the RIFF chunk */
		if ((unsigned long)length > riffChunkSize + 8)
			length = riffChunkSize + 8;

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
				LoadDataChunk(ptr + offset, size);
			if (CompareFourCC(header->fccChunk, "fmt "))
				LoadFormatChunk(ptr + offset, size);
			if (CompareFourCC(header->fccChunk, "cue ")) /* This used to only work if !load m_pipe_percussive[i] */
				LoadCueChunk(ptr + offset, size);
			if (CompareFourCC(header->fccChunk, "smpl")) /* This used to only work if !load m_pipe_percussive[i] */
				LoadSamplerChunk(ptr + offset, size);

			/* Move to next chunk respecting word alignment */
			offset += size + (size & 1);

		}

		if (offset != length)
			throw (wxString)_("<Invalid WAV file");

		// learning lesson: never ever trust the range values of outside sources to be correct!
		for (unsigned int i = 0; i < loops.size(); i++)
		{
			if ((loops[i].start_sample >= loops[i].end_sample) ||
			    (loops[i].start_sample >= GetLength()) ||
			    (loops[i].end_sample >= GetLength()) ||
				(loops[i].end_sample == 0) ||
				(release == 0))
				loops.erase(loops.begin() + i);
		}

		/* Free the memory used to hold the file */
		free(ptr);

	}
	catch (wxString msg)
	{
		wxLogError(_("unhandled exception: %s\n"), msg.c_str());

		/* Free the memory used to hold the file */
		free(ptr);

		/* Free any memory that was allocated by chunk loading procedures */
		Close();

		/* Rethrow the exception */
		throw;
	}
	catch (...)
	{
		/* Free the memory used to hold the file */
		free(ptr);

		/* Free any memory that was allocated by chunk loading procedures */
		Close();

		/* Rethrow the exception */
		throw;
	}
}

void GOrgueWave::Close()
{
	/* Free the wave data if it has been alloc'ed */
	if (data != NULL)
	{
		free(data);
		data = NULL;
	}

	/* Set the wave to the invalid state. This ensures that data will be set
	 * to NULL and dataSize will be set to zero... etc */
	SetInvalid();
}

unsigned GOrgueWave::GetChannels() const
{
	return channels;
}

bool GOrgueWave::HasReleaseMarker() const
{
	return hasRelease;
}

unsigned GOrgueWave::GetReleaseMarkerPosition() const
{
	/* release = dwSampleOffset from cue chunk. This is a byte offset into
	 * the data chunk. Compute this to a block start
	 */
	return release;
}

const GO_WAVE_LOOP& GOrgueWave::GetLongestLoop() const
{
	if (loops.size() < 1)
		throw (wxString)_("wave does not contain loops");

	assert(loops[0].end_sample > loops[0].start_sample);
	unsigned lidx = 0;
	for (unsigned int i = 1; i < loops.size(); i++)
	{
		assert(loops[i].end_sample > loops[i].start_sample);
		if ((loops[i].end_sample - loops[i].start_sample) >
			(loops[lidx].end_sample - loops[lidx].start_sample))
			lidx = i;
	}

	return loops[lidx];
}

unsigned GOrgueWave::GetLength() const
{
	/* return number of samples in the stream */
	assert((dataSize % (bytesPerSample * channels)) == 0);
	return dataSize / (bytesPerSample * channels);
}

void GOrgueWave::ReadSamples
	(void* dest_buffer                        /** Pointer to received sample data */
	,GOrgueWave::SAMPLE_FORMAT read_format    /** Format of the above buffer */
	,unsigned sample_rate                     /** Sample rate to read data at */
	,unsigned return_channels                 /** number of channels to return */
	) const
{
	if (sampleRate != sample_rate)
		throw (wxString)_("bad format!");

	if (bytesPerSample < 1 || bytesPerSample > 4)
		throw (wxString)_("Unsupported format");

	if (channels != return_channels && return_channels != 1)
		throw (wxString)_("Unsupported channel count");

	unsigned merge_count = 1;
	/* need reduce stereo to mono ? */
	if (channels != return_channels && return_channels == 1)
		merge_count = channels;

	char* input  = (char*)data;
	char* output = (char*)dest_buffer;

	for(unsigned i = 0; i < channels * GetLength() / merge_count; i++)
	{
		int value = 0; /* Value will be stored with 24 fractional bits of precision */
		for (unsigned j = 0; j < merge_count; j++)
		{
			int val; /* Value will be stored with 24 fractional bits of precision */
			switch(bytesPerSample)
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
			input += bytesPerSample;

			value += val;
		}
		if (merge_count > 1)
			value = value / merge_count;

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
	return loops.size();
}

const GO_WAVE_LOOP& GOrgueWave::GetLoop(unsigned idx) const
{
	assert(idx < loops.size());
	return loops[idx];
}

unsigned GOrgueWave::GetSampleRate() const
{
	return sampleRate;
}

unsigned GOrgueWave::GetMidiNote() const
{
	return m_MidiNote;
}

float GOrgueWave::GetPitchFract() const
{
	return m_PitchFract;
}
