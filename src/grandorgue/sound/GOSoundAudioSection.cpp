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

// maximal readahead is necessary for polyphase resampling
static constexpr unsigned MAX_READAHEAD = GOSoundResample::POLYPHASE_POINTS;
static constexpr unsigned DEFAULT_END_SEG_LENGTH = MAX_READAHEAD * 2;

const unsigned GOSoundAudioSection::getMaxReadAhead() { return MAX_READAHEAD; }

GOSoundAudioSection::GOSoundAudioSection(GOMemoryPool &pool)
  : m_data(NULL),
    m_ReleaseAligner(NULL),
    m_ReleaseStartSegment(0),
    m_Pool(pool) {
  ClearData();
}

void GOSoundAudioSection::ClearData() {
  m_AllocSize = 0;
  m_SampleCount = 0;
  m_SampleRate = 0;
  m_BitsPerSample = 0;
  m_BytesPerSample = 0;
  m_WaveTremulantStateFor = BOOL3_DEFAULT;
  m_IsCompressed = false;
  m_channels = 0;
  if (m_data) {
    m_Pool.Free(m_data);
    m_data = NULL;
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
  if (!cache.Read(&m_channels, sizeof(m_channels)))
    return false;
  if (!cache.Read(&m_SampleFracBits, sizeof(m_SampleFracBits)))
    return false;
  if (!cache.Read(&m_MaxAmplitude, sizeof(m_MaxAmplitude)))
    return false;
  if (!cache.Read(&m_ReleaseStartSegment, sizeof(m_ReleaseStartSegment)))
    return false;
  if (!cache.Read(&m_ReleaseCrossfadeLength, sizeof(m_ReleaseCrossfadeLength)))
    return false;
  m_data = (unsigned char *)cache.ReadBlock(m_AllocSize);
  if (!m_data)
    return false;

  unsigned temp;
  if (!cache.Read(&temp, sizeof(temp)))
    return false;
  for (unsigned i = 0; i < temp; i++) {
    StartSegment s;
    if (!cache.Read(&s, sizeof(s)))
      return false;
    m_StartSegments.push_back(s);
  }

  if (!cache.Read(&temp, sizeof(temp)))
    return false;
  for (unsigned i = 0; i < temp; i++) {
    EndSegment s;

    if (!cache.Read(
          static_cast<EndSegmentDescription *>(&s),
          sizeof(EndSegmentDescription)))
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
  if (!cache.Write(&m_channels, sizeof(m_channels)))
    return false;
  if (!cache.Write(&m_SampleFracBits, sizeof(m_SampleFracBits)))
    return false;
  if (!cache.Write(&m_MaxAmplitude, sizeof(m_MaxAmplitude)))
    return false;
  if (!cache.Write(&m_ReleaseStartSegment, sizeof(m_ReleaseStartSegment)))
    return false;
  if (!cache.Write(&m_ReleaseCrossfadeLength, sizeof(m_ReleaseCrossfadeLength)))
    return false;
  if (!cache.WriteBlock(m_data, m_AllocSize))
    return false;

  unsigned temp;

  temp = m_StartSegments.size();
  if (!cache.Write(&temp, sizeof(unsigned)))
    return false;
  for (unsigned i = 0; i < temp; i++) {
    const StartSegment *s = &m_StartSegments[i];
    if (!cache.Write(s, sizeof(*s)))
      return false;
  }

  temp = m_EndSegments.size();
  if (!cache.Write(&temp, sizeof(temp)))
    return false;
  for (unsigned i = 0; i < temp; i++) {
    const EndSegment &s = m_EndSegments[i];

    if (!cache.Write(
          static_cast<const EndSegmentDescription *>(&s),
          sizeof(EndSegmentDescription)))
      return false;
    if (!cache.WriteBlock(s.end_data, s.end_size))
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

unsigned GOSoundAudioSection::PickEndSegment(
  unsigned start_segment_index) const {
  const unsigned x = abs(rand());
  for (unsigned i = 0; i < m_EndSegments.size(); i++) {
    const unsigned idx = (i + x) % m_EndSegments.size();
    const EndSegment *end = &m_EndSegments[idx];
    if (
      end->transition_offset
      >= m_StartSegments[start_segment_index].start_offset)
      return idx;
  }
  assert(0 && "should always find suitable end segment");
  return 0;
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
    for (unsigned int j = 0; j < m_channels; j++) {
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
  unsigned fade_length,
  unsigned loop_length,
  unsigned length) {
  for (; dest_offset < length; dest_offset += loop_length)
    for (unsigned pos = 0; pos < fade_length; pos++) {
      float factor = (cos(M_PI * (pos + 0.5) / fade_length) + 1.0) * 0.5;

      for (uint8_t j = 0; j < m_channels; j++) {
        float val1 = GetSampleData(dest, pos + dest_offset, j);
        float val2 = GetSampleData(src, pos + src_offset, j);
        float result = val1 * factor + val2 * (1 - factor);

        SetSampleData(dest, pos + dest_offset, j, (int)result);
      }
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

  m_channels = pcm_data_channels;
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
    StartSegment start_seg;
    start_seg.start_offset = 0;
    m_StartSegments.push_back(start_seg);
  }

  if ((loop_points) && (loop_points->size() >= 1)) {
    /* Setup the loops and find the amount of data we need to store in the
     * main block. */
    unsigned min_reqd_samples = 0;

    for (unsigned i = 0; i < loop_points->size(); i++) {
      StartSegment start_seg;
      EndSegment end_seg;
      const GOWaveLoop &loop = (*loop_points)[i];

      if (loop.m_EndPosition + 1 > min_reqd_samples)
        min_reqd_samples = loop.m_EndPosition + 1;

      start_seg.start_offset = loop.m_StartPosition;
      end_seg.end_pos = loop.m_EndPosition;
      end_seg.next_start_segment_index = i + 1;
      const unsigned loop_length = end_seg.end_pos - start_seg.start_offset;
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
        if (loop_length >= SHORT_LOOP_LENGTH) {
          end_seg.transition_offset
            = end_seg.end_pos - MAX_READAHEAD - fade_len;
          end_length = DEFAULT_END_SEG_LENGTH + fade_len;
        } else {
          end_seg.transition_offset = start_seg.start_offset;
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

        // make a virtual pointer for reading the end segment with the same
        // offset as the main data
        end_seg.end_ptr
          = end_seg.end_data - m_BytesPerSample * end_seg.transition_offset;

        const unsigned nBytesToCopy
          = (end_seg.end_pos - end_seg.transition_offset) * m_BytesPerSample;

        // Fill the fade seg with transition data, then with the loop start data
        memcpy(
          end_seg.end_data,
          ((const unsigned char *)pcm_data)
            + end_seg.transition_offset * m_BytesPerSample,
          nBytesToCopy);
        loop_memcpy(
          ((unsigned char *)end_seg.end_data) + nBytesToCopy,
          ((const unsigned char *)pcm_data)
            + loop.m_StartPosition * m_BytesPerSample,
          loop_length * m_BytesPerSample,
          end_seg.end_size - nBytesToCopy);
        if (fade_len > 0)
          // TODO: reduce the number of parameters of DoCrossfade that the call
          // would be easy readable without additional comments
          DoCrossfade(
            end_seg.end_data,
            MAX_READAHEAD,
            (const unsigned char *)pcm_data,
            start_seg.start_offset - fade_len,
            fade_len,
            loop_length,
            end_length);

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
    EndSegment end_seg;

    end_seg.end_pos = pcm_data_nb_samples;
    end_seg.next_start_segment_index = -1;
    end_seg.end_size = m_BytesPerSample * DEFAULT_END_SEG_LENGTH;
    end_seg.end_data = (unsigned char *)m_Pool.Alloc(end_seg.end_size, true);

    if (!end_seg.end_data)
      throw GOOutOfMemory();

    end_seg.transition_offset = limitedDiff(end_seg.end_pos, MAX_READAHEAD);
    end_seg.end_ptr
      = end_seg.end_data - m_BytesPerSample * end_seg.transition_offset;

    const unsigned nBytesToCopy
      = (end_seg.end_pos - end_seg.transition_offset) * m_BytesPerSample;

    memcpy(
      end_seg.end_data,
      ((const unsigned char *)pcm_data)
        + m_BytesPerSample * end_seg.transition_offset,
      nBytesToCopy);
    memset(
      ((unsigned char *)end_seg.end_data) + nBytesToCopy,
      0,
      end_seg.end_size - nBytesToCopy);

    m_EndSegments.push_back(end_seg);
  }

  m_AllocSize = total_alloc_samples * m_BytesPerSample;
  m_data = (unsigned char *)m_Pool.Alloc(m_AllocSize, !compress);
  if (m_data == NULL)
    throw GOOutOfMemory();
  m_SampleRate = pcm_data_sample_rate;
  m_SampleCount = total_alloc_samples;
  m_SampleFracBits = m_BitsPerSample - 1;
  m_IsCompressed = false;
  m_WaveTremulantStateFor = waveTremulantStateFor;

  /* Store the main data blob. */
  memcpy(m_data, pcm_data, m_AllocSize);

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
    if (m_channels > 1)
      state.value[1] = GetSample(i, 1);

    for (unsigned j = 0; j < m_channels; j++) {
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
        m_data = (unsigned char *)m_Pool.MoveToPool(m_data, m_AllocSize);
        if (m_data == NULL)
          throw GOOutOfMemory();
        return;
      }
    }
  }

  m_Pool.Free(m_data);
  m_data = data;
  m_AllocSize = output_len;
  m_IsCompressed = true;

  m_data = (unsigned char *)m_Pool.MoveToPool(m_data, m_AllocSize);
  if (m_data == NULL)
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
