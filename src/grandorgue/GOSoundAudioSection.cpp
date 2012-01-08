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

#include "GOSoundAudioSection.h"
#include "GOrgueCache.h"
#include "GOrgueCacheWriter.h"
#include "GOrgueReleaseAlignTable.h"
#include "GOrgueMemoryPool.h"
#include "GOSoundCompress.h"

GOAudioSection::GOAudioSection(GOrgueMemoryPool& pool)
	: m_Data(NULL)
	, m_ReleaseAligner(NULL)
	, m_Pool(pool)
{
	ClearData();
}

GOAudioSection::~GOAudioSection()
{
	ClearData();
}

void GOAudioSection::ClearData()
{
	m_AllocSize = 0;
	m_SampleCount = 0;
	m_SampleRate = 0;
	memset(m_History, 0, sizeof(m_History));
	m_BitsPerSample = 0;
	m_Compressed = false;
	m_Channels = 0;
	if (m_Data)
	{
		m_Pool.Free(m_Data);
		m_Data = NULL;
	}
	if (m_ReleaseAligner)
	{
		delete m_ReleaseAligner;
		m_ReleaseAligner = NULL;
	}
	m_SampleFracBits = 0;
	while (m_EndSegments.size())
	{
		if (m_EndSegments.back().end_data)
			m_Pool.Free(m_EndSegments.back().end_data);
		m_EndSegments.pop_back();
	}
	m_StartSegments.clear();
}

bool GOAudioSection::LoadCache(GOrgueCache& cache)
{
	if (!cache.Read(&m_AllocSize, sizeof(m_AllocSize)))
		return false;
	if (!cache.Read(&m_SampleCount, sizeof(m_SampleCount)))
		return false;
	if (!cache.Read(&m_SampleRate, sizeof(m_SampleRate)))
		return false;
	if (!cache.Read(&m_BitsPerSample, sizeof(m_BitsPerSample)))
		return false;
	if (!cache.Read(&m_Compressed, sizeof(m_Compressed)))
		return false;
	if (!cache.Read(&m_Channels, sizeof(m_Channels)))
		return false;
	if (!cache.Read(&m_History, sizeof(m_History)))
		return false;
	if (!cache.Read(&m_SampleFracBits, sizeof(m_SampleFracBits)))
		return false;
	m_Data = (unsigned char*)cache.ReadBlock(m_AllocSize);
	if (!m_Data)
		return false;

	const unsigned bytes_per_sample_frame = m_Channels * ((m_BitsPerSample + 7) / 8);

	unsigned temp;
	if (!cache.Read(&temp, sizeof(temp)))
		return false;
	for (unsigned i = 0; i < temp; i++)
	{
		audio_start_data_segment s;
		if (!cache.Read(&s, sizeof(s)))
			return false;
		m_StartSegments.push_back(s);
	}

	if (!cache.Read(&temp, sizeof(temp)))
		return false;
	for (unsigned i = 0; i < temp; i++)
	{
		audio_end_data_segment s;
		if (!cache.Read(&s.end_offset, sizeof(s.end_offset)))
			return false;
		if (!cache.Read(&s.next_start_segment_index, sizeof(s.next_start_segment_index)))
			return false;
		if (!cache.Read(&s.transition_offset, sizeof(s.transition_offset)))
			return false;
		s.end_data = (unsigned char*)cache.ReadBlock(4 * BLOCKS_PER_FRAME * bytes_per_sample_frame);
		if (!s.end_data)
			return false;
		m_EndSegments.push_back(s);
	}

	bool load_align_tracker;
	if (!cache.Read(&load_align_tracker, sizeof(load_align_tracker)))
		return false;
	m_ReleaseAligner = NULL;
	if (load_align_tracker)
	{
		m_ReleaseAligner = new GOrgueReleaseAlignTable();
		if (!m_ReleaseAligner->Load(cache))
			return false;
	}

	return true;
}

bool GOAudioSection::SaveCache(GOrgueCacheWriter& cache) const
{
	if (!cache.Write(&m_AllocSize, sizeof(m_AllocSize)))
		return false;
	if (!cache.Write(&m_SampleCount, sizeof(m_SampleCount)))
		return false;
	if (!cache.Write(&m_SampleRate, sizeof(m_SampleRate)))
		return false;
	if (!cache.Write(&m_BitsPerSample, sizeof(m_BitsPerSample)))
		return false;
	if (!cache.Write(&m_Compressed, sizeof(m_Compressed)))
		return false;
	if (!cache.Write(&m_Channels, sizeof(m_Channels)))
		return false;
	if (!cache.Write(&m_History, sizeof(m_History)))
		return false;
	if (!cache.Write(&m_SampleFracBits, sizeof(m_SampleFracBits)))
		return false;
	if (!cache.WriteBlock(m_Data, m_AllocSize))
		return false;

	const unsigned bytes_per_sample_frame = m_Channels * ((m_BitsPerSample + 7) / 8);

	unsigned temp;

	temp = m_StartSegments.size();
	if (!cache.Write(&temp, sizeof(unsigned)))
		return false;
	for (unsigned i = 0; i < temp; i++)
	{
		const audio_start_data_segment *s = &m_StartSegments[i];
		if (!cache.Write(s, sizeof(*s)))
			return false;
	}

	temp = m_EndSegments.size();
	if (!cache.Write(&temp, sizeof(temp)))
		return false;
	for (unsigned i = 0; i < temp; i++)
	{
		const audio_end_data_segment *s = &m_EndSegments[i];
		if (!cache.Write(&s->end_offset, sizeof(s->end_offset)))
			return false;
		if (!cache.Write(&s->next_start_segment_index, sizeof(s->next_start_segment_index)))
			return false;
		if (!cache.Write(&s->transition_offset, sizeof(s->transition_offset)))
			return false;
		if (!cache.WriteBlock(s->end_data, 4 * BLOCKS_PER_FRAME * bytes_per_sample_frame))
			return false;
	}

	const bool save_align_tracker = (m_ReleaseAligner != NULL);
	if (!cache.Write(&save_align_tracker, sizeof(save_align_tracker)))
		return false;

	if (save_align_tracker)
	{
		if (!m_ReleaseAligner->Save(cache))
			return false;
	}

	return true;
}

/* Block reading functions */

/* The block decode functions should provide whatever the normal resolution of
 * the audio is. The fade engine should ensure that this data is always brought
 * into the correct range. */

template<class T>
inline
void GOAudioSection::MonoUncompressed
	(audio_section_stream *stream
	,float                *output
	)
{
	// copy the sample buffer
	T* input = (T*)(stream->ptr);
	for (unsigned int i = 0
	    ;i < BLOCKS_PER_FRAME
	    ;i++
	    ,stream->position += stream->increment
	    ,output += 2
	    )
	{
		unsigned pos = (unsigned)stream->position;
		float fract = stream->position - pos;
		output[0] = input[pos] * (1 - fract) + input[pos + 1] * fract;
		output[1] = input[pos] * (1 - fract) + input[pos + 1] * fract;
	}

	// update sample history (for release alignment / compression)
	unsigned pos = (unsigned)stream->position;
	for (unsigned i = BLOCK_HISTORY; i > 0 && pos; i--, pos--)
		stream->history[i - 1][0] = input[pos];
}

template<class T>
inline
void GOAudioSection::StereoUncompressed
	(audio_section_stream *stream
	,float                *output
	)
{
	typedef T stereoSample[0][2];

	// "borrow" the output buffer to compute release alignment info
	stereoSample& input = (stereoSample&)*(T*)(stream->ptr);

	// copy the sample buffer
	for (unsigned int i = 0
	    ;i < BLOCKS_PER_FRAME
	    ;stream->position += stream->increment
	    ,output += 2
	    ,i++
	    )
	{
		unsigned pos = (unsigned)stream->position;
		float fract = stream->position - pos;
		output[0] = input[pos][0] * (1 - fract) + input[pos + 1][0] * fract;
		output[1] = input[pos][1] * (1 - fract) + input[pos + 1][1] * fract;
	}

	// update sample history (for release alignment / compression)
	unsigned pos = (unsigned)stream->position;
	for (unsigned i = BLOCK_HISTORY; i > 0 && pos; i--, pos--)
	{
		stream->history[i - 1][0] = input[pos][0];
		stream->history[i - 1][1] = input[pos][1];
	}
}

template<bool format16>
inline
void GOAudioSection::MonoCompressed
	(audio_section_stream *stream
	,float                *output
	)
{
	int history[BLOCK_HISTORY];
	unsigned hist_ptr = 0;

	for (unsigned int i = 0
	    ;i < BLOCKS_PER_FRAME
	    ;i++
	    ,stream->position += stream->increment
	    ,output += 2
	    )
	{
		unsigned pos = (unsigned)stream->position;
		float fract = stream->position - pos;

		while(stream->last_position <= pos + 1)
		{
			int val;
			if (format16)
				val = AudioReadCompressed16(stream->ptr);
			else
				val = AudioReadCompressed8(stream->ptr);

			stream->curr_value[0] = stream->next_value[0];
			stream->next_value[0] = stream->last_value[0] + val;
			stream->diff_value[0] = (stream->diff_value[0] +  val) / 2;
			stream->last_value[0] = stream->next_value[0] + stream->diff_value[0];
			history[hist_ptr] = stream->curr_value[0];

			stream->last_position++;
			hist_ptr++;
			if (hist_ptr >= BLOCK_HISTORY)
				hist_ptr = 0;
		}

		output[0] = stream->curr_value[0] * (1 - fract) + stream->next_value[0] * fract;
		output[1] = stream->curr_value[0] * (1 - fract) + stream->next_value[0] * fract;
	}

	// update sample history (for release alignment / compression)
	for (unsigned i = BLOCK_HISTORY; i > 0; i--)
	{
		if (hist_ptr == 0)
			hist_ptr = BLOCK_HISTORY - 1;
		else
			hist_ptr--;
		stream->history[i - 1][0] = history[hist_ptr];
	}
}

template<bool format16>
inline
void GOAudioSection::StereoCompressed
	(audio_section_stream *stream
	,float                *output
	)
{
	int history[BLOCK_HISTORY][2];
	unsigned hist_ptr = 0;

	// copy the sample buffer
	for (unsigned int i = 0; i < BLOCKS_PER_FRAME; stream->position += stream->increment, output += 2, i++)
	{
		unsigned pos = (unsigned)stream->position;
		float fract = stream->position - pos;


		while(stream->last_position <= pos + 1)
		{
			for(unsigned j = 0; j < 2; j++)
			{
				int val;
				if (format16)
					val = AudioReadCompressed16(stream->ptr);
				else
					val = AudioReadCompressed8(stream->ptr);
				stream->curr_value[j] = stream->next_value[j];
				stream->next_value[j] = stream->last_value[j] + val;
				stream->diff_value[j] = (stream->diff_value[j] +  val) / 2;
				stream->last_value[j] = stream->next_value[j] + stream->diff_value[j];
				history[hist_ptr][j] = stream->curr_value[j];
			}

			stream->last_position++;
			hist_ptr++;
			if (hist_ptr >= BLOCK_HISTORY)
				hist_ptr = 0;
		}

		output[0] = stream->curr_value[0] * (1 - fract) + stream->next_value[0] * fract;
		output[1] = stream->curr_value[1] * (1 - fract) + stream->next_value[1] * fract;
	}

	// update sample history (for release alignment / compression)
	for (unsigned i = BLOCK_HISTORY; i > 0; i--)
	{
		if (hist_ptr == 0)
			hist_ptr = BLOCK_HISTORY - 1;
		else
			hist_ptr--;
		stream->history[i - 1][0] = history[hist_ptr][0];
		stream->history[i - 1][1] = history[hist_ptr][1];
	}
}

inline
DecodeBlockFunction GOAudioSection::GetDecodeBlockFunction
	(unsigned channels
	,unsigned bits_per_sample
	,bool     compressed
	)
{
	if ((channels == 1) && (bits_per_sample == 8) && (!compressed))
		return MonoUncompressed<wxInt8>;
	if ((channels == 1) && (bits_per_sample == 16) && (!compressed))
		return MonoUncompressed<wxInt16>;
	if ((channels == 1) && (bits_per_sample == 24) && (!compressed))
		return MonoUncompressed<Int24>;

	if ((channels == 2) && (bits_per_sample == 8) && (!compressed))
		return StereoUncompressed<wxInt8>;
	if ((channels == 2) && (bits_per_sample == 16) && (!compressed))
		return StereoUncompressed<wxInt16>;
	if ((channels == 2) && (bits_per_sample == 24) && (!compressed))
		return StereoUncompressed<Int24>;

	if ((channels == 1) && (compressed))
	{
		if (bits_per_sample >= 20)
			return MonoCompressed<true>;

		assert(bits_per_sample >= 12);
		return MonoCompressed<false>;
	}

	if ((channels == 2) && (compressed))
	{
		if (bits_per_sample >= 20)
			return StereoCompressed<true>;

		assert(bits_per_sample >= 12);
		return StereoCompressed<false>;
	}

	assert(0 && "unsupported decoder configuration");
	return NULL;
}

inline
unsigned GOAudioSection::PickEndSegment(unsigned start_segment_index) const
{
	const unsigned x = abs(rand());
	for (unsigned i = 0; i < m_EndSegments.size(); i++)
	{
		const unsigned idx = (i + x) % m_EndSegments.size();
		const audio_end_data_segment *end = &m_EndSegments[idx];
		if (end->transition_offset >= m_StartSegments[start_segment_index].start_offset)
			return idx;
	}
	assert(0 && "should always find suitable end segment");
	return 0;
}

bool GOAudioSection::ReadBlock
	(audio_section_stream *stream
	,float                *buffer
	)
{
	if (stream->position >= stream->transition_position)
	{
		assert(stream->end_decode_call);

		/* Setup ptr and position required by the end-block */
		stream->ptr         = stream->end_ptr;
		stream->position   -= (float)stream->transition_position;
		stream->end_decode_call(stream, buffer);

		/* Restore the existing position */
		stream->position   += (float)stream->transition_position;
		if (stream->position >= stream->section_length)
		{
			if (stream->next_start_segment_index < 0)
				return 0;

			const unsigned next_index = stream->next_start_segment_index;
			const audio_start_data_segment *next =
					&stream->audio_section->m_StartSegments[next_index];

			/* Find a suitable end segment */
			const unsigned next_end_segment_index = stream->audio_section->PickEndSegment(next_index);
			const audio_end_data_segment *next_end =
					&stream->audio_section->m_EndSegments[next_end_segment_index];

			/* TODO: this is also where we need to copy startup information to the stream */
			while (stream->position >= stream->section_length)
				stream->position -= stream->section_length;

			/* Because we support linear interpolation, we can't assume that
			 * the position after a seek-back will be within a single block. In
			 * reality, this shouldn't ever be very much more than BLOCKS_PER_
			 * FRAME, but including a case for 2 times the size allows for a
			 * very large tuning. */
			assert(stream->position < 2 * BLOCKS_PER_FRAME);
			stream->ptr = stream->audio_section->m_Data + next->data_offset;
			stream->last_position = 0;
			stream->end_ptr = next_end->end_data;
			memcpy(stream->last_value, next->last_value, sizeof(stream->last_value));
			memcpy(stream->next_value, next->next_value, sizeof(stream->next_value));
			memcpy(stream->diff_value, next->diff_value, sizeof(stream->diff_value));
			memcpy(stream->curr_value, next->curr_value, sizeof(stream->curr_value));
			assert(next_end->end_offset >= next->start_offset);
			stream->transition_position
				= (next_end->transition_offset >= next->start_offset)
				? next_end->transition_offset - next->start_offset
				: 0;
			stream->section_length = 1 + next_end->end_offset - next->start_offset;
			stream->next_start_segment_index  = next_end->next_start_segment_index;
		}
	}
	else
	{
		assert(stream->decode_call);
		stream->decode_call(stream, buffer);
		assert(stream->position < stream->section_length);
	}

	return 1;
}

static
inline
unsigned wave_bits_per_sample(GOrgueWave::SAMPLE_FORMAT format)
{
	switch (format)
	{
	case GOrgueWave::SF_SIGNEDBYTE_8:
		return 8;
	case GOrgueWave::SF_SIGNEDSHORT_12:
		return 12;
	case GOrgueWave::SF_SIGNEDSHORT_16:
		return 16;
	case GOrgueWave::SF_SIGNEDINT24_20:
		return 20;
	case GOrgueWave::SF_SIGNEDINT24_24:
		return 24;
	case GOrgueWave::SF_IEEE_FLOAT:
		return 32;
	default:
		assert(0 && "bad sample format enumeration");
		return 0;
	}
}

static
inline
unsigned wave_bytes_per_sample(GOrgueWave::SAMPLE_FORMAT format)
{
	return (wave_bits_per_sample(format) + 7) / 8;
}

static
inline
void loop_memcpy
	(unsigned char*        dest
	,const unsigned char*  source
	,unsigned              source_len
	,unsigned              count
	)
{
	while (count > source_len)
	{
		memcpy
			(dest
			,source
			,source_len
			);
		dest += source_len;
		count -= source_len;
	}
	memcpy
		(dest
		,source
		,count
		);
}

void GOAudioSection::GetMaxAmplitudeAndDerivative()
{
	DecompressionCache cache;

	InitDecompressionCache(cache);
	m_MaxAbsAmplitude = 0;
	m_MaxAbsDerivative = 0;

	int f_p = 0; /* to avoid compiler warning */
	for (unsigned int i = 0; i < m_SampleCount; i++)
	{
		/* Get sum of amplitudes in channels */
		int f = 0;
		for (unsigned int j = 0; j < m_Channels; j++)
			f += GetSample(i, j, &cache);

		if (abs(f) > m_MaxAbsAmplitude)
			m_MaxAbsAmplitude = abs(f);

		if (i != 0)
		{
			/* Get v */
			int v = f - f_p;
			if (abs(v) > m_MaxAbsDerivative)
				m_MaxAbsDerivative = abs(v);
		}
		f_p = f;
	}
}

void GOAudioSection::Setup
	(const void                      *pcm_data
	,const GOrgueWave::SAMPLE_FORMAT  pcm_data_format
	,const unsigned                   pcm_data_channels
	,const unsigned                   pcm_data_sample_rate
	,const unsigned                   pcm_data_nb_samples
	,const std::vector<GO_WAVE_LOOP> *loop_points
	,const bool                       compress
	)
{
	if (pcm_data_channels < 1 || pcm_data_channels > 2)
		throw (wxString)_("< More than 2 channels in");

	assert(pcm_data_nb_samples > 0);

	const unsigned bytes_per_sample = wave_bytes_per_sample(pcm_data_format);
	const unsigned bytes_per_sample_frame = bytes_per_sample * pcm_data_channels;

	unsigned total_alloc_samples = pcm_data_nb_samples;
	/* Create a start segment */
	{
		audio_start_data_segment start_seg;
		start_seg.data_offset = 0;
		start_seg.start_offset = 0;
		memset(&start_seg.history, 0, sizeof(start_seg.history));
		m_StartSegments.push_back(start_seg);
	}

	if ((loop_points) && (loop_points->size() >= 1))
	{
		/* Setup the loops and find the amount of data we need to store in the
		 * main block. */
		unsigned min_reqd_samples = 0;
		for (unsigned i = 0; i < loop_points->size(); i++)
		{
			audio_start_data_segment start_seg;
			audio_end_data_segment end_seg;
			const GO_WAVE_LOOP& loop = (*loop_points)[i];
			if (loop.end_sample + 1 > min_reqd_samples)
				min_reqd_samples = loop.end_sample + 1;

			start_seg.start_offset           = loop.start_sample;
			start_seg.data_offset            = loop.start_sample * bytes_per_sample_frame;
			end_seg.end_offset               = loop.end_sample;
			end_seg.next_start_segment_index = i + 1;
			end_seg.end_data                 = (unsigned char*)m_Pool.Alloc(4 * BLOCKS_PER_FRAME * bytes_per_sample_frame);
			end_seg.transition_offset
				= (end_seg.end_offset - start_seg.start_offset > 2 * BLOCKS_PER_FRAME)
				? end_seg.end_offset - 2 * BLOCKS_PER_FRAME
				: start_seg.start_offset;

			if (!end_seg.end_data)
				throw (wxString)_("out of memory");

			const unsigned copy_len = 1 + end_seg.end_offset - end_seg.transition_offset;
			const unsigned loop_length = 1 + end_seg.end_offset - start_seg.start_offset;

			memcpy
				(end_seg.end_data
				,((const unsigned char*)pcm_data) + end_seg.transition_offset * bytes_per_sample_frame
				,copy_len * bytes_per_sample_frame
				);
			loop_memcpy
				(((unsigned char*)end_seg.end_data) + copy_len * bytes_per_sample_frame
				,((const unsigned char*)pcm_data) + loop.start_sample * bytes_per_sample_frame
				,loop_length * bytes_per_sample_frame
				,(4 * BLOCKS_PER_FRAME - copy_len) * bytes_per_sample_frame
				);

			m_StartSegments.push_back(start_seg);
			m_EndSegments.push_back(end_seg);
		}

		/* There is no need to store any samples after the end of the last loop. */
		if (total_alloc_samples > min_reqd_samples)
			total_alloc_samples = min_reqd_samples;
	}
	else
	{
		/* Create a default end segment */
		audio_end_data_segment end_seg;
		end_seg.end_offset               = pcm_data_nb_samples - 1;
		end_seg.next_start_segment_index = -1;
		end_seg.end_data                 = (unsigned char*)m_Pool.Alloc(4 * BLOCKS_PER_FRAME * bytes_per_sample_frame);
		end_seg.transition_offset
			= (end_seg.end_offset > 2 * BLOCKS_PER_FRAME)
			? end_seg.end_offset - 2 * BLOCKS_PER_FRAME
			: 0;

		const unsigned copy_len = 1 + end_seg.end_offset - end_seg.transition_offset;

		memcpy
			(end_seg.end_data
			,((const unsigned char*)pcm_data) + end_seg.transition_offset * bytes_per_sample_frame
			,copy_len * bytes_per_sample_frame
			);
		memset
			(((unsigned char*)end_seg.end_data) + copy_len * bytes_per_sample_frame
			,0
			,(4 * BLOCKS_PER_FRAME - copy_len) * bytes_per_sample_frame
			);

		m_EndSegments.push_back(end_seg);
	}

	m_AllocSize = total_alloc_samples * bytes_per_sample_frame;
	m_Data = (unsigned char*)m_Pool.Alloc(m_AllocSize);
	if (m_Data == NULL)
		throw (wxString)_("< out of memory allocating attack");
	m_SampleRate     = pcm_data_sample_rate;
	m_SampleCount    = total_alloc_samples;
	m_BitsPerSample  = wave_bits_per_sample(pcm_data_format);
	m_SampleFracBits = m_BitsPerSample - 1;
	m_Channels       = pcm_data_channels;
	m_Compressed     = false;

	/* Store the main data blob. */
	memcpy
		(m_Data
		,pcm_data
		,m_AllocSize
		);

	GetMaxAmplitudeAndDerivative();

	if ((compress) && (m_BitsPerSample >= 12))
		Compress(m_BitsPerSample >= 20);

}

void GOAudioSection::Compress(bool format16)
{
	unsigned char* data = (unsigned char*)malloc(m_AllocSize);
	if (!data)
		return;

	unsigned output_len = 0;
	int diff[MAX_OUTPUT_CHANNELS];
	int last[MAX_OUTPUT_CHANNELS];
	int prev[MAX_OUTPUT_CHANNELS];
	memset(diff, 0, sizeof(diff));
	memset(last, 0, sizeof(last));
	memset(prev, 0, sizeof(prev));

	for (unsigned i = 0; i < m_SampleCount; i++)
	{
		int this_val[MAX_OUTPUT_CHANNELS];
		this_val[0] = GetSample(i, 0);
		if (m_Channels > 1)
			this_val[1] = GetSample(i, 1);

		for (unsigned j = 0; j < m_StartSegments.size(); j++)
		{
			if (m_StartSegments[j].start_offset == i)
			{
				m_StartSegments[j].data_offset = output_len;
				m_StartSegments[j].last_value[0] = last[0];
				m_StartSegments[j].last_value[1] = last[1];
				m_StartSegments[j].diff_value[0] = diff[0];
				m_StartSegments[j].diff_value[1] = diff[1];
				m_StartSegments[j].next_value[0] = this_val[0];
				m_StartSegments[j].next_value[1] = this_val[1];
				m_StartSegments[j].curr_value[0] = prev[0];
				m_StartSegments[j].curr_value[1] = prev[1];
			}
		}

		prev[0] = this_val[0];
		prev[1] = this_val[1];

		for (unsigned j = 0; j < m_Channels; j++)
		{
			int val = this_val[j];
			int encode = val - last[j];
			diff[j] = (diff[j] + val - last[j]) / 2;
			last[j] = val + diff[j];

			if (format16)
				AudioWriteCompressed16(data, output_len, encode);
			else
				AudioWriteCompressed8(data, output_len, encode);

			/* Early abort if the compressed data will be larger than the
			 * uncompressed data. */
			if (output_len + 10 >= m_AllocSize)
			{
				free(data);
				return;
			}
		}
	}

#if 0
	GOAudioSection new_section = *this;
	new_section.m_Data = data;
	new_section.m_AllocSize = output_len;
	new_section.m_Compressed = true;
	DecompressionCache tmp;
	InitDecompressionCache(tmp);

	for (unsigned i = 0; i < m_SampleCount; i++)
	{
		for (unsigned j = 0; j < m_Channels; j++)
		{
			int old_value = GetSample(i, j);
			int new_value = new_section.GetSample(i, j, &tmp);
			if (old_value != new_value)
				wxLogError(wxT("%d %d: %d %d"), i, j, old_value, new_value);
		}
	}

	new_section.m_Data = NULL;

	wxLogError(wxT("Compress: %d %d"), m_AllocSize, output_len);
#endif

	m_Data = (unsigned char*)m_Pool.Realloc(m_Data, m_AllocSize, output_len);
	memcpy(m_Data, data, output_len);
	m_AllocSize = output_len;
	m_Compressed = true;
	free(data);
}

void GOAudioSection::SetupStreamAlignment
	(const std::vector<const GOAudioSection*> &joinables
	)
{
	if (m_ReleaseAligner)
	{
		delete m_ReleaseAligner;
		m_ReleaseAligner = NULL;
	}

	int max_amplitude = m_MaxAbsAmplitude;
	int max_derivative = m_MaxAbsDerivative;
	for (unsigned i = 0; i < joinables.size(); i++)
	{
		if (joinables[i]->m_MaxAbsAmplitude > max_amplitude)
			max_amplitude = joinables[i]->m_MaxAbsAmplitude;
		if (joinables[i]->m_MaxAbsDerivative > max_derivative)
			max_derivative = joinables[i]->m_MaxAbsDerivative;
	}

	if ((max_derivative != 0) && (max_amplitude != 0))
	{
		m_ReleaseAligner = new GOrgueReleaseAlignTable();
		m_ReleaseAligner->ComputeTable
			(*this
			,max_amplitude
			,max_derivative
			,m_SampleRate
			);
	}
}


void GOAudioSection::InitStream
	(audio_section_stream    *stream
	,float                    sample_rate_adjustment
	) const
{
	stream->audio_section = this;

	const audio_start_data_segment &start = m_StartSegments[0];
	const audio_end_data_segment &end     = m_EndSegments[PickEndSegment(0)];
	assert(end.transition_offset > start.start_offset);

	stream->ptr                      = stream->audio_section->m_Data + start.data_offset;
	stream->transition_position      = end.transition_offset - start.start_offset;
	stream->section_length           = 1 + end.end_offset - start.start_offset;
	stream->next_start_segment_index = end.next_start_segment_index;
	stream->end_ptr                  = end.end_data;
	stream->increment                = sample_rate_adjustment * m_SampleRate;
	stream->position                 = 0.0f;
	stream->decode_call              = GetDecodeBlockFunction(m_Channels, m_BitsPerSample, m_Compressed);
	stream->end_decode_call          = GetDecodeBlockFunction(m_Channels, m_BitsPerSample, false);
	stream->last_position = 0;
	memset(stream->curr_value, 0, sizeof(stream->curr_value));
	memset(stream->next_value, 0, sizeof(stream->next_value));
	memset(stream->diff_value, 0, sizeof(stream->diff_value));
	memset(stream->last_value, 0, sizeof(stream->last_value));
	memcpy
		(stream->history
		,m_History
		,sizeof(stream->history)
		);
}

void GOAudioSection::InitAlignedStream
	(audio_section_stream       *stream
	,const audio_section_stream *existing_stream
	) const
{
	stream->audio_section = this;

	const audio_start_data_segment &start = m_StartSegments[0];
	const audio_end_data_segment &end     = m_EndSegments[PickEndSegment(0)];
	assert(end.transition_offset > start.start_offset);

	stream->ptr                      = stream->audio_section->m_Data + start.data_offset;
	stream->transition_position      = end.transition_offset - start.start_offset;
	stream->section_length           = 1 + end.end_offset - start.start_offset;
	stream->next_start_segment_index = end.next_start_segment_index;
	stream->end_ptr                  = end.end_data;
	/* Translate increment in case of differing sample rates */
	stream->increment                = (existing_stream->increment / existing_stream->audio_section->m_SampleRate) * m_SampleRate;
	stream->position                 = 0.0f;
	stream->decode_call              = GetDecodeBlockFunction(m_Channels, m_BitsPerSample, m_Compressed);
	stream->end_decode_call          = GetDecodeBlockFunction(m_Channels, m_BitsPerSample, false);
	stream->last_position = 0;
	memset(stream->curr_value, 0, sizeof(stream->curr_value));
	memset(stream->next_value, 0, sizeof(stream->next_value));
	memset(stream->diff_value, 0, sizeof(stream->diff_value));
	memset(stream->last_value, 0, sizeof(stream->last_value));
	memcpy
		(stream->history
		,m_History
		,sizeof(stream->history)
		);
	if (!m_ReleaseAligner)
	{
		return;
	}
	m_ReleaseAligner->SetupRelease(*stream, *existing_stream);
}


