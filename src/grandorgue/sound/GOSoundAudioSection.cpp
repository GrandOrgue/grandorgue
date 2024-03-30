/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundAudioSection.h"

#include <wx/intl.h>
#include <wx/log.h>

#include "loader/cache/GOCache.h"
#include "loader/cache/GOCacheWriter.h"

#include "loader/GOLoaderFilename.h"
#include "model/GOCacheObject.h"

#include "GOAlloc.h"
#include "GOMemoryPool.h"
#include "GOSampleStatistic.h"
#include "GOSoundCompress.h"
#include "GOSoundReleaseAlignTable.h"
#include "GOSoundResample.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static unsigned limited_diff(unsigned a, unsigned b) {
  if (a > b)
    return a - b;
  else
    return 0;
}

GOSoundAudioSection::GOSoundAudioSection(GOMemoryPool &pool)
  : m_Data(NULL),
    m_ReleaseAligner(NULL),
    m_ReleaseStartSegment(0),
    m_Pool(pool) {
  ClearData();
}

GOSoundAudioSection::~GOSoundAudioSection() { ClearData(); }

void GOSoundAudioSection::ClearData() {
  m_AllocSize = 0;
  m_SampleCount = 0;
  m_SampleRate = 0;
  m_BitsPerSample = 0;
  m_BytesPerSample = 0;
  m_WaveTremulantStateFor = BOOL3_DEFAULT;
  m_IsCompressed = false;
  m_Channels = 0;
  if (m_Data) {
    m_Pool.Free(m_Data);
    m_Data = NULL;
  }
  if (m_ReleaseAligner) {
    delete m_ReleaseAligner;
    m_ReleaseAligner = NULL;
  }
  m_SampleFracBits = 0;
  while (m_EndSegments.size()) {
    if (m_EndSegments.back().end_data)
      m_Pool.Free(m_EndSegments.back().end_data);
    m_EndSegments.pop_back();
  }
  m_StartSegments.clear();
  m_ReleaseCrossfadeLength = 0;
}

bool GOSoundAudioSection::LoadCache(GOCache &cache) {
  if (!cache.Read(&m_AllocSize, sizeof(m_AllocSize)))
    return false;
  if (!cache.Read(&m_SampleCount, sizeof(m_SampleCount)))
    return false;
  if (!cache.Read(&m_SampleRate, sizeof(m_SampleRate)))
    return false;
  if (!cache.Read(&m_BitsPerSample, sizeof(m_BitsPerSample)))
    return false;
  if (!cache.Read(&m_BytesPerSample, sizeof(m_BytesPerSample)))
    return false;
  if (!cache.Read(&m_WaveTremulantStateFor, sizeof(m_WaveTremulantStateFor)))
    return false;
  if (!cache.Read(&m_IsCompressed, sizeof(m_IsCompressed)))
    return false;
  if (!cache.Read(&m_Channels, sizeof(m_Channels)))
    return false;
  if (!cache.Read(&m_SampleFracBits, sizeof(m_SampleFracBits)))
    return false;
  if (!cache.Read(&m_MaxAmplitude, sizeof(m_MaxAmplitude)))
    return false;
  if (!cache.Read(&m_ReleaseStartSegment, sizeof(m_ReleaseStartSegment)))
    return false;
  if (!cache.Read(&m_ReleaseCrossfadeLength, sizeof(m_ReleaseCrossfadeLength)))
    return false;
  m_Data = (unsigned char *)cache.ReadBlock(m_AllocSize);
  if (!m_Data)
    return false;

  unsigned temp;
  if (!cache.Read(&temp, sizeof(temp)))
    return false;
  for (unsigned i = 0; i < temp; i++) {
    audio_start_data_segment s;
    if (!cache.Read(&s, sizeof(s)))
      return false;
    m_StartSegments.push_back(s);
  }

  if (!cache.Read(&temp, sizeof(temp)))
    return false;
  for (unsigned i = 0; i < temp; i++) {
    audio_end_data_segment s;
    if (!cache.Read(&s.end_offset, sizeof(s.end_offset)))
      return false;
    if (!cache.Read(
          &s.next_start_segment_index, sizeof(s.next_start_segment_index)))
      return false;
    if (!cache.Read(&s.transition_offset, sizeof(s.transition_offset)))
      return false;
    if (!cache.Read(&s.end_loop_length, sizeof(s.end_loop_length)))
      return false;
    if (!cache.Read(&s.read_end, sizeof(s.read_end)))
      return false;
    if (!cache.Read(&s.end_pos, sizeof(s.end_pos)))
      return false;
    if (!cache.Read(&s.end_size, sizeof(s.end_size)))
      return false;
    s.end_data = (unsigned char *)cache.ReadBlock(s.end_size);
    if (!s.end_data)
      return false;
    s.end_ptr = s.end_data - m_BytesPerSample * s.transition_offset;

    m_EndSegments.push_back(s);
  }

  bool load_align_tracker;
  if (!cache.Read(&load_align_tracker, sizeof(load_align_tracker)))
    return false;
  m_ReleaseAligner = NULL;
  if (load_align_tracker) {
    m_ReleaseAligner = new GOSoundReleaseAlignTable();
    if (!m_ReleaseAligner->Load(cache))
      return false;
  }

  return true;
}

bool GOSoundAudioSection::SaveCache(GOCacheWriter &cache) const {
  if (!cache.Write(&m_AllocSize, sizeof(m_AllocSize)))
    return false;
  if (!cache.Write(&m_SampleCount, sizeof(m_SampleCount)))
    return false;
  if (!cache.Write(&m_SampleRate, sizeof(m_SampleRate)))
    return false;
  if (!cache.Write(&m_BitsPerSample, sizeof(m_BitsPerSample)))
    return false;
  if (!cache.Write(&m_BytesPerSample, sizeof(m_BytesPerSample)))
    return false;
  if (!cache.Write(&m_WaveTremulantStateFor, sizeof(m_WaveTremulantStateFor)))
    return false;
  if (!cache.Write(&m_IsCompressed, sizeof(m_IsCompressed)))
    return false;
  if (!cache.Write(&m_Channels, sizeof(m_Channels)))
    return false;
  if (!cache.Write(&m_SampleFracBits, sizeof(m_SampleFracBits)))
    return false;
  if (!cache.Write(&m_MaxAmplitude, sizeof(m_MaxAmplitude)))
    return false;
  if (!cache.Write(&m_ReleaseStartSegment, sizeof(m_ReleaseStartSegment)))
    return false;
  if (!cache.Write(&m_ReleaseCrossfadeLength, sizeof(m_ReleaseCrossfadeLength)))
    return false;
  if (!cache.WriteBlock(m_Data, m_AllocSize))
    return false;

  unsigned temp;

  temp = m_StartSegments.size();
  if (!cache.Write(&temp, sizeof(unsigned)))
    return false;
  for (unsigned i = 0; i < temp; i++) {
    const audio_start_data_segment *s = &m_StartSegments[i];
    if (!cache.Write(s, sizeof(*s)))
      return false;
  }

  temp = m_EndSegments.size();
  if (!cache.Write(&temp, sizeof(temp)))
    return false;
  for (unsigned i = 0; i < temp; i++) {
    const audio_end_data_segment *s = &m_EndSegments[i];
    if (!cache.Write(&s->end_offset, sizeof(s->end_offset)))
      return false;
    if (!cache.Write(
          &s->next_start_segment_index, sizeof(s->next_start_segment_index)))
      return false;
    if (!cache.Write(&s->transition_offset, sizeof(s->transition_offset)))
      return false;
    if (!cache.Write(&s->end_loop_length, sizeof(s->end_loop_length)))
      return false;
    if (!cache.Write(&s->read_end, sizeof(s->read_end)))
      return false;
    if (!cache.Write(&s->end_pos, sizeof(s->end_pos)))
      return false;
    if (!cache.Write(&s->end_size, sizeof(s->end_size)))
      return false;
    if (!cache.WriteBlock(s->end_data, s->end_size))
      return false;
  }

  const bool save_align_tracker = (m_ReleaseAligner != NULL);
  if (!cache.Write(&save_align_tracker, sizeof(save_align_tracker)))
    return false;

  if (save_align_tracker) {
    if (!m_ReleaseAligner->Save(cache))
      return false;
  }

  return true;
}

/* Block reading functions */

/* The block decode functions should provide whatever the normal resolution of
 * the audio is. The fade engine should ensure that this data is always brought
 * into the correct range. */

template <class T>
inline void GOSoundAudioSection::MonoUncompressedLinear(
  audio_section_stream *stream, float *output, unsigned int n_blocks) {
  // copy the sample buffer
  T *input = (T *)(stream->ptr);
  for (unsigned int i = 0; i < n_blocks; i++,
                    stream->position_fraction += stream->increment_fraction,
                    output += 2) {
    stream->position_index += stream->position_fraction >> UPSAMPLE_BITS;
    stream->position_fraction
      = stream->position_fraction & (UPSAMPLE_FACTOR - 1);
    unsigned pos = stream->position_index;
    output[0] = input[pos]
        * stream->resample_coefs->linear[stream->position_fraction][1]
      + input[pos + 1]
        * stream->resample_coefs->linear[stream->position_fraction][0];
    output[1] = output[0];
  }

  stream->position_index += stream->position_fraction >> UPSAMPLE_BITS;
  stream->position_fraction = stream->position_fraction & (UPSAMPLE_FACTOR - 1);
}

template <class T>
inline void GOSoundAudioSection::StereoUncompressedLinear(
  audio_section_stream *stream, float *output, unsigned int n_blocks) {
  typedef T stereoSample[][2];

  // "borrow" the output buffer to compute release alignment info
  stereoSample &input = (stereoSample &)*(T *)(stream->ptr);

  // copy the sample buffer
  for (unsigned int i = 0; i < n_blocks;
       stream->position_fraction += stream->increment_fraction,
                    output += 2,
                    i++) {
    stream->position_index += stream->position_fraction >> UPSAMPLE_BITS;
    stream->position_fraction
      = stream->position_fraction & (UPSAMPLE_FACTOR - 1);
    unsigned pos = stream->position_index;
    output[0] = input[pos][0]
        * stream->resample_coefs->linear[stream->position_fraction][1]
      + input[pos + 1][0]
        * stream->resample_coefs->linear[stream->position_fraction][0];
    output[1] = input[pos][1]
        * stream->resample_coefs->linear[stream->position_fraction][1]
      + input[pos + 1][1]
        * stream->resample_coefs->linear[stream->position_fraction][0];
  }

  stream->position_index += stream->position_fraction >> UPSAMPLE_BITS;
  stream->position_fraction = stream->position_fraction & (UPSAMPLE_FACTOR - 1);
}

template <class T>
inline void GOSoundAudioSection::MonoUncompressedPolyphase(
  audio_section_stream *stream, float *output, unsigned int n_blocks) {
  // copy the sample buffer
  T *input = ((T *)stream->ptr);
  const float *coef = stream->resample_coefs->coefs;
  for (unsigned i = 0; i < n_blocks; ++i,
                output += 2,
                stream->position_fraction += stream->increment_fraction) {
    stream->position_index += stream->position_fraction >> UPSAMPLE_BITS;
    stream->position_fraction
      = stream->position_fraction & (UPSAMPLE_FACTOR - 1);
    float out1 = 0.0f;
    float out2 = 0.0f;
    float out3 = 0.0f;
    float out4 = 0.0f;
    const float *coef_set = &coef[stream->position_fraction << SUBFILTER_BITS];
    T *in_set = &input[stream->position_index];
    for (unsigned j = 0; j < SUBFILTER_TAPS; j += 4) {
      out1 += in_set[j] * coef_set[j];
      out2 += in_set[j + 1] * coef_set[j + 1];
      out3 += in_set[j + 2] * coef_set[j + 2];
      out4 += in_set[j + 3] * coef_set[j + 3];
    }
    output[0] = out1 + out2 + out3 + out4;
    output[1] = output[0];
  }

  stream->position_index += stream->position_fraction >> UPSAMPLE_BITS;
  stream->position_fraction = stream->position_fraction & (UPSAMPLE_FACTOR - 1);
}

template <class T>
inline void GOSoundAudioSection::StereoUncompressedPolyphase(
  audio_section_stream *stream, float *output, unsigned int n_blocks) {
  typedef T stereoSample[][2];

  // copy the sample buffer
  stereoSample &input = (stereoSample &)*(T *)(stream->ptr);
  const float *coef = stream->resample_coefs->coefs;
  for (unsigned i = 0; i < n_blocks; ++i,
                output += 2,
                stream->position_fraction += stream->increment_fraction) {
    stream->position_index += stream->position_fraction >> UPSAMPLE_BITS;
    stream->position_fraction
      = stream->position_fraction & (UPSAMPLE_FACTOR - 1);
    float out1 = 0.0f;
    float out2 = 0.0f;
    float out3 = 0.0f;
    float out4 = 0.0f;
    const float *coef_set = &coef[stream->position_fraction << SUBFILTER_BITS];
    T *in_set = (T *)&input[stream->position_index][0];
    for (unsigned j = 0; j < SUBFILTER_TAPS; j += 4) {
      out1 += in_set[2 * j] * coef_set[j];
      out2 += in_set[2 * j + 1] * coef_set[j];
      out3 += in_set[2 * j + 2] * coef_set[j + 1];
      out4 += in_set[2 * j + 3] * coef_set[j + 1];
      out1 += in_set[2 * j + 4] * coef_set[j + 2];
      out2 += in_set[2 * j + 5] * coef_set[j + 2];
      out3 += in_set[2 * j + 6] * coef_set[j + 3];
      out4 += in_set[2 * j + 7] * coef_set[j + 3];
    }
    output[0] = out1 + out3;
    output[1] = out2 + out4;
  }

  stream->position_index += stream->position_fraction >> UPSAMPLE_BITS;
  stream->position_fraction = stream->position_fraction & (UPSAMPLE_FACTOR - 1);
}

template <bool format16>
inline void GOSoundAudioSection::MonoCompressedLinear(
  audio_section_stream *stream, float *output, unsigned int n_blocks) {
  for (unsigned int i = 0; i < n_blocks; i++,
                    stream->position_fraction += stream->increment_fraction,
                    output += 2) {
    stream->position_index += stream->position_fraction >> UPSAMPLE_BITS;
    stream->position_fraction
      = stream->position_fraction & (UPSAMPLE_FACTOR - 1);

    while (stream->cache.position <= stream->position_index + 1) {
      DecompressionStep(stream->cache, 1, format16);
    }

    output[0] = stream->cache.prev[0]
        * stream->resample_coefs->linear[stream->position_fraction][1]
      + stream->cache.value[0]
        * stream->resample_coefs->linear[stream->position_fraction][0];
    output[1] = output[0];
  }

  stream->position_index += stream->position_fraction >> UPSAMPLE_BITS;
  stream->position_fraction = stream->position_fraction & (UPSAMPLE_FACTOR - 1);
}

template <bool format16>
inline void GOSoundAudioSection::StereoCompressedLinear(
  audio_section_stream *stream, float *output, unsigned int n_blocks) {
  // copy the sample buffer
  for (unsigned int i = 0; i < n_blocks;
       stream->position_fraction += stream->increment_fraction,
                    output += 2,
                    i++) {
    stream->position_index += stream->position_fraction >> UPSAMPLE_BITS;
    stream->position_fraction
      = stream->position_fraction & (UPSAMPLE_FACTOR - 1);

    while (stream->cache.position <= stream->position_index + 1) {
      DecompressionStep(stream->cache, 2, format16);
    }

    output[0] = stream->cache.prev[0]
        * stream->resample_coefs->linear[stream->position_fraction][1]
      + stream->cache.value[0]
        * stream->resample_coefs->linear[stream->position_fraction][0];
    output[1] = stream->cache.prev[1]
        * stream->resample_coefs->linear[stream->position_fraction][1]
      + stream->cache.value[1]
        * stream->resample_coefs->linear[stream->position_fraction][0];
  }

  stream->position_index += stream->position_fraction >> UPSAMPLE_BITS;
  stream->position_fraction = stream->position_fraction & (UPSAMPLE_FACTOR - 1);
}

inline DecodeBlockFunction GOSoundAudioSection::GetDecodeBlockFunction(
  unsigned channels,
  unsigned bits_per_sample,
  bool compressed,
  interpolation_type interpolation,
  bool is_end) {
  if (compressed && !is_end) {
    /* TODO: Add support for polyphase compressed decoders. Fallback to
     * linear interpolation for now. */
    if (channels == 1) {
      if (bits_per_sample >= 20)
        return MonoCompressedLinear<true>;

      assert(bits_per_sample >= 12);
      return MonoCompressedLinear<false>;
    } else if (channels == 2) {
      if (bits_per_sample >= 20)
        return StereoCompressedLinear<true>;

      assert(bits_per_sample >= 12);
      return StereoCompressedLinear<false>;
    }
  } else {
    if (interpolation == GO_POLYPHASE_INTERPOLATION && !compressed) {
      if (channels == 1) {
        if (bits_per_sample <= 8)
          return MonoUncompressedPolyphase<GOInt8>;
        if (bits_per_sample <= 16)
          return MonoUncompressedPolyphase<GOInt16>;
        if (bits_per_sample <= 24)
          return MonoUncompressedPolyphase<GOInt24>;
      } else if (channels == 2) {
        if (bits_per_sample <= 8)
          return StereoUncompressedPolyphase<GOInt8>;
        if (bits_per_sample <= 16)
          return StereoUncompressedPolyphase<GOInt16>;
        if (bits_per_sample <= 24)
          return StereoUncompressedPolyphase<GOInt24>;
      }
    } else {
      if (channels == 1) {
        if (bits_per_sample <= 8)
          return MonoUncompressedLinear<GOInt8>;
        if (bits_per_sample <= 16)
          return MonoUncompressedLinear<GOInt16>;
        if (bits_per_sample <= 24)
          return MonoUncompressedLinear<GOInt24>;
      } else if (channels == 2) {
        if (bits_per_sample <= 8)
          return StereoUncompressedLinear<GOInt8>;
        if (bits_per_sample <= 16)
          return StereoUncompressedLinear<GOInt16>;
        if (bits_per_sample <= 24)
          return StereoUncompressedLinear<GOInt24>;
      }
    }
  }

  assert(0 && "unsupported decoder configuration");
  return NULL;
}

inline unsigned GOSoundAudioSection::PickEndSegment(
  unsigned start_segment_index) const {
  const unsigned x = abs(rand());
  for (unsigned i = 0; i < m_EndSegments.size(); i++) {
    const unsigned idx = (i + x) % m_EndSegments.size();
    const audio_end_data_segment *end = &m_EndSegments[idx];
    if (
      end->transition_offset
      >= m_StartSegments[start_segment_index].start_offset)
      return idx;
  }
  assert(0 && "should always find suitable end segment");
  return 0;
}

bool GOSoundAudioSection::ReadBlock(
  audio_section_stream *stream, float *buffer, unsigned int n_blocks) {
  while (n_blocks > 0) {
    if (stream->position_index >= stream->transition_position) {
      assert(stream->end_decode_call);

      /* Setup ptr and position required by the end-block */
      stream->ptr = stream->end_ptr;
      unsigned len
        = ((UPSAMPLE_FACTOR / 2)
           + ((stream->end_pos - stream->position_index) << UPSAMPLE_BITS))
        / stream->increment_fraction;
      if (len == 0)
        len = 1;
      len = std::min(len, n_blocks);
      stream->end_decode_call(stream, buffer, len);
      buffer += 2 * len;
      n_blocks -= len;

      if (stream->position_index >= stream->end_pos) {
        const audio_end_data_segment *end = stream->end_seg;
        if (end->next_start_segment_index < 0) {
          for (unsigned i = 0; i < n_blocks * 2; i++)
            buffer[i] = 0;
          return 0;
        }

        stream->position_index -= end->end_offset + 1;
        while (stream->position_index >= end->end_loop_length)
          stream->position_index -= end->end_loop_length;

        const unsigned next_index = end->next_start_segment_index;
        const audio_start_data_segment *next
          = &stream->audio_section->m_StartSegments[next_index];

        /* Find a suitable end segment */
        const unsigned next_end_segment_index
          = stream->audio_section->PickEndSegment(next_index);
        const audio_end_data_segment *next_end
          = &stream->audio_section->m_EndSegments[next_end_segment_index];

        stream->position_index += next->start_offset;
        stream->ptr = stream->audio_section->m_Data;
        stream->cache = next->cache;
        stream->cache.ptr
          = stream->audio_section->m_Data + (intptr_t)stream->cache.ptr;
        assert(next_end->end_offset >= next->start_offset);
        stream->transition_position = next_end->transition_offset;
        stream->end_ptr = next_end->end_ptr;
        stream->read_end = limited_diff(next_end->read_end, stream->margin);
        stream->end_pos = next_end->end_pos - stream->margin;
        stream->end_seg = next_end;
      }
    } else {
      assert(stream->decode_call);
      unsigned len
        = ((stream->read_end - stream->position_index) << UPSAMPLE_BITS)
        / stream->increment_fraction;
      if (len == 0)
        len = 1;
      len = std::min(len, n_blocks);
      stream->decode_call(stream, buffer, len);
      buffer += 2 * len;
      n_blocks -= len;
    }
  }

  return 1;
}

static inline unsigned wave_bits_per_sample(GOWave::SAMPLE_FORMAT format) {
  switch (format) {
  case GOWave::SF_SIGNEDBYTE_8:
    return 8;
  case GOWave::SF_SIGNEDSHORT_12:
    return 12;
  case GOWave::SF_SIGNEDSHORT_16:
    return 16;
  case GOWave::SF_SIGNEDINT24_20:
    return 20;
  case GOWave::SF_SIGNEDINT24_24:
    return 24;
  case GOWave::SF_IEEE_FLOAT:
    return 32;
  default:
    assert(0 && "bad sample format enumeration");
    return 0;
  }
}

static inline unsigned wave_bytes_per_sample(GOWave::SAMPLE_FORMAT format) {
  return (wave_bits_per_sample(format) + 7) / 8;
}

static inline void loop_memcpy(
  unsigned char *dest,
  const unsigned char *source,
  unsigned source_len,
  unsigned count) {
  while (count > source_len) {
    memcpy(dest, source, source_len);
    dest += source_len;
    count -= source_len;
  }
  memcpy(dest, source, count);
}

void GOSoundAudioSection::GetMaxAmplitudeAndDerivative() {
  DecompressionCache cache;

  InitDecompressionCache(cache);
  m_MaxAmplitude = 0;
  m_MaxAbsAmplitude = 0;
  m_MaxAbsDerivative = 0;

  int f_p = 0; /* to avoid compiler warning */
  for (unsigned int i = 0; i < m_SampleCount; i++) {
    /* Get sum of amplitudes in channels */
    int f = 0;
    for (unsigned int j = 0; j < m_Channels; j++) {
      int val = GetSample(i, j, &cache);
      f += val;
      if (abs(val) > m_MaxAmplitude)
        m_MaxAmplitude = abs(val);
    }

    if (abs(f) > m_MaxAbsAmplitude)
      m_MaxAbsAmplitude = abs(f);

    if (i != 0) {
      /* Get v */
      int v = f - f_p;
      if (abs(v) > m_MaxAbsDerivative)
        m_MaxAbsDerivative = abs(v);
    }
    f_p = f;
  }
}

void GOSoundAudioSection::DoCrossfade(
  unsigned char *dest,
  unsigned dest_offset,
  const unsigned char *src,
  unsigned src_offset,
  unsigned channels,
  unsigned bits_per_sample,
  unsigned fade_length,
  unsigned loop_length,
  unsigned length) {
  for (; dest_offset < length; dest_offset += loop_length)
    for (unsigned pos = 0; pos < fade_length; pos++)
      for (unsigned j = 0; j < channels; j++) {
        float val1 = GetSampleData(
          pos + dest_offset, j, bits_per_sample, channels, dest);
        float val2
          = GetSampleData(pos + src_offset, j, bits_per_sample, channels, src);
        float factor = (cos(M_PI * (pos + 0.5) / fade_length) + 1.0) * 0.5;
        float result = val1 * factor + val2 * (1 - factor);
        SetSampleData(
          pos + dest_offset, j, bits_per_sample, channels, result, dest);
      }
}

void GOSoundAudioSection::Setup(
  const GOCacheObject *pObjectFor,
  const GOLoaderFilename *pLoaderFilename,
  const void *pcm_data,
  const GOWave::SAMPLE_FORMAT pcm_data_format,
  const unsigned pcm_data_channels,
  const unsigned pcm_data_sample_rate,
  const unsigned pcm_data_nb_samples,
  const std::vector<GOWaveLoop> *loop_points,
  GOBool3 waveTremulantStateFor,
  bool compress,
  unsigned loopCrossfadeLength,
  unsigned releaseCrossfadeLength) {
  if (pcm_data_channels < 1 || pcm_data_channels > 2)
    throw(wxString) _("< More than 2 channels in");

  m_BitsPerSample = wave_bits_per_sample(pcm_data_format);
  compress = (compress) && (m_BitsPerSample > 8);

  unsigned fade_len = loopCrossfadeLength * pcm_data_sample_rate / 1000;

  m_ReleaseCrossfadeLength = releaseCrossfadeLength;
  assert(pcm_data_nb_samples > 0);

  const unsigned bytes_per_sample = wave_bytes_per_sample(pcm_data_format);

  m_BytesPerSample = bytes_per_sample * pcm_data_channels;

  unsigned total_alloc_samples = pcm_data_nb_samples;
  /* Create a start segment */
  {
    audio_start_data_segment start_seg;
    start_seg.start_offset = 0;
    m_StartSegments.push_back(start_seg);
  }

  if ((loop_points) && (loop_points->size() >= 1)) {
    /* Setup the loops and find the amount of data we need to store in the
     * main block. */
    unsigned min_reqd_samples = 0;

    for (unsigned i = 0; i < loop_points->size(); i++) {
      audio_start_data_segment start_seg;
      audio_end_data_segment end_seg;
      const GOWaveLoop &loop = (*loop_points)[i];

      if (loop.m_EndPosition + 1 > min_reqd_samples)
        min_reqd_samples = loop.m_EndPosition + 1;

      start_seg.start_offset = loop.m_StartPosition;
      end_seg.end_offset = loop.m_EndPosition;
      end_seg.next_start_segment_index = i + 1;
      const unsigned loop_length
        = 1 + end_seg.end_offset - start_seg.start_offset;
      wxString loopError;

      if (fade_len > loop_length - 1)
        loopError = wxString::Format(
          _("The loop %u is ignored: it is too short for crossfade"), i + 1);
      else if (start_seg.start_offset < fade_len)
        loopError = wxString::Format(
          _("The loop %u is ignored: not enough samples for crossfade before "
            "it's start"),
          i + 1);

      if (loopError.IsEmpty()) {
        unsigned end_length;

        // calculate the fade segment size and offsets
        if (end_seg.end_offset - start_seg.start_offset > SHORT_LOOP_LENGTH) {
          end_seg.transition_offset
            = end_seg.end_offset - MAX_READAHEAD - fade_len + 1;
          end_seg.read_end = end_seg.end_offset - fade_len;
          end_length = 2 * MAX_READAHEAD + fade_len;
        } else {
          end_seg.transition_offset = start_seg.start_offset;
          end_seg.read_end = end_seg.end_offset;
          end_length = SHORT_LOOP_LENGTH + MAX_READAHEAD;
          if (
            end_length < MAX_READAHEAD
              + (SHORT_LOOP_LENGTH / loop_length) * SHORT_LOOP_LENGTH
              + fade_len)
            end_length = MAX_READAHEAD
              + (SHORT_LOOP_LENGTH / loop_length) * SHORT_LOOP_LENGTH
              + fade_len;
        }
        end_seg.end_size = end_length * m_BytesPerSample;

        // Allocate the fade segment
        end_seg.end_data
          = (unsigned char *)m_Pool.Alloc(end_seg.end_size, true);
        if (!end_seg.end_data)
          throw GOOutOfMemory();
        end_seg.end_ptr
          = end_seg.end_data - m_BytesPerSample * end_seg.transition_offset;

        const unsigned copy_len
          = 1 + end_seg.end_offset - end_seg.transition_offset;

        // Fill the fade seg with transition data, then with the loop start data
        memcpy(
          end_seg.end_data,
          ((const unsigned char *)pcm_data)
            + end_seg.transition_offset * m_BytesPerSample,
          copy_len * m_BytesPerSample);
        loop_memcpy(
          ((unsigned char *)end_seg.end_data) + copy_len * m_BytesPerSample,
          ((const unsigned char *)pcm_data)
            + loop.m_StartPosition * m_BytesPerSample,
          loop_length * m_BytesPerSample,
          (end_length - copy_len) * m_BytesPerSample);
        if (fade_len > 0)
          // TODO: reduce the number of parameters of DoCrossfade that the call
          // would be easy readable without additional comments
          DoCrossfade(
            end_seg.end_data,
            MAX_READAHEAD,
            (const unsigned char *)pcm_data,
            start_seg.start_offset - fade_len,
            pcm_data_channels,
            m_BitsPerSample,
            fade_len,
            loop_length,
            end_length);

        end_seg.end_loop_length = loop_length;
        end_seg.end_pos = end_length + end_seg.transition_offset;
        assert(end_length >= MAX_READAHEAD);

        m_StartSegments.push_back(start_seg);
        m_EndSegments.push_back(end_seg);
      } else
        wxLogWarning(GOCacheObject::generateMessage(
          pObjectFor, pLoaderFilename, loopError));
      if (!m_EndSegments.size())
        throw(wxString) _("No valid loops exist in the file");
    }

    /* There is no need to store any samples after the end of the last loop. */
    if (total_alloc_samples > min_reqd_samples)
      total_alloc_samples = min_reqd_samples;
  } else {
    /* Create a default end segment */
    audio_end_data_segment end_seg;
    end_seg.end_offset = pcm_data_nb_samples - 1;
    end_seg.read_end = end_seg.end_offset + 1;
    end_seg.next_start_segment_index = -1;
    unsigned end_length = 2 * MAX_READAHEAD;
    end_seg.end_size = end_length * m_BytesPerSample;
    end_seg.end_data = (unsigned char *)m_Pool.Alloc(end_seg.end_size, true);
    end_seg.transition_offset = limited_diff(end_seg.end_offset, MAX_READAHEAD);
    end_seg.end_loop_length = end_length * 2;
    end_seg.end_ptr
      = end_seg.end_data - m_BytesPerSample * end_seg.transition_offset;

    const unsigned copy_len
      = 1 + end_seg.end_offset - end_seg.transition_offset;

    if (!end_seg.end_data)
      throw GOOutOfMemory();

    memcpy(
      end_seg.end_data,
      ((const unsigned char *)pcm_data)
        + end_seg.transition_offset * m_BytesPerSample,
      copy_len * m_BytesPerSample);
    memset(
      ((unsigned char *)end_seg.end_data) + copy_len * m_BytesPerSample,
      0,
      (end_length - copy_len) * m_BytesPerSample);

    end_seg.end_pos = end_length + end_seg.transition_offset;
    assert(end_length >= MAX_READAHEAD);

    m_EndSegments.push_back(end_seg);
  }

  m_AllocSize = total_alloc_samples * m_BytesPerSample;
  m_Data = (unsigned char *)m_Pool.Alloc(m_AllocSize, !compress);
  if (m_Data == NULL)
    throw GOOutOfMemory();
  m_SampleRate = pcm_data_sample_rate;
  m_SampleCount = total_alloc_samples;
  m_SampleFracBits = m_BitsPerSample - 1;
  m_Channels = pcm_data_channels;
  m_IsCompressed = false;
  m_WaveTremulantStateFor = waveTremulantStateFor;

  /* Store the main data blob. */
  memcpy(m_Data, pcm_data, m_AllocSize);

  GetMaxAmplitudeAndDerivative();

  if (compress)
    Compress(m_BitsPerSample > 16);
}

void GOSoundAudioSection::Compress(bool format16) {
  unsigned char *data = (unsigned char *)m_Pool.Alloc(m_AllocSize, false);
  if (data == NULL)
    throw GOOutOfMemory();

  unsigned output_len = 0;
  DecompressionCache state;
  InitDecompressionCache(state);

  for (unsigned i = 0; i < m_SampleCount; i++) {
    state.position = i;
    state.ptr = (const unsigned char *)(intptr_t)output_len;
    for (unsigned j = 0; j < m_StartSegments.size(); j++) {
      if (m_StartSegments[j].start_offset == i) {
        m_StartSegments[j].cache = state;
      }
    }

    state.last[0] = state.prev[0];
    state.last[1] = state.prev[1];
    state.prev[0] = state.value[0];
    state.prev[1] = state.value[1];

    state.value[0] = GetSample(i, 0);
    if (m_Channels > 1)
      state.value[1] = GetSample(i, 1);

    for (unsigned j = 0; j < m_Channels; j++) {
      int val = state.value[j];
      int encode = val - (state.prev[j] + (state.prev[j] - state.last[j]) / 2);

      if (format16)
        AudioWriteCompressed16(data, output_len, encode);
      else
        AudioWriteCompressed8(data, output_len, encode);

      /* Early abort if the compressed data will be larger than the
       * uncompressed data. */
      if (output_len + 10 >= m_AllocSize) {
        m_Pool.Free(data);
        m_Data = (unsigned char *)m_Pool.MoveToPool(m_Data, m_AllocSize);
        if (m_Data == NULL)
          throw GOOutOfMemory();
        return;
      }
    }
  }

  m_Pool.Free(m_Data);
  m_Data = data;
  m_AllocSize = output_len;
  m_IsCompressed = true;

  m_Data = (unsigned char *)m_Pool.MoveToPool(m_Data, m_AllocSize);
  if (m_Data == NULL)
    throw GOOutOfMemory();
}

void GOSoundAudioSection::SetupStreamAlignment(
  const std::vector<const GOSoundAudioSection *> &joinables,
  unsigned start_index) {
  if (m_ReleaseAligner) {
    delete m_ReleaseAligner;
    m_ReleaseAligner = NULL;
  }
  if (!joinables.size())
    return;

  int max_amplitude = m_MaxAbsAmplitude;
  int max_derivative = m_MaxAbsDerivative;
  for (unsigned i = 0; i < joinables.size(); i++) {
    if (joinables[i]->m_MaxAbsAmplitude > max_amplitude)
      max_amplitude = joinables[i]->m_MaxAbsAmplitude;
    if (joinables[i]->m_MaxAbsDerivative > max_derivative)
      max_derivative = joinables[i]->m_MaxAbsDerivative;
  }
  m_ReleaseStartSegment = start_index;
  if (m_ReleaseStartSegment >= m_StartSegments.size())
    m_ReleaseStartSegment = 0;

  if ((max_derivative != 0) && (max_amplitude != 0)) {
    m_ReleaseAligner = new GOSoundReleaseAlignTable();
    m_ReleaseAligner->ComputeTable(
      *this,
      max_amplitude,
      max_derivative,
      m_SampleRate,
      m_StartSegments[m_ReleaseStartSegment].start_offset);
  }
}

unsigned GOSoundAudioSection::GetMargin(
  bool compressed, interpolation_type interpolation) {
  if (interpolation == GO_POLYPHASE_INTERPOLATION && !compressed)
    return POLYPHASE_READAHEAD;
  else if (compressed)
    return LINEAR_COMPRESSED_READAHEAD;
  else
    return LINEAR_READAHEAD;
}

void GOSoundAudioSection::InitStream(
  const struct resampler_coefs_s *resampler_coefs,
  audio_section_stream *stream,
  float sample_rate_adjustment) const {
  stream->audio_section = this;

  const audio_start_data_segment &start = m_StartSegments[0];
  const audio_end_data_segment &end = m_EndSegments[PickEndSegment(0)];
  assert(end.transition_offset >= start.start_offset);
  stream->resample_coefs = resampler_coefs;
  stream->ptr = stream->audio_section->m_Data;
  stream->transition_position = end.transition_offset;
  stream->end_seg = &end;
  stream->end_ptr = end.end_ptr;
  stream->increment_fraction
    = sample_rate_adjustment * m_SampleRate * UPSAMPLE_FACTOR;
  stream->position_index = start.start_offset;
  stream->position_fraction = 0;
  stream->decode_call = GetDecodeBlockFunction(
    m_Channels,
    m_BitsPerSample,
    m_IsCompressed,
    stream->resample_coefs->interpolation,
    false);
  stream->end_decode_call = GetDecodeBlockFunction(
    m_Channels,
    m_BitsPerSample,
    m_IsCompressed,
    stream->resample_coefs->interpolation,
    true);
  stream->margin
    = GetMargin(m_IsCompressed, stream->resample_coefs->interpolation);
  assert(stream->margin <= MAX_READAHEAD);
  stream->read_end = limited_diff(end.read_end, stream->margin);
  stream->end_pos = end.end_pos - stream->margin;
  stream->cache = start.cache;
  stream->cache.ptr
    = stream->audio_section->m_Data + (intptr_t)stream->cache.ptr;
}

void GOSoundAudioSection::InitAlignedStream(
  audio_section_stream *stream,
  const audio_section_stream *existing_stream) const {
  stream->audio_section = this;

  const audio_start_data_segment &start
    = m_StartSegments[m_ReleaseStartSegment];
  const audio_end_data_segment &end
    = m_EndSegments[PickEndSegment(m_ReleaseStartSegment)];
  assert(end.transition_offset >= start.start_offset);

  stream->ptr = stream->audio_section->m_Data;
  stream->transition_position = end.transition_offset;
  stream->end_seg = &end;
  stream->end_ptr = end.end_ptr;
  /* Translate increment in case of differing sample rates */
  stream->resample_coefs = existing_stream->resample_coefs;
  stream->increment_fraction = roundf(
    (((float)existing_stream->increment_fraction)
     / existing_stream->audio_section->GetSampleRate())
    * m_SampleRate);
  stream->position_index = start.start_offset;
  stream->position_fraction = existing_stream->position_fraction;
  stream->decode_call = GetDecodeBlockFunction(
    m_Channels,
    m_BitsPerSample,
    m_IsCompressed,
    stream->resample_coefs->interpolation,
    false);
  stream->end_decode_call = GetDecodeBlockFunction(
    m_Channels,
    m_BitsPerSample,
    m_IsCompressed,
    stream->resample_coefs->interpolation,
    true);
  stream->margin
    = GetMargin(m_IsCompressed, stream->resample_coefs->interpolation);
  assert(stream->margin <= MAX_READAHEAD);
  stream->read_end = limited_diff(end.read_end, stream->margin);
  stream->end_pos = end.end_pos - stream->margin;
  stream->cache = start.cache;
  stream->cache.ptr
    = stream->audio_section->m_Data + (intptr_t)stream->cache.ptr;
  if (!m_ReleaseAligner) {
    return;
  }
  m_ReleaseAligner->SetupRelease(*stream, *existing_stream);
}

unsigned GOSoundAudioSection::GetSampleRate() const { return m_SampleRate; }

void GOSoundAudioSection::GetHistory(
  const audio_section_stream *stream,
  int history[BLOCK_HISTORY][MAX_OUTPUT_CHANNELS]) {
  memset(
    history, 0, sizeof(history[0][0]) * BLOCK_HISTORY * MAX_OUTPUT_CHANNELS);
  if (stream->position_index >= stream->transition_position) {
    for (unsigned i = 0; i < BLOCK_HISTORY; i++)
      for (unsigned j = 0; j < stream->audio_section->m_Channels; j++)
        history[i][j] = GetSampleData(
          stream->position_index - stream->transition_position + i,
          j,
          stream->audio_section->m_BitsPerSample,
          stream->audio_section->m_Channels,
          stream->end_ptr);
  } else {
    if (!stream->audio_section->m_IsCompressed) {
      for (unsigned i = 0; i < BLOCK_HISTORY; i++)
        for (unsigned j = 0; j < stream->audio_section->m_Channels; j++)
          history[i][j] = GetSampleData(
            stream->position_index + i,
            j,
            stream->audio_section->m_BitsPerSample,
            stream->audio_section->m_Channels,
            stream->ptr);
    } else {
      DecompressionCache cache = stream->cache;
      for (unsigned i = 0; i < BLOCK_HISTORY; i++) {
        for (unsigned j = 0; j < stream->audio_section->m_Channels; j++)
          history[i][j] = cache.value[j];
        DecompressionStep(
          cache,
          stream->audio_section->m_Channels,
          stream->audio_section->m_BitsPerSample >= 20);
      }
    }
  }
}

GOSampleStatistic GOSoundAudioSection::GetStatistic() {
  GOSampleStatistic stat;

  size_t size = 0;
  for (unsigned i = 0; i < m_EndSegments.size(); i++)
    size += m_EndSegments[i].end_size;
  stat.SetEndSegmentSize(size);
  stat.SetMemorySize(size + m_AllocSize);
  stat.SetBitsPerSample(m_BitsPerSample, m_SampleCount, m_MaxAmplitude);

  return stat;
}
