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

#ifndef GOSOUNDAUDIOSECTION_H_
#define GOSOUNDAUDIOSECTION_H_

#include "GOSoundDefs.h"
#include "GOSoundResample.h"
#include "GOrgueInt24.h"
#include "GOrgueWave.h"
#include "GOSoundCompress.h"
#include <assert.h>
#include <wx/wx.h>

class GOrgueReleaseAlignTable;
class GOrgueCache;
class GOrgueCacheWriter;
class GOrgueMemoryPool;
class GOAudioSection;

struct audio_section_stream_s;

typedef void (*DecodeBlockFunction)(audio_section_stream_s *stream, float *output);

/* Reduces the number of samples which can be read from the main data stream
 * of the audio section (sample_count). This is necessary to ensure that
 * during a block read call, samples will not be read beyond the end of a
 * particular loop - but will be wrapped back to the beginning of the loop. */
#define SECTION_STOP_SAMPLES_BEFORE (2 * BLOCKS_PER_FRAME)

/* Number of additional samples to save after the end of a loop to guarantee
 * correct wrapping of samples. This parameter must be greater than
 * BLOCKS_PER_FRAME to accomodate for stretching of blocks (when they are
 * re-pitched). */
#define SECTION_SAVE_SAMPLES_AFTER (2 * BLOCKS_PER_FRAME)

typedef struct audio_start_data_segment_s
{

	/* Byte offset into data buffer where this section begins. */
	unsigned    data_offset; /* TODO: Will need to be an array... */

	/* Sample offset into entire audio section where data begins. */
	unsigned    start_offset;

	/* History of samples required to start in this blob
	 * TODO: this is where we'll need compression info etc... */
	int         history[BLOCK_HISTORY][MAX_OUTPUT_CHANNELS];
	int         last_value[MAX_OUTPUT_CHANNELS];
	int         diff_value[MAX_OUTPUT_CHANNELS];
	int         curr_value[MAX_OUTPUT_CHANNELS];
	int         next_value[MAX_OUTPUT_CHANNELS];


} audio_start_data_segment;

typedef struct audio_end_data_segment_s
{

	/* Sample offset where the loop ends and needs to jump into the next
	 * start segment. */
	unsigned      end_offset;

	/* Sample offset where the uncompressed end data blob begins (must be less
	 * than end_offset). */
	unsigned      transition_offset;

	/* Uncompressed ending data blob. This data must start before sample_offset*/
	unsigned char *end_data;

	/* Index of the next section segment to be played (-1 indicates the
	 * end of the blob. */
	int            next_start_segment_index;

} audio_end_data_segment;

typedef struct audio_section_stream_s
{
	const GOAudioSection           *audio_section;
	const struct resampler_coefs_s *resample_coefs;

	/* Method used to decode stream */
	DecodeBlockFunction          decode_call;

	/* Cached method used to decode the data in end_ptr. This is just the
	 * uncompressed version of decode_call */
	DecodeBlockFunction          end_decode_call;

	/* Used for both compressed and uncompressed decoding */
	const unsigned char         *ptr;

	/* Derived from the start and end audio segments which were used to setup
	 * this stream. */
	const unsigned char         *end_ptr;
	unsigned                     transition_position;
	unsigned                     section_length;
	int                          next_start_segment_index;

	unsigned                     filter_index;
	unsigned                     position_index;
	unsigned                     position_fraction;
	unsigned                     increment_fraction;

	/* for decoding compressed format */
	unsigned                     last_position;
	int                          last_value[MAX_OUTPUT_CHANNELS];
	int                          diff_value[MAX_OUTPUT_CHANNELS];
	int                          curr_value[MAX_OUTPUT_CHANNELS];
	int                          next_value[MAX_OUTPUT_CHANNELS];
	int                          history[BLOCK_HISTORY][MAX_OUTPUT_CHANNELS];
} audio_section_stream;

class GOAudioSection
{

private:
	template<class T>
	static void MonoUncompressedLinear(audio_section_stream *stream, float *output);
	template<class T>
	static void StereoUncompressedLinear(audio_section_stream *stream, float *output);
	template<class T>
	static void MonoUncompressedPolyphase(audio_section_stream *stream, float *output);
	template<class T>
	static void StereoUncompressedPolyphase(audio_section_stream *stream, float *output);
	template<bool format16>
	static void MonoCompressedLinear(audio_section_stream *stream, float *output);
	template<bool format16>
	static void StereoCompressedLinear(audio_section_stream *stream, float *output);

	static DecodeBlockFunction GetDecodeBlockFunction
		(unsigned channels
		,unsigned bits_per_sample
		,bool     compressed
		,interpolation_type interpolation
		);

	void Compress(bool format16);

	unsigned PickEndSegment(unsigned start_segment_index) const;

	void GetMaxAmplitudeAndDerivative();

	std::vector<audio_start_data_segment> m_StartSegments;
	std::vector<audio_end_data_segment>  m_EndSegments;

	/* Pointer to (size) bytes of data encoded in the format (type) */
	unsigned char             *m_Data;

	/* If this is a release section, it may contain an alignment table */
	GOrgueReleaseAlignTable   *m_ReleaseAligner;

	/* Number of significant bits in the decoded sample data */
	unsigned                   m_SampleFracBits;

	unsigned                   m_SampleCount;
	unsigned                   m_SampleRate;

	/* Type of the data which is stored in the data pointer */
	unsigned                   m_Compressed;
	unsigned                   m_BitsPerSample;
	unsigned                   m_Channels;

	/* The starting sample and derivatives for each channel (used in the
	 * compression and release-alignment schemes */
	int                        m_History[BLOCK_HISTORY][MAX_OUTPUT_CHANNELS];

	/* Size of the section in BYTES */
	GOrgueMemoryPool          &m_Pool;
	unsigned                   m_AllocSize;

	int                        m_MaxAbsAmplitude;
	int                        m_MaxAbsDerivative;

public:
	GOAudioSection(GOrgueMemoryPool& pool);
	~GOAudioSection();
	void ClearData();
	unsigned GetChannels() const;
	unsigned GetBytesPerSample() const;
	unsigned GetLength() const;
	bool LoadCache(GOrgueCache& cache);
	bool SaveCache(GOrgueCacheWriter& cache) const;

	/* Initialize a stream to play this audio section and seek into it using
	 * release alignment if available. */
	void InitAlignedStream
		(audio_section_stream           *stream
		,const audio_section_stream     *existing_stream
		) const;

	/* Initialize a stream to play this audio section */
	void InitStream
		(const struct resampler_coefs_s *resampler_coefs
		,audio_section_stream           *stream
		,float                           sample_rate_adjustment
		) const;

	/* Read an audio buffer from an audio section stream */
	static bool ReadBlock(audio_section_stream *stream, float *buffer);

	void Setup
		(const void                      *pcm_data
		,GOrgueWave::SAMPLE_FORMAT        pcm_data_format
		,unsigned                         pcm_data_channels
		,unsigned                         pcm_data_sample_rate
		,unsigned                         pcm_data_nb_samples
		,const std::vector<GO_WAVE_LOOP> *loop_points
		,const bool                       compress
		);

	bool IsOneshot() const;

	int GetSample
		(unsigned              position
		,unsigned              channel
		,DecompressionCache   *cache = NULL
		) const;

	float GetNormGain() const;
	bool SupportsStreamAlignment() const;
	void SetupStreamAlignment(const std::vector<const GOAudioSection*> &joinables);

};

inline
unsigned GOAudioSection::GetChannels() const
{
	return m_Channels;
}

inline
unsigned GOAudioSection::GetBytesPerSample() const
{
	return (m_Compressed) ? 0 : (m_BitsPerSample / 8);
}

inline
unsigned GOAudioSection::GetLength() const
{
	return m_SampleCount;
}

inline
bool GOAudioSection::IsOneshot() const
{
	return (m_EndSegments.size() == 1) && (m_EndSegments[0].next_start_segment_index < 0);
}

inline
int GOAudioSection::GetSample
	(unsigned              position
	,unsigned              channel
	,DecompressionCache   *cache
	) const
{
	if (!m_Compressed)
	{
		if (m_BitsPerSample == 8)
		{
			wxInt8* data = (wxInt8*)m_Data;
			return data[position * m_Channels + channel];
		}
		if (m_BitsPerSample == 16)
		{
			wxInt16* data = (wxInt16*)m_Data;
			return data[position * m_Channels + channel];
		}
		if (m_BitsPerSample == 24)
		{
			Int24* data = (Int24*)m_Data;
			return data[position * m_Channels + channel];
		}
	}
	else
	{
		DecompressionCache tmp;
		if (!cache)
		{
			cache = &tmp;
			InitDecompressionCache(*cache);
		}

		assert(m_BitsPerSample >= 12);
		DecompressTo
			(*cache
			,position
			,m_Data
			,m_Channels
			,(m_BitsPerSample >= 20)
			);
		return cache->value[channel];
	}

	assert(0 && "broken sampler type");
	return 0;
}

inline
float GOAudioSection::GetNormGain() const
{
	return scalbnf(1.0f, -((int)m_SampleFracBits));
}

inline
bool GOAudioSection::SupportsStreamAlignment() const
{
	return (m_ReleaseAligner != NULL);
}


#endif /* GOSOUNDAUDIOSECTION_H_ */
