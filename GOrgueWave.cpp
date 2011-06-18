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

#include "GOrgueWave.h"
#include "GOrgueWaveTypes.h"

void GOrgueWave::SetInvalid()
{
	data = NULL;
	dataSize = 0;
	channels = 0;
	bytesPerSample = 0;
	sampleRate = 0;
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
		throw (char*)"< Malformed wave file. Format chunk must preceed data chunk.";

	data = (char*)malloc(length);
	if (data == NULL)
		throw (char*)"< Failed to allocate memory for wave data";

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
		throw (char*)"< Invalid WAVE format chunk";

	GO_WAVEFORMATPCMEX* format = (GO_WAVEFORMATPCMEX*)ptr;
	unsigned formatCode = wxUINT16_SWAP_ON_BE(format->wf.wf.wFormatTag);
	if (formatCode != 1)
		throw (char*)"< Not PCM data in";

	/* get channels and ensure only mono or stereo */
	channels = wxUINT16_SWAP_ON_BE(format->wf.wf.nChannels);

	/* get sample rate and ensure only 44.1 kHz */
	sampleRate = wxUINT32_SWAP_ON_BE(format->wf.wf.nSamplesPerSec);

	unsigned bitsPerSample = wxUINT16_SWAP_ON_BE(format->wf.wBitsPerSample);
	if (bitsPerSample % 8)
		throw (char*)"< Bits per sample must be a multiple of 8 in this implementation";
	bytesPerSample = bitsPerSample / 8;

	hasFormat = true;

}

void GOrgueWave::LoadCueChunk(char* ptr, unsigned long length)
{

	if (length < sizeof(GO_WAVECUECHUNK))
		throw (char*)"< Invalid CUE chunk in";

	GO_WAVECUECHUNK* cue = (GO_WAVECUECHUNK*)ptr;
	unsigned nbPoints = wxUINT32_SWAP_ON_BE(cue->dwCuePoints);
	if (length < sizeof(GO_WAVECUECHUNK) + sizeof(GO_WAVECUEPOINT) * nbPoints)
		throw (char*)"< Invalid CUE chunk in";

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
		throw (char*)"< Invalid SMPL chunk in";

	GO_WAVESAMPLERCHUNK* sampler = (GO_WAVESAMPLERCHUNK*)ptr;
	unsigned numberOfLoops = wxUINT32_SWAP_ON_BE(sampler->cSampleLoops);
	if (length < sizeof(GO_WAVESAMPLERCHUNK) + sizeof(GO_WAVESAMPLERLOOP) * numberOfLoops)
		throw (char*)"<Invalid SMPL chunk in";

	GO_WAVESAMPLERLOOP* loops = (GO_WAVESAMPLERLOOP*)(ptr + sizeof(GO_WAVESAMPLERCHUNK));
	this->loops.clear();
	for (unsigned k = 0; k < numberOfLoops; k++)
	{
		GO_WAVE_LOOP l;
		l.start_sample = wxUINT32_SWAP_ON_BE(loops[k].dwStart);
		l.end_sample = wxUINT32_SWAP_ON_BE(loops[k].dwEnd);
		this->loops.push_back(l);
	}

}

void GOrgueWave::FindPeaks()
{

	/* Find the wave's peak samples */
	short* s_ptr = (short*)data;
	if (channels == 1)
	{
		unsigned peaktemp;
		for (unsigned k = 0; k < dataSize; k++)
		{
			/* FIXME: if sample < 0 sample++ ?? what
			 * this cannot be good... what is going on here?
			 *
			 * peaktemp = s_ptr[k] xor (s_ptr[k] >> 15)
			 * ... which basically means if s_ptr[k] < 0,
			 * flip the LSB of s_ptr[k]. */
			/*
			if (s_ptr[k] & 0x8000)
				s_ptr[k]++;
			peaktemp  = s_ptr[k];
			peaktemp ^= (peaktemp >> 15);
			*/
			peaktemp = abs(s_ptr[k]);
			peak = std::max(peaktemp, peak);
		}
	}
	else
	{
		unsigned peaktemp;
		for (unsigned k = 0; k < dataSize; k += 2)
		{
			/*
			if (s_ptr[k] & 0x8000)
				s_ptr[k]++;
			peaktemp  = s_ptr[k++];
			if (s_ptr[k] & 0x8000)
				s_ptr[k]++;
			peaktemp += s_ptr[k++];
			peaktemp ^= (peaktemp >> 15);
			*/
			peaktemp = abs(s_ptr[k] + s_ptr[k+1]);
			peak = std::max(peaktemp,peak);
		}
		peak = peak / 2;
	}

}

int readOneFile(int file, char* buffer, unsigned length)
{
  unsigned remain = length;
  for(;;)
	{
#ifdef _WIN32
	  remain -= _read(file, buffer, remain >= BUFSIZ ? BUFSIZ : remain);
#endif
#ifdef linux
	  remain -= read(file, buffer, remain >= BUFSIZ ? BUFSIZ : remain); // TODO better error handling
#endif
	  if (!remain)
		break;
	  buffer += BUFSIZ;
	}
#ifdef _WIN32
  _close(file);
#endif
#ifdef linux
  close(file);
#endif
  return length;
}

void GOrgueWave::Open(const wxString& filename)
{

	/* Close any currently open wave data */
	Close();

	wxString temp = filename;

	/* Open file, because of efficiency wxFile is not used
	 * FIXME: ^^^ the above comment was here when I started working
	 * on the code... I'm not sure what it means? Has the efficency
	 * of wxFile been tested? I think we should put it back in and
	 * make use of the cross-platform benefits. */
#ifdef linux
	temp.Replace(wxT("\\"), wxT("/"));
	int ffile = open(temp.mb_str(), O_RDONLY);
	struct stat ffile_info;
	fstat(ffile, &ffile_info);
	unsigned length = ffile_info.st_size;
#endif
#ifdef _WIN32
	int ffile = _open(temp, _O_BINARY | _O_RDONLY | _O_SEQUENTIAL);
	unsigned length = _filelength(ffile);
#endif

	/* FIXME: better errorhandling (linux) < this causes GrandOrgue
	 * to quit ungracefully! very bad. */
	if (ffile == -1)
	{
		char message[1024];
		const char* x = temp.mb_str();
		sprintf(message, "Failed to open file '%s'\n", x);
		throw message;
	}

	// Allocate memory for wave and read it.
	char* ptr = (char*)malloc(length);
	if (!ptr)
		throw (char*)_(" Out of memory loading");

	unsigned offset = 0;
	try
	{

		if (length < 12)
			throw (char*)"< Not a RIFF file";

		length = readOneFile(ffile, ptr, length);

		/* Read the header, get it's size and make sure that it makes sense. */
		GO_WAVECHUNKHEADER* riffHeader = (GO_WAVECHUNKHEADER*)(ptr + offset);
		unsigned long riffChunkSize = wxUINT32_SWAP_ON_BE(riffHeader->dwSize);

		/*
		if (!CompareFourCC(riffHeader->fccChunk, "RIFF") || (riffChunkSize > length - 8))
			throw (char*)"< Invalid RIFF file";
		*/

		/* Pribac compatibility */
		if (!CompareFourCC(riffHeader->fccChunk, "RIFF"))
			throw (char*)"< Invalid RIFF file";
		offset += sizeof(GO_WAVECHUNKHEADER);

		/* Make sure this is a RIFF/WAVE file */
		GO_FOURCC* riffIdent = (GO_FOURCC*)(ptr + offset);
		if (!CompareFourCC(*riffIdent, "WAVE"))
			throw (char*)"< Invalid RIFF/WAVE file";
		offset += sizeof(GO_FOURCC);

		/* This is a bit more leaniant than the original code... it will
		 * truncate the usable size of the file if the size on disk is larger
		 * than the size of the RIFF chunk */
		if (length > riffChunkSize + 8)
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
			throw (char*)"<Invalid WAV file";

		// learning lesson: never ever trust the range values of outside sources to be correct!
		for (unsigned int i = 0; i < loops.size(); i++)
		{
			if ((loops[i].start_sample >= loops[i].end_sample) ||
				(loops[i].start_sample >= dataSize) ||
				(loops[i].end_sample >= dataSize) ||
				(loops[i].end_sample == 0) ||
				(release == 0))
				loops.erase(loops.begin() + i);
		}

		/* Free the memory used to hold the file */
		free(ptr);

	}
	catch (char* msg)
	{

		fprintf(stderr, "unhandled exception: %s\n", msg);

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

unsigned GOrgueWave::GetChannels()
{

	return channels;

}

bool GOrgueWave::HasReleaseMarker()
{

	return hasRelease;

}

unsigned GOrgueWave::GetReleaseMarkerPosition()
{

	/* release = dwSampleOffset from cue chunk. This is a byte offset into
	 * the data chunk. Compute this to a block start
	 */
	return release;

}

const unsigned GOrgueWave::GetLongestLoop()
{

	if (loops.size() < 1)
		throw (char*)"wave does not contain loops";

	assert(loops[0].end_sample > loops[0].start_sample);

	unsigned lidx = 0;
	for (unsigned int i = 1; i < loops.size(); i++)
	{

		assert(loops[i].end_sample > loops[i].start_sample);
		if ((loops[i].end_sample - loops[i].start_sample) >
			(loops[lidx].end_sample - loops[lidx].start_sample))
			lidx = i;

	}

	return lidx;

}

unsigned GOrgueWave::GetLoopStartPosition()
{

	return loops[GetLongestLoop()].start_sample;

}

unsigned GOrgueWave::GetLoopEndPosition()
{

	return loops[GetLongestLoop()].end_sample;

}

unsigned GOrgueWave::GetPeak()
{

	return peak;

}

unsigned GOrgueWave::GetLength()
{

	/* return number of samples in the stream */
	assert((dataSize % (bytesPerSample * channels)) == 0);
	return dataSize / (bytesPerSample * channels);

}

void GOrgueWave::ReadSamples(void* destBuffer, GOrgueWave::SAMPLE_FORMAT readFormat, unsigned sampleRate)
{

	if (sampleRate != 44100)
		throw (char*)"bad format!";

	switch (readFormat)
	{
		case SF_SIGNEDSHORT:
			if (bytesPerSample != 2)
				throw (char*)"bad format!";
			memcpy(destBuffer, data, bytesPerSample * channels * GetLength());
			break;

		default:
			throw (char*)"bad format!";
	}

}

unsigned GOrgueWave::GetNbLoops()
{

	return loops.size();

}

const GO_WAVE_LOOP& GOrgueWave::GetLoop(unsigned idx)
{

	assert(idx < loops.size());
	return loops[idx];

}

unsigned GOrgueWave::GetSampleRate()
{

	return sampleRate;

}
