/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundStream.h"

#include <wx/log.h>

#include "GOSoundAudioSection.h"
#include "GOSoundReleaseAlignTable.h"

/* Block reading functions */

template <class SampleT, uint8_t nChannels>
class GOSoundStream::StreamPtrWindow
  : public GOSoundResample::PtrSampleVector<SampleT, int, nChannels> {
public:
  inline StreamPtrWindow(GOSoundStream &stream)
    : GOSoundResample::PtrSampleVector<SampleT, int, nChannels>(
      (SampleT *)stream.ptr) {}
};

template <bool format16, uint8_t nChannels>
class GOSoundStream::StreamCacheWindow
  : public GOSoundResample::FloatingSampleVector<nChannels> {
private:
  DecompressionCache &r_cache;
  uint8_t m_ChannelN;
  enum { PREV, VALUE, ZERO } m_curr;

public:
  inline StreamCacheWindow(GOSoundStream &stream) : r_cache(stream.cache) {}

  inline void Seek(unsigned index, uint8_t channelN) {
    while (r_cache.position <= index + 1) {
      DecompressionStep(r_cache, nChannels, format16);
    }
    m_ChannelN = channelN;
    m_curr = PREV;
  }

  inline int NextSample() {
    int res;

    if (m_curr == PREV) {
      res = r_cache.prev[m_ChannelN];
      m_curr = VALUE;
    } else if (m_curr == VALUE) {
      res = r_cache.value[m_ChannelN];
      m_curr = ZERO;
    } else
      res = 0;
    return res;
  }
};

template <bool format16, unsigned windowLen, uint8_t nChannels>
class GOSoundStream::StreamCacheReadAheadWindow
  : public GOSoundResample::PtrSampleVector<int, int, nChannels> {
private:
  static constexpr unsigned WINDOW_SAMPLES = nChannels * windowLen;
  static constexpr unsigned BUFFER_SAMPLES = WINDOW_SAMPLES * 2;

  DecompressionCache &r_cache;
  int *p_begin;
  int *p_end;

public:
  inline StreamCacheReadAheadWindow(GOSoundStream &stream)
    : GOSoundResample::PtrSampleVector<int, int, nChannels>(
      stream.m_ReadAheadBuffer),
      r_cache(stream.cache),
      p_begin(stream.m_ReadAheadBuffer),
      p_end(p_begin + BUFFER_SAMPLES) {}

  inline void Seek(unsigned index, uint8_t channelN) {
    unsigned readAheadIndexTo = index + windowLen;

    if (r_cache.position < readAheadIndexTo) {
      unsigned writePosition = std::max(r_cache.position, index);
      // pWrite1 points somewhere in the first half of the buffer
      int *pWrite1 = p_begin + nChannels * (writePosition % windowLen);
      // pWrite2 points somewhere in the second half of the buffer
      int *pWrite2 = pWrite1 + WINDOW_SAMPLES;

      while (r_cache.position < readAheadIndexTo) {
        DecompressionStep(r_cache, nChannels, format16);

        /* fill the read ahead buffer. If r_cache.position > index we assume
          that the previous samples already present */
        if (r_cache.position >= index) {
          const int *pRead = r_cache.value;

          for (uint8_t i = nChannels; i > 0; i--)
            *(pWrite1++) = *(pWrite2++) = *(pRead++);
          /* because stream.m_ReadAheadBuffer is a ring buffer of a double
           * windowLen, pWrite2 may move out of the buffer. Reset it to the
           * buffer begin, so pWrite pointers would swap */
          if (pWrite2 >= p_end)
            pWrite2 -= BUFFER_SAMPLES;
        }
      }
    }
    /* Now the ring buffer contains windowLen continous samples in the positions
     * from index % windowLen to index % windowLen + windowLen. So use Seek of
     * the parent class */
    GOSoundResample::PtrSampleVector<int, int, nChannels>::Seek(
      index % windowLen, channelN);
  }
};

/* The block decode functions should provide whatever the normal resolution of
 * the audio is. The fade engine should ensure that this data is always brought
 * into the correct range. */

template <class ResamplerT, class WindowT>
void GOSoundStream::DecodeBlock(float *pOut, unsigned nOutSamples) {
  ResamplerT resampler(*resample);
  WindowT w(*this);

  resampler.template ResampleBlock<WindowT, 2>(
    m_ResamplingPos, w, pOut, nOutSamples);
}

GOSoundStream::DecodeBlockFunction GOSoundStream::getDecodeBlockFunction(
  uint8_t channels,
  uint8_t bits_per_sample,
  bool compressed,
  GOSoundResample::InterpolationType interpolation,
  bool is_end) {
  if (compressed && !is_end) {
    if (interpolation == GOSoundResample::GO_POLYPHASE_INTERPOLATION) {
      if (channels == 1) {
        if (bits_per_sample >= 20)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::PolyphaseResampler,
            StreamCacheReadAheadWindow<
              true,
              GOSoundResample::PolyphaseResampler::VECTOR_LENGTH,
              1>>;

        assert(bits_per_sample >= 12);
        return &GOSoundStream::DecodeBlock<
          GOSoundResample::PolyphaseResampler,
          StreamCacheReadAheadWindow<
            false,
            GOSoundResample::PolyphaseResampler::VECTOR_LENGTH,
            1>>;
      } else if (channels == 2) {
        if (bits_per_sample >= 20)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::PolyphaseResampler,
            StreamCacheReadAheadWindow<
              true,
              GOSoundResample::PolyphaseResampler::VECTOR_LENGTH,
              2>>;

        assert(bits_per_sample >= 12);
        return &GOSoundStream::DecodeBlock<
          GOSoundResample::PolyphaseResampler,
          StreamCacheReadAheadWindow<
            false,
            GOSoundResample::PolyphaseResampler::VECTOR_LENGTH,
            2>>;
      }
    } else {
      if (channels == 1) {
        if (bits_per_sample >= 20)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::LinearResampler,
            StreamCacheWindow<true, 1>>;

        assert(bits_per_sample >= 12);
        return &GOSoundStream::DecodeBlock<
          GOSoundResample::LinearResampler,
          StreamCacheWindow<false, 1>>;
      } else if (channels == 2) {
        if (bits_per_sample >= 20)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::LinearResampler,
            StreamCacheWindow<true, 2>>;

        assert(bits_per_sample >= 12);
        return &GOSoundStream::DecodeBlock<
          GOSoundResample::LinearResampler,
          StreamCacheWindow<false, 2>>;
      }
    }
  } else {
    if (interpolation == GOSoundResample::GO_POLYPHASE_INTERPOLATION) {
      if (channels == 1) {
        if (bits_per_sample <= 8)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::PolyphaseResampler,
            StreamPtrWindow<GOInt8, 1>>;
        if (bits_per_sample <= 16)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::PolyphaseResampler,
            StreamPtrWindow<GOInt16, 1>>;
        if (bits_per_sample <= 24)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::PolyphaseResampler,
            StreamPtrWindow<GOInt24, 1>>;
      } else if (channels == 2) {
        if (bits_per_sample <= 8)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::PolyphaseResampler,
            StreamPtrWindow<GOInt8, 2>>;
        if (bits_per_sample <= 16)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::PolyphaseResampler,
            StreamPtrWindow<GOInt16, 2>>;
        if (bits_per_sample <= 24)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::PolyphaseResampler,
            StreamPtrWindow<GOInt24, 2>>;
      }
    } else {
      if (channels == 1) {
        if (bits_per_sample <= 8)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::LinearResampler,
            StreamPtrWindow<GOInt8, 1>>;
        if (bits_per_sample <= 16)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::LinearResampler,
            StreamPtrWindow<GOInt16, 1>>;
        if (bits_per_sample <= 24)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::LinearResampler,
            StreamPtrWindow<GOInt24, 1>>;
      } else if (channels == 2) {
        if (bits_per_sample <= 8)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::LinearResampler,
            StreamPtrWindow<GOInt8, 2>>;
        if (bits_per_sample <= 16)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::LinearResampler,
            StreamPtrWindow<GOInt16, 2>>;
        if (bits_per_sample <= 24)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::LinearResampler,
            StreamPtrWindow<GOInt24, 2>>;
      }
    }
  }

  assert(0 && "unsupported decoder configuration");
  return NULL;
}

void GOSoundStream::InitStream(
  const GOSoundResample *pResample,
  const GOSoundAudioSection *pSection,
  GOSoundResample::InterpolationType interpolation,
  float sample_rate_adjustment) {
  audio_section = pSection;

  const GOSoundAudioSection::StartSegment &start = pSection->GetStartSegment(0);
  const GOSoundAudioSection::EndSegment &end
    = pSection->GetEndSegment(pSection->PickEndSegment(0));

  assert(end.transition_offset >= start.start_offset);
  resample = pResample;
  ptr = audio_section->GetData();
  transition_position = end.transition_offset;
  m_NextStartSegmentIndex = end.next_start_segment_index;
  end_ptr = end.end_ptr;
  m_ResamplingPos.Init(
    sample_rate_adjustment * pSection->GetSampleRate(), start.start_offset);
  decode_call = getDecodeBlockFunction(
    pSection->GetChannels(),
    pSection->GetBitsPerSample(),
    pSection->IsCompressed(),
    interpolation,
    false);
  end_decode_call = getDecodeBlockFunction(
    pSection->GetChannels(),
    pSection->GetBitsPerSample(),
    pSection->IsCompressed(),
    interpolation,
    true);
  end_pos = end.end_pos;
  cache = start.cache;
  cache.ptr = audio_section->GetData() + (intptr_t)cache.ptr;
}

void GOSoundStream::InitAlignedStream(
  const GOSoundAudioSection *pSection,
  GOSoundResample::InterpolationType interpolation,
  const GOSoundStream *existing_stream) {
  const unsigned releaseStartSegment = pSection->GetReleaseStartSegment();
  const GOSoundAudioSection::StartSegment &start
    = pSection->GetStartSegment(releaseStartSegment);
  const GOSoundAudioSection::EndSegment &end
    = pSection->GetEndSegment(pSection->PickEndSegment(releaseStartSegment));
  GOSoundReleaseAlignTable *releaseAligner = pSection->GetReleaseAligner();
  unsigned startIndex;

  if (releaseAligner) {
    int history[BLOCK_HISTORY][MAX_OUTPUT_CHANNELS];

    existing_stream->GetHistory(history);
    startIndex = releaseAligner->GetPositionFor(history);
  } else
    startIndex = start.start_offset;

  assert(end.transition_offset >= start.start_offset);

  audio_section = pSection;
  ptr = audio_section->GetData();
  transition_position = end.transition_offset;
  m_NextStartSegmentIndex = end.next_start_segment_index;
  end_ptr = end.end_ptr;
  /* Translate increment in case of differing sample rates */
  resample = existing_stream->resample;
  m_ResamplingPos.Init(
    (float)pSection->GetSampleRate()
      / existing_stream->audio_section->GetSampleRate(),
    startIndex,
    &existing_stream->m_ResamplingPos);
  decode_call = getDecodeBlockFunction(
    pSection->GetChannels(),
    pSection->GetBitsPerSample(),
    pSection->IsCompressed(),
    interpolation,
    false);
  end_decode_call = getDecodeBlockFunction(
    pSection->GetChannels(),
    pSection->GetBitsPerSample(),
    pSection->IsCompressed(),
    interpolation,
    true);
  end_pos = end.end_pos;
  cache = start.cache;
  cache.ptr = audio_section->GetData() + (intptr_t)cache.ptr;
}

bool GOSoundStream::ReadBlock(float *buffer, unsigned int n_blocks) {
  bool res = true;

  while (n_blocks > 0) {
    unsigned pos = m_ResamplingPos.GetIndex();

    if (pos < end_pos) { // the loop has not yet fully played
      // whether we are playing the start or the end segment
      bool isToPlayMain = pos < transition_position;
      unsigned finishPos = isToPlayMain ? transition_position : end_pos;
      unsigned targetSamples
        = std::min(m_ResamplingPos.AvailableTargetSamples(finishPos), n_blocks);

      assert(targetSamples > 0);
      if (isToPlayMain) {
        assert(decode_call);
        (this->*decode_call)(buffer, targetSamples);
      } else {
        assert(end_decode_call);

        // Setup ptr and position required by the end-block
        ptr = end_ptr;
        // we continue to use the same position as before because end_ptr is a
        // "virtual" pointer: end_data - transition_offset
        (this->*end_decode_call)(buffer, targetSamples);
      }
      buffer += 2 * targetSamples;
      n_blocks -= targetSamples;
      assert(!n_blocks || m_ResamplingPos.GetIndex() >= finishPos);
    } else if (m_NextStartSegmentIndex >= 0) {
      // switch to the start of the loop
      const GOSoundAudioSection::StartSegment *next
        = &audio_section->GetStartSegment(m_NextStartSegmentIndex);
      unsigned newPos = next->start_offset + (pos - end_pos);

      // switch to the start of the loop
      ptr = audio_section->GetData();

      /* Find a suitable end segment */
      const unsigned next_end_segment_index
        = audio_section->PickEndSegment(m_NextStartSegmentIndex);
      const GOSoundAudioSection::EndSegment *next_end
        = &audio_section->GetEndSegment(next_end_segment_index);

      assert(next_end->end_pos >= next->start_offset);

      cache = next->cache;
      cache.ptr = audio_section->GetData() + (intptr_t)cache.ptr;
      transition_position = next_end->transition_offset;
      end_pos = next_end->end_pos;
      end_ptr = next_end->end_ptr;

      m_ResamplingPos.SetIndex(newPos);
      if (newPos < end_pos) // valid loop
        m_NextStartSegmentIndex = next_end->next_start_segment_index;
      else { // invalid loop. Using it might cause infinite iterations here
        wxLogError(
          "GOSoundStream::ReadBlock: Breaking invalid loop: start_offset=%d, "
          "end_pos=%d, new_pos=%d",
          next_end->end_pos,
          next->start_offset,
          newPos);
        // force exit at the next iteration
        m_NextStartSegmentIndex = -1;
      }
    } else { // no loop available
      // fill the buffer with zeros
      float *p = buffer;

      for (unsigned i = n_blocks * 2; i > 0; i--)
        *(p++) = 0.0f;
      n_blocks = 0;
      res = false;
    }
  }
  return res;
}

void GOSoundStream::GetHistory(
  int history[BLOCK_HISTORY][MAX_OUTPUT_CHANNELS]) const {
  uint8_t nChannels = audio_section->GetChannels();
  unsigned pos = m_ResamplingPos.GetIndex();

  memset(
    history, 0, sizeof(history[0][0]) * BLOCK_HISTORY * MAX_OUTPUT_CHANNELS);
  if (pos >= transition_position)
    for (unsigned i = 0; i < BLOCK_HISTORY; i++)
      for (uint8_t j = 0; j < nChannels; j++)
        history[i][j] = audio_section->GetSampleData(
          end_ptr, pos - transition_position + i, j);
  else if (!audio_section->IsCompressed())
    for (unsigned i = 0; i < BLOCK_HISTORY; i++)
      for (uint8_t j = 0; j < nChannels; j++)
        history[i][j] = audio_section->GetSampleData(ptr, pos + i, j);
  else {
    DecompressionCache tmpCache = cache;

    for (unsigned i = 0; i < BLOCK_HISTORY; i++) {
      for (uint8_t j = 0; j < nChannels; j++)
        history[i][j] = tmpCache.value[j];
      DecompressionStep(
        tmpCache, nChannels, audio_section->GetBitsPerSample() >= 20);
    }
  }
}
