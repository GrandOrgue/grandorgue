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

/* The block decode functions should provide whatever the normal resolution of
 * the audio is. The fade engine should ensure that this data is always brought
 * into the correct range. */

template <class ResamplerT, class WindowT>
inline void GOSoundStream::DecodeBlock(float *pOut, unsigned nOutSamples) {
  WindowT w(*this);

  resample->ResampleBlock<ResamplerT, WindowT, 2>(
    resamplingPos, w, pOut, nOutSamples);
}

GOSoundStream::DecodeBlockFunction GOSoundStream::getDecodeBlockFunction(
  unsigned channels,
  unsigned bits_per_sample,
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
  if (
    interpolation == GOSoundResample::GO_POLYPHASE_INTERPOLATION && !compressed)
    return POLYPHASE_READAHEAD;
  else if (compressed)
    return LINEAR_COMPRESSED_READAHEAD;
  else
    return LINEAR_READAHEAD;
}

void GOSoundStream::InitStream(
  const GOSoundAudioSection *pSection,
  const struct GOSoundResample *resampler_coefs,
  float sample_rate_adjustment) {
  audio_section = pSection;

  const GOSoundAudioSection::StartSegment &start = pSection->GetStartSegment(0);
  const GOSoundAudioSection::EndSegment &end
    = pSection->GetEndSegment(pSection->PickEndSegment(0));

  assert(end.transition_offset >= start.start_offset);
  resample = resampler_coefs;
  ptr = audio_section->GetData();
  transition_position = end.transition_offset;
  end_seg = &end;
  end_ptr = end.end_ptr;
  resamplingPos.Init(
    sample_rate_adjustment * pSection->GetSampleRate(), start.start_offset);
  decode_call = getDecodeBlockFunction(
    pSection->GetChannels(),
    pSection->GetBitsPerSample(),
    pSection->IsCompressed(),
    resample->m_interpolation,
    false);
  end_decode_call = getDecodeBlockFunction(
    pSection->GetChannels(),
    pSection->GetBitsPerSample(),
    pSection->IsCompressed(),
    resample->m_interpolation,
    true);
  margin
    = calculate_margin(pSection->IsCompressed(), resample->m_interpolation);
  assert(margin <= MAX_READAHEAD);
  read_end = GOSoundAudioSection::limitedDiff(end.read_end, margin);
  end_pos = end.end_pos - margin;
  cache = start.cache;
  cache.ptr = audio_section->GetData() + (intptr_t)cache.ptr;
}

void GOSoundStream::InitAlignedStream(
  const GOSoundAudioSection *pSection, const GOSoundStream *existing_stream) {
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
  resamplingPos.Init(
    pSection->GetSampleRate() / existing_stream->audio_section->GetSampleRate(),
    startIndex,
    &existing_stream->resamplingPos);
  decode_call = getDecodeBlockFunction(
    pSection->GetChannels(),
    pSection->GetBitsPerSample(),
    pSection->IsCompressed(),
    resample->m_interpolation,
    false);
  end_decode_call = getDecodeBlockFunction(
    pSection->GetChannels(),
    pSection->GetBitsPerSample(),
    pSection->IsCompressed(),
    resample->m_interpolation,
    true);
  margin
    = calculate_margin(pSection->IsCompressed(), resample->m_interpolation);
  assert(margin <= MAX_READAHEAD);
  read_end = GOSoundAudioSection::limitedDiff(end.read_end, margin);
  end_pos = end.end_pos - margin;
  cache = start.cache;
  cache.ptr = audio_section->GetData() + (intptr_t)cache.ptr;
}

bool GOSoundStream::ReadBlock(float *buffer, unsigned int n_blocks) {
  while (n_blocks > 0) {
    unsigned pos = resamplingPos.GetIndex();
    // Calculate target number of samples
    unsigned len
      = roundf((end_pos - pos) / resamplingPos.GetResamplingFactor());

    if (len == 0)
      len = 1;
    len = std::min(len, n_blocks);

    if (pos >= transition_position) {
      assert(end_decode_call);

      /* Setup ptr and position required by the end-block */
      ptr = end_ptr;
      // May be we also need to set the position to 0?
      (this->*end_decode_call)(buffer, len);
      buffer += 2 * len;
      n_blocks -= len;

      if (pos >= end_pos) {
        const GOSoundAudioSection::EndSegment *end = end_seg;

        if (end->next_start_segment_index < 0) {
          for (unsigned i = 0; i < n_blocks * 2; i++)
            buffer[i] = 0;
          return 0;
        }

        pos -= end->end_offset + 1;
        while (pos >= end->end_loop_length)
          pos -= end->end_loop_length;

        const unsigned next_index = end->next_start_segment_index;
        const GOSoundAudioSection::StartSegment *next
          = &audio_section->GetStartSegment(next_index);

        /* Find a suitable end segment */
        const unsigned next_end_segment_index
          = audio_section->PickEndSegment(next_index);
        const GOSoundAudioSection::EndSegment *next_end
          = &audio_section->GetEndSegment(next_end_segment_index);

        ptr = audio_section->GetData();
        resamplingPos.SetIndex(next->start_offset + pos);
        cache = next->cache;
        cache.ptr = audio_section->GetData() + (intptr_t)cache.ptr;
        assert(next_end->end_offset >= next->start_offset);
        transition_position = next_end->transition_offset;
        end_ptr = next_end->end_ptr;
        read_end = GOSoundAudioSection::limitedDiff(next_end->read_end, margin);
        end_pos = next_end->end_pos - margin;
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
  unsigned pos = resamplingPos.GetIndex();

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
