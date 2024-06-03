/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundStream.h"

#include "GOSoundAudioSection.h"
#include "GOSoundReleaseAlignTable.h"

/* Block reading functions */

template <class SampleT, uint8_t nChannels>
class StreamPtrWindow
  : public GOSoundResample::PtrSampleVector<SampleT, int, nChannels> {
public:
  inline StreamPtrWindow(GOSoundStream &stream)
    : GOSoundResample::PtrSampleVector<SampleT, int, nChannels>(
      (SampleT *)stream.GetPtr()) {}
};

template <bool format16, uint8_t nChannels>
class StreamCacheWindow
  : public GOSoundResample::FloatingSampleVector<nChannels> {
private:
  DecompressionCache &r_cache;
  uint8_t m_ChannelN;
  enum { PREV, VALUE, ZERO } m_curr;

public:
  inline StreamCacheWindow(GOSoundStream &stream)
    : r_cache(stream.GetDecompressionCache()) {}

  inline void Seek(unsigned index, uint8_t channelN) {
    while (r_cache.position <= index + 1) {
      DecompressionStep(r_cache, nChannels, format16);
    }
    m_ChannelN = channelN;
    m_curr = PREV;
  }

  inline float NextSample() {
    int res;

    if (m_curr == PREV) {
      res = r_cache.prev[m_ChannelN];
      m_curr = VALUE;
    } else if (m_curr == VALUE) {
      res = r_cache.value[m_ChannelN];
      m_curr = ZERO;
    } else
      res = 0;
    return (float)res;
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
    /* TODO: Add support for polyphase compressed decoders. Fallback to
     * linear interpolation for now. */
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
  } else {
    if (
      interpolation == GOSoundResample::GO_POLYPHASE_INTERPOLATION
      && !compressed) {
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

static unsigned calculate_margin(
  bool compressed, GOSoundResample::InterpolationType interpolation) {
  return GOSoundResample::getVectorLength(interpolation) - 1
    + (unsigned)compressed;
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
  end_seg = &end;
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
  m_ReadAheadMarginLength
    = calculate_margin(pSection->IsCompressed(), interpolation);
  assert(m_ReadAheadMarginLength <= MAX_READAHEAD);
  end_pos = end.end_pos - m_ReadAheadMarginLength;
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
  end_seg = &end;
  end_ptr = end.end_ptr;
  /* Translate increment in case of differing sample rates */
  resample = existing_stream->resample;
  m_ResamplingPos.Init(
    pSection->GetSampleRate() / existing_stream->audio_section->GetSampleRate(),
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
  m_ReadAheadMarginLength
    = calculate_margin(pSection->IsCompressed(), interpolation);
  assert(m_ReadAheadMarginLength <= MAX_READAHEAD);
  end_pos = end.end_pos - m_ReadAheadMarginLength;
  cache = start.cache;
  cache.ptr = audio_section->GetData() + (intptr_t)cache.ptr;
}

bool GOSoundStream::ReadBlock(float *buffer, unsigned int n_blocks) {
  while (n_blocks > 0) {
    /*
      Calculate target number of samples available from the current loop
      min=1 - for infinite loop pervention we need to advance position after
      end_pos. 1 target sample is always available due read-ahead
     */
    unsigned len = std::clamp(
      m_ResamplingPos.AvailableTargetSamples(end_pos), 1U, n_blocks);

    if (m_ResamplingPos.GetIndex() >= transition_position) {
      // switch to or continue playing the end-segment
      assert(end_decode_call);

      /* Setup ptr and position required by the end-block */
      ptr = end_ptr;
      // we continue to use the same position as before because end_ptr is a
      // "virtual" pointer: end_data - transition_offset
      (this->*end_decode_call)(buffer, len);
      buffer += 2 * len;
      n_blocks -= len;

      // A new position after playing the end segment
      unsigned pos = m_ResamplingPos.GetIndex();

      if (pos >= end_pos) { // the loop has been fully played. Move to the start
                            // of the loop
        const GOSoundAudioSection::EndSegment *end = end_seg;

        if (end->next_start_segment_index < 0) { // it is not a loop
          for (unsigned i = 0; i < n_blocks * 2; i++)
            buffer[i] = 0;
          return 0;
        }

        const unsigned next_index = end->next_start_segment_index;
        const GOSoundAudioSection::StartSegment *next
          = &audio_section->GetStartSegment(next_index);

        // switch to the start of the loop
        ptr = audio_section->GetData();
        m_ResamplingPos.SetIndex(next->start_offset + (pos - end_pos));

        /* Find a suitable end segment */
        const unsigned next_end_segment_index
          = audio_section->PickEndSegment(next_index);
        const GOSoundAudioSection::EndSegment *next_end
          = &audio_section->GetEndSegment(next_end_segment_index);

        cache = next->cache;
        cache.ptr = audio_section->GetData() + (intptr_t)cache.ptr;
        assert(next_end->end_offset >= next->start_offset);
        transition_position = next_end->transition_offset;
        end_ptr = next_end->end_ptr;
        end_pos = next_end->end_pos - m_ReadAheadMarginLength;
        end_seg = next_end;
      }
    } else {
      assert(decode_call);
      (this->*decode_call)(buffer, len);
      buffer += 2 * len;
      n_blocks -= len;
    }
  }

  return 1;
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
