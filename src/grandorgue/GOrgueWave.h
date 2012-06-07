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

#ifndef GORGUEWAVE_H
#define GORGUEWAVE_H

#include <wx/wx.h>
#include <vector>

typedef struct
{
	unsigned start_sample;
	unsigned end_sample;
} GO_WAVE_LOOP;

class GOrgueWave
{

private:

	char* data;
	unsigned dataSize;

	unsigned channels;
	unsigned release;
	unsigned bytesPerSample;
	unsigned sampleRate;
	unsigned m_MidiNote;
	float m_PitchFract;
	bool hasFormat;

	bool hasRelease;

	unsigned peak;

	void SetInvalid();
	void LoadDataChunk(char* ptr, unsigned long length);
	void LoadFormatChunk(char* ptr, unsigned long length);
	void LoadCueChunk(char* ptr, unsigned long length);
	void LoadSamplerChunk(char* ptr, unsigned long length);

	std::vector<GO_WAVE_LOOP> loops;

public:

	typedef enum
	{
		SF_SIGNEDBYTE_8 = 8,
		SF_SIGNEDSHORT_12 = 12,
		SF_SIGNEDSHORT_16 = 16,
		SF_SIGNEDINT24_20 = 20,
		SF_SIGNEDINT24_24 = 24,
		SF_IEEE_FLOAT
	} SAMPLE_FORMAT;

	GOrgueWave();
	~GOrgueWave();

	void Open(const wxString& filename);
	void Close();

	/* GetChannels()
	 * Returns the number of channels in the wave stream.
	 */
	unsigned GetChannels() const;

	/* GetReleaseMarkerPosition()
	 * Returns the block where the release marker has been placed at.
	 */
	bool HasReleaseMarker() const;
	unsigned GetReleaseMarkerPosition() const;

	unsigned GetNbLoops() const;
	const GO_WAVE_LOOP& GetLoop(unsigned idx) const;
	const GO_WAVE_LOOP& GetLongestLoop() const;

	/* GetLength()
	 * Returns the number of blocks in the wave file (there are *channel*
	 * samples in a block so for a mono wave, this will simply be the number
	 * of samples)
	 */
	unsigned GetLength() const;

	/* ReadSamples()
	 * Reads all of the samples in the wave file into destBuffer at the
	 * specified read format and sample rate.
	 */
	void ReadSamples(void* dest_buffer, GOrgueWave::SAMPLE_FORMAT read_format, unsigned sample_rate, unsigned channels) const;


	unsigned GetSampleRate() const;
	unsigned GetMidiNote() const;
	float GetPitchFract() const;


};

#endif /* GORGUEWAVE_H */
