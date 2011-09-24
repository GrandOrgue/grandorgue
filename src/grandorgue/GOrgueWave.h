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
		SF_SIGNEDSHORT
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
	void ReadSamples(void* destBuffer, GOrgueWave::SAMPLE_FORMAT readFormat, unsigned sampleRate) const;


	unsigned GetSampleRate() const;



};

#endif /* GORGUEWAVE_H */
