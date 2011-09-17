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

#include "GOSoundProviderWave.h"
#include "GOrgueWave.h"
#include <wx/filename.h>

#define FREE_AND_NULL(x) do { if (x) { free(x); x = NULL; } } while (0)
#define DELETE_AND_NULL(x) do { if (x) { delete x; x = NULL; } } while (0)

void GOSoundProviderWave::LoadFromFile
	(wxString filename
	,int fixed_amplitude
	)
{

	wxLogDebug(_("Loading file %s"), filename.c_str());

	/* Translate directory seperator from ODF(\) to native format */
	wxString temp = filename;
	temp.Replace(wxT("\\"), wxString(wxFileName::GetPathSeparator()));

	GOrgueWave wave;
	wave.Open(temp);

	/* allocate data to work with */
	unsigned totalDataSize = wave.GetLength() * sizeof(wxInt16) * wave.GetChannels();
	wxInt16* data = (wxInt16*)malloc(totalDataSize);
	if (data == NULL)
		throw (wxString)_("< out of memory allocating wave");

	memset(&m_loop, 0, sizeof(m_loop));
	memset(&m_attack, 0, sizeof(m_attack));
	memset(&m_release, 0, sizeof(m_release));

	try
	{

		wave.ReadSamples(data, GOrgueWave::SF_SIGNEDSHORT, 44100);
		m_SampleRate = 44100;

		m_Channels = wave.GetChannels();
		if (m_Channels < 1 || m_Channels > 2)
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
			attackSamples = wave.GetLoopStartPosition();

			/* Get the loop parameters */
			unsigned loopStart = wave.GetLoopStartPosition();
			unsigned loopSamples = wave.GetLoopEndPosition() - loopStart + 1;
			unsigned loopSamplesInMem = loopSamples + BLOCKS_PER_FRAME;
			assert(loopStart > 0);
			assert(wave.GetLoopEndPosition() > loopStart);

			/* Allocate memory for the loop, copy the loop into it and then
			 * copy some slack samples from the beginning of the loop onto
			 * the end to ensure correct operation of the sampler.
			 */
			m_loop.size = loopSamples * sizeof(wxInt16) * m_Channels;
			m_loop.alloc_size = loopSamplesInMem * sizeof(wxInt16) * m_Channels;
			m_loop.data = (unsigned char*)malloc(m_loop.alloc_size);
			if (m_loop.data == NULL)
				throw (wxString)_("< out of memory allocating loop");
			memcpy(m_loop.data,
				&data[loopStart * m_Channels],
				m_loop.size);
			memcpy(&m_loop.data[m_loop.size],
				&data[loopStart * m_Channels],
				loopSamplesInMem * sizeof(wxInt16) * m_Channels - m_loop.size);

			/* Get the release parameters from the wave file. */
			unsigned releaseOffset = wave.GetReleaseMarkerPosition();
			unsigned releaseSamples = wave.GetLength() - releaseOffset;
			unsigned releaseSamplesInMem = releaseSamples + BLOCKS_PER_FRAME;

			/* Allocate memory for the release, copy the release into it and
			 * pad the slack samples with zeroes to ensure correct operation
			 * of the sampler.
			 */
			m_release.size = releaseSamples * sizeof(wxInt16) * m_Channels;
			m_release.alloc_size = releaseSamplesInMem * sizeof(wxInt16) * m_Channels;
			m_release.data = (unsigned char*)malloc(m_release.alloc_size);
			if (m_release.data == NULL)
				throw (wxString)_("< out of memory allocating release");
			memcpy(m_release.data,
				&data[(wave.GetLength() - releaseSamples) * m_Channels],
				m_release.size);
			memset(&m_release.data[m_release.size],
				0,
				releaseSamplesInMem * sizeof(wxInt16) * m_Channels - m_release.size);

		}

		/* Allocate memory for the attack. */
		assert(attackSamples != 0);
		unsigned attackSamplesInMem = attackSamples + BLOCKS_PER_FRAME;
		m_attack.size = attackSamples * sizeof(wxInt16) * m_Channels;
		m_attack.alloc_size = attackSamplesInMem * sizeof(wxInt16) * m_Channels;
		assert((unsigned)m_attack.size <= totalDataSize); /* can be equal for percussive samples */
		m_attack.data = (unsigned char*)malloc(m_attack.alloc_size);
		if (m_attack.data == NULL)
			throw (wxString)_("< out of memory allocating attack");

		if (attackSamplesInMem <= wave.GetLength())
		{
			memcpy(m_attack.data,
				&data[0],
				attackSamplesInMem * sizeof(wxInt16) * m_Channels);
		}
		else
		{
			memset(
				m_attack.data,
				0,
				(attackSamplesInMem - wave.GetLength()) * sizeof(wxInt16) * m_Channels);
			memcpy(&m_attack.data[(attackSamplesInMem - wave.GetLength()) * sizeof(wxInt16) * m_Channels],
				&data[0],
				totalDataSize);
		}

		/* data is no longer needed */
		FREE_AND_NULL(data);

		/* Amplitude is the combination of global amplitude volume and the stop
		 * volume. 10000 would correspond to sample playback at normal volume.
		 */
		int amp = fixed_amplitude;
		ra_shift = 7;
		while (amp > 10000)
		{
			ra_shift--;
			amp >>= 1;
		}

		/* During normal playback, this value will be multipled with each
		 * sample (in the form of the "fade" parameter) fademax is also set
		 * to this value to specify a maximum multiplier that can be applied
		 * to keep the sample within the required headroom. */
		ra_amp = (amp << 15) / -10000;

		if (m_Channels == 1)
		{
			m_attack.type = AC_UNCOMPRESSED_MONO;
			m_loop.type = AC_UNCOMPRESSED_MONO;
			m_release.type = AC_UNCOMPRESSED_MONO;
		}
		else
		{
			m_attack.type = AC_UNCOMPRESSED_STEREO;
			m_loop.type = AC_UNCOMPRESSED_STEREO;
			m_release.type = AC_UNCOMPRESSED_STEREO;
		}

		m_attack.stage = GSS_ATTACK;
		m_loop.stage = GSS_LOOP;
		m_release.stage = GSS_RELEASE;

		if (wave.HasReleaseMarker())
			ComputeReleaseAlignmentInfo();

	}
	catch (wxString error)
	{
		wxLogError(_("caught exception: %s\n"), error.c_str());
		FREE_AND_NULL(data);
		FREE_AND_NULL(m_attack.data);
		FREE_AND_NULL(m_loop.data);
		FREE_AND_NULL(m_release.data);
		throw;
	}
	catch (...)
	{
		FREE_AND_NULL(data);
		FREE_AND_NULL(m_attack.data);
		FREE_AND_NULL(m_loop.data);
		FREE_AND_NULL(m_release.data);
		throw;
	}

}

/* FIXME: This function must be broken - not going to even start describing the problem.
 * TODO: "count" refers to the number of BYTES in the stream... not samples.
 *
 */

//void GrandOrgueFile::CompressWAV(char*& compress, short* fv, short* ptr, int count, int channels, int stage)
//{
//	int f[4] = {0, 0, 0, 0}, v[4] = {0, 0, 0, 0}, a[4] = {0, 0, 0, 0};
//	int size = 0, index;
//	char* origlength;
//	int i, j, count2;
//
//	/* not recommended, but if they want to, reduce stereo to mono
//	 * ^^ the above was an original comment.
//	 *
//	 * the stream is converted to mono and count still refers to the number of
//	 * bytes in the stream.
//	 */
//	if (channels == 2 && !(g_sound->IsStereo()))
//	{
//		for (int i = 0, j = 0; i < count; i += 2)
//			ptr[j++] = ((int)ptr[i] + (int)ptr[i + 1]) >> 1;
//		count >>= 1;
//		channels = 1;
//	}
//
//	/* maxsearch is the number of BYTES to look for phase alignment
//	 */
//	int maxsearch = 2206 * channels;	// ~20Hz maximum search width for phase alignment table
//	if (maxsearch > count)
//		maxsearch = count - (count & channels);
//
//	origlength = compress;
//	m_compress_p->types[stage] = (channels - 1) << 1;
//
//	/*
//	 * Initialise f's and v's in preparation for compression/phase alignment
//	 */
//	for (i = 0; i < 4; i++)
//	{
//		// stereo : j = i
//		// mono   : j = i / 2
//		j = i >> (2 - channels);
//		if (count > channels * 2)
//            v[i] = fv[i + 12] = ptr[j + channels * 2] - ptr[j];
//		else
//			v[i] = fv[i + 12] = 0;
//		f[i] = fv[i] = ptr[j] - v[i];
//	}
//
//	bool flags[PHASE_ALIGN_RES_VA];
//	if (stage == 2)
//	{
//		for (j = 0; j < PHASE_ALIGN_RES_VA; j++)
//		{
//			flags[j] = false;
//			m_compress_p->ra_offset[j] = 0;
//			for (i = 0; i < 4; i++)
//			{
//				m_compress_p->ra_f[j][i] = f[i];
//				m_compress_p->ra_v[j][i] = v[i];
//			}
//		}
//
//		int prev_index = m_compress_p->ra_getindex( f, v), prev_i = 0;
//		for (i = 0, j = 0; i < maxsearch; )
//		{
//			v[j] = ptr[i] - f[j];
//			f[j] = ptr[i];
//			j++;
//			if (channels == 1)
//			{
//				v[j] = v[j-1];
//				f[j] = f[j-1];
//				j++;
//			}
//			j &= 3;
//			i++;
//			if (!j)
//			{
//				index = m_compress_p->ra_getindex( f, v);
//				if(index>=PHASE_ALIGN_RES_VA)
//				  std::cout << index << ":index out of bound\n";
//				if (index != prev_index)
//				{
//					if (!m_compress_p->ra_offset[prev_index])
//						m_compress_p->ra_offset[prev_index] = ((prev_i + i) >> (channels + 1)) << (channels + 1);
//					prev_i = i;
//					prev_index = index;
//				}
//			}
//		}
//
//		for (i = 0; i < 4; i++)
//		{
//			f[i] = fv[i];
//			v[i] = fv[i + 12];
//		}
//	}
//
//	if (!m_b_squash)
//		goto CantCompress;
//
//	count2 = count + (count & channels);
//	if (count & channels)
//		for (j = 0; j < channels; j++)
//			*(ptr + count + j) = 2 * (int)*(ptr + count + j - 2 * channels) - (int)*(ptr + count + j - 4 * channels);
//
//	if (channels == 2)
//	{
//		for (i = 0; i < count2; )
//		{
//			int value;
//			value = ptr[i] - f[0];
//			a[0] = value - v[0];
//			v[0] = value;
//			f[0] = ptr[i++];
//			value = ptr[i] - f[1];
//			a[1] = value - v[1];
//			v[1] = value;
//			f[1] = ptr[i++];
//			value = ptr[i] - f[2];
//			a[2] = value - v[2];
//			v[2] = value;
//			f[2] = ptr[i++];
//			value = ptr[i] - f[3];
//			a[3] = value - v[3];
//			v[3] = value;
//			f[3] = ptr[i++];
//
//            if (((((v[0] + 32768) | (v[1] + 32768) | (v[2] + 32768) | (v[3] + 32768)) >> 16) | (((a[0] + 16384) | (a[1] + 16384) | (a[2] + 16384) | (a[3] + 16384)) >> 15)) && i < count)
//                goto CantCompress;
//            if ( (((a[0] + 128) | (a[1] + 128) | (a[2] + 128)) >> 8) | ((a[3] + 64) >> 7))
//            {
//                *(int*)(compress     ) = ((a[3] & 0xFF00) << 17) | ((a[2] & 0xFF00) <<  9) | ((a[1] & 0xFF00) <<  1) | ((a[0] & 0xFF00) >>  7);
//                *(int*)(compress += 4) = ((a[3] & 0x00FF) << 24) | ((a[2] & 0x00FF) << 16) | ((a[1] & 0x00FF) <<  8) | ((a[0] & 0x00FF)      );
//                size += i <= count ? 8 : 4;
//            }
//            else
//            {
//                *(int*)(compress     ) = ((a[3] & 0x00FF) << 25) | ((a[2] & 0x00FF) << 17) | ((a[1] & 0x00FF) <<  9) | ((a[0] & 0x00FF) <<  1) | 1;
//                size += i <= count ? 4 : 2;
//            }
//            compress += 4;
//
//            if (i < maxsearch && stage == 2)
//            {
//                index = m_compress_p->ra_getindex( f, v);
//                if (!flags[index] && m_compress_p->ra_offset[index] == i << 1)
//                {
//                    flags[index] = true;
//                    m_compress_p->ra_offset[index] = size;
//                    for (j = 0; j < 4; j++)
//                    {
//                        m_compress_p->ra_f[index][j] = f[j];
//                        m_compress_p->ra_v[index][j] = v[j];
//                    }
//                }
//            }
//		}
//	}
//	else
//	{
//		for (i = 0; i < count2; )
//		{
//			int value;
//			value = ptr[i] - f[0];
//			a[0] = value - v[0];
//			v[0] = value;
//			f[0] = ptr[i++];
//			value = ptr[i] - f[2];
//			a[2] = value - v[2];
//			v[2] = value;
//			f[2] = ptr[i++];
//
//            if (((((v[0] + 32768) | (v[2] + 32768)) >> 16) | (((a[0] + 16384) | (a[2] + 16384)) >> 15)) && i < count)
//                goto CantCompress;
//
//            if (((a[0] + 128) >> 8) | ((a[2] + 64) >> 7))
//            {
//                *(short*)(compress     ) = ((a[2] & 0xFF00) <<  1) | ((a[0] & 0xFF00) >>  7);
//                *(short*)(compress += 2) = ((a[2] & 0x00FF) <<  8) | ((a[0] & 0x00FF)      );
//                size += i <= count ? 4 : 2;
//            }
//            else
//            {
//                *(short*)(compress     ) = ((a[2] & 0x00FF) <<  9) | ((a[0] & 0x00FF) <<  1) | 1;
//                size += i <= count ? 2 : 1;
//            }
//            compress += 2;
//
//            if (i < maxsearch && stage == 2)
//            {
//                f[1] = f[0];
//                f[3] = f[2];
//                v[1] = v[0];
//                v[3] = v[2];
//                index = m_compress_p->ra_getindex( f, v);
//                if (!flags[index] && m_compress_p->ra_offset[index] == i << 1)
//                {
//                    flags[index] = true;
//                    m_compress_p->ra_offset[index] = size;
//                    for (j = 0; j < 4; j++)
//                    {
//                        m_compress_p->ra_f[index][j] = f[j];
//                        m_compress_p->ra_v[index][j] = v[j];
//                    }
//                }
//            }
//		}
//	}
//	compress += channels << 1;
//
//	goto Done;
//
//CantCompress:
//	m_compress_p->types[stage] |= 1;
//
//	compress = origlength;
//	memcpy(compress, ptr, sizeof(short) * count);
//	memcpy(compress+(sizeof(short) * count),ptr, sizeof(short) * (count & channels));
//	compress=compress+(sizeof(short)*((count&channels) + count));
//
//Done:
//	m_compress_p->ptr[stage]    = (wxByte*)((wxByte*)(origlength + size) - (wxByte*)m_compress_p);
//	m_compress_p->offset[stage] = -size;
//	if (stage == 2)
//		for (j = 0; j < PHASE_ALIGN_RES_VA; j++)
//			m_compress_p->ra_offset[j] -= size;
//}

