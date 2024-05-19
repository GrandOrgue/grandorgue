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

template <class T>
void GOSoundStream::MonoUncompressedLinear(
  float *output, unsigned int n_blocks) {
  // copy the sample buffer
  T *input = (T *)(ptr);
  for (unsigned int i = 0; i < n_blocks;
       i++, position_fraction += increment_fraction, output += 2) {
    NormalisePosition();
    unsigned pos = position_index;
    output[0] = input[pos] * resample_coefs->linear[position_fraction][1]
      + input[pos + 1] * resample_coefs->linear[position_fraction][0];
    output[1] = output[0];
  }

  NormalisePosition();
}

template <class T>
void GOSoundStream::StereoUncompressedLinear(
  float *output, unsigned int n_blocks) {
  typedef T stereoSample[][2];

  // "borrow" the output buffer to compute release alignment info
  stereoSample &input = (stereoSample &)*(T *)(ptr);

  // copy the sample buffer
  for (unsigned int i = 0; i < n_blocks;
       position_fraction += increment_fraction, output += 2, i++) {
    NormalisePosition();
    unsigned pos = position_index;
    output[0] = input[pos][0] * resample_coefs->linear[position_fraction][1]
      + input[pos + 1][0] * resample_coefs->linear[position_fraction][0];
    output[1] = input[pos][1] * resample_coefs->linear[position_fraction][1]
      + input[pos + 1][1] * resample_coefs->linear[position_fraction][0];
  }

  NormalisePosition();
}

template <class T>
void GOSoundStream::MonoUncompressedPolyphase(
  float *output, unsigned int n_blocks) {
  // copy the sample buffer
  T *input = ((T *)ptr);
  const float *coef = resample_coefs->coefs;
  for (unsigned i = 0; i < n_blocks;
       ++i, output += 2, position_fraction += increment_fraction) {
    NormalisePosition();
    float out1 = 0.0f;
    float out2 = 0.0f;
    float out3 = 0.0f;
    float out4 = 0.0f;
    const float *coef_set = &coef[position_fraction << SUBFILTER_BITS];
    T *in_set = &input[position_index];
    for (unsigned j = 0; j < SUBFILTER_TAPS; j += 4) {
      out1 += in_set[j] * coef_set[j];
      out2 += in_set[j + 1] * coef_set[j + 1];
      out3 += in_set[j + 2] * coef_set[j + 2];
      out4 += in_set[j + 3] * coef_set[j + 3];
    }
    output[0] = out1 + out2 + out3 + out4;
    output[1] = output[0];
  }

  NormalisePosition();
}

template <class T>
void GOSoundStream::StereoUncompressedPolyphase(
  float *output, unsigned int n_blocks) {
  typedef T stereoSample[][2];

  // copy the sample buffer
  stereoSample &input = (stereoSample &)*(T *)(ptr);
  const float *coef = resample_coefs->coefs;
  for (unsigned i = 0; i < n_blocks;
       ++i, output += 2, position_fraction += increment_fraction) {
    NormalisePosition();
    float out1 = 0.0f;
    float out2 = 0.0f;
    float out3 = 0.0f;
    float out4 = 0.0f;
    const float *coef_set = &coef[position_fraction << SUBFILTER_BITS];
    T *in_set = (T *)&input[position_index][0];
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
  NormalisePosition();
}

template <bool format16>
void GOSoundStream::MonoCompressedLinear(float *output, unsigned int n_blocks) {
  for (unsigned int i = 0; i < n_blocks;
       i++, position_fraction += increment_fraction, output += 2) {
    NormalisePosition();

    while (cache.position <= position_index + 1) {
      DecompressionStep(cache, 1, format16);
    }

    output[0] = cache.prev[0] * resample_coefs->linear[position_fraction][1]
      + cache.value[0] * resample_coefs->linear[position_fraction][0];
    output[1] = output[0];
  }
  NormalisePosition();
}

template <bool format16>
void GOSoundStream::StereoCompressedLinear(
  float *output, unsigned int n_blocks) {
  // copy the sample buffer
  for (unsigned int i = 0; i < n_blocks;
       position_fraction += increment_fraction, output += 2, i++) {
    NormalisePosition();

    while (cache.position <= position_index + 1) {
      DecompressionStep(cache, 2, format16);
    }

    output[0] = cache.prev[0] * resample_coefs->linear[position_fraction][1]
      + cache.value[0] * resample_coefs->linear[position_fraction][0];
    output[1] = cache.prev[1] * resample_coefs->linear[position_fraction][1]
      + cache.value[1] * resample_coefs->linear[position_fraction][0];
  }
  NormalisePosition();
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
        return &GOSoundStream::MonoCompressedLinear<true>;

      assert(bits_per_sample >= 12);
      return &GOSoundStream::MonoCompressedLinear<false>;
    } else if (channels == 2) {
      if (bits_per_sample >= 20)
        return &GOSoundStream::StereoCompressedLinear<true>;

      assert(bits_per_sample >= 12);
      return &GOSoundStream::StereoCompressedLinear<false>;
    }
  } else {
    if (
      interpolation == GOSoundResample::GO_POLYPHASE_INTERPOLATION
      && !compressed) {
      if (channels == 1) {
        if (bits_per_sample <= 8)
          return &GOSoundStream::MonoUncompressedPolyphase<GOInt8>;
        if (bits_per_sample <= 16)
          return &GOSoundStream::MonoUncompressedPolyphase<GOInt16>;
        if (bits_per_sample <= 24)
          return &GOSoundStream::MonoUncompressedPolyphase<GOInt24>;
      } else if (channels == 2) {
        if (bits_per_sample <= 8)
          return &GOSoundStream::StereoUncompressedPolyphase<GOInt8>;
        if (bits_per_sample <= 16)
          return &GOSoundStream::StereoUncompressedPolyphase<GOInt16>;
        if (bits_per_sample <= 24)
          return &GOSoundStream::StereoUncompressedPolyphase<GOInt24>;
      }
    } else {
      if (channels == 1) {
        if (bits_per_sample <= 8)
          return &GOSoundStream::MonoUncompressedLinear<GOInt8>;
        if (bits_per_sample <= 16)
          return &GOSoundStream::MonoUncompressedLinear<GOInt16>;
        if (bits_per_sample <= 24)
          return &GOSoundStream::MonoUncompressedLinear<GOInt24>;
      } else if (channels == 2) {
        if (bits_per_sample <= 8)
          return &GOSoundStream::StereoUncompressedLinear<GOInt8>;
        if (bits_per_sample <= 16)
          return &GOSoundStream::StereoUncompressedLinear<GOInt16>;
        if (bits_per_sample <= 24)
          return &GOSoundStream::StereoUncompressedLinear<GOInt24>;
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
  resample_coefs = resampler_coefs;
  ptr = audio_section->GetData();
  transition_position = end.transition_offset;
  end_seg = &end;
  end_ptr = end.end_ptr;
  increment_fraction
    = sample_rate_adjustment * pSection->GetSampleRate() * UPSAMPLE_FACTOR;
  position_index = start.start_offset;
  position_fraction = 0;
  decode_call = getDecodeBlockFunction(
    pSection->GetChannels(),
    pSection->GetBitsPerSample(),
    pSection->IsCompressed(),
    resample_coefs->interpolation,
    false);
  end_decode_call = getDecodeBlockFunction(
    pSection->GetChannels(),
    pSection->GetBitsPerSample(),
    pSection->IsCompressed(),
    resample_coefs->interpolation,
    true);
  margin
    = calculate_margin(pSection->IsCompressed(), resample_coefs->interpolation);
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

  assert(end.transition_offset >= start.start_offset);

  audio_section = pSection;
  ptr = audio_section->GetData();
  transition_position = end.transition_offset;
  end_seg = &end;
  end_ptr = end.end_ptr;
  /* Translate increment in case of differing sample rates */
  resample_coefs = existing_stream->resample_coefs;
  increment_fraction = roundf(
    (((float)existing_stream->increment_fraction)
     / existing_stream->audio_section->GetSampleRate())
    * pSection->GetSampleRate());
  position_index = start.start_offset;
  position_fraction = existing_stream->position_fraction;
  decode_call = getDecodeBlockFunction(
    pSection->GetChannels(),
    pSection->GetBitsPerSample(),
    pSection->IsCompressed(),
    resample_coefs->interpolation,
    false);
  end_decode_call = getDecodeBlockFunction(
    pSection->GetChannels(),
    pSection->GetBitsPerSample(),
    pSection->IsCompressed(),
    resample_coefs->interpolation,
    true);
  margin
    = calculate_margin(pSection->IsCompressed(), resample_coefs->interpolation);
  assert(margin <= MAX_READAHEAD);
  read_end = GOSoundAudioSection::limitedDiff(end.read_end, margin);
  end_pos = end.end_pos - margin;
  cache = start.cache;
  cache.ptr = audio_section->GetData() + (intptr_t)cache.ptr;

  GOSoundReleaseAlignTable *releaseAligner = pSection->GetReleaseAligner();

  if (releaseAligner) {
    int history[BLOCK_HISTORY][MAX_OUTPUT_CHANNELS];

    existing_stream->GetHistory(history);
    position_index = releaseAligner->GetPositionFor(history);
  }
}

bool GOSoundStream::ReadBlock(float *buffer, unsigned int n_blocks) {
  while (n_blocks > 0) {
    if (position_index >= transition_position) {
      assert(end_decode_call);

      /* Setup ptr and position required by the end-block */
      ptr = end_ptr;
      unsigned len = ((UPSAMPLE_FACTOR / 2)
                      + ((end_pos - position_index) << UPSAMPLE_BITS))
        / increment_fraction;
      if (len == 0)
        len = 1;
      len = std::min(len, n_blocks);
      (this->*end_decode_call)(buffer, len);
      buffer += 2 * len;
      n_blocks -= len;

      if (position_index >= end_pos) {
        const GOSoundAudioSection::EndSegment *end = end_seg;

        if (end->next_start_segment_index < 0) {
          for (unsigned i = 0; i < n_blocks * 2; i++)
            buffer[i] = 0;
          return 0;
        }

        position_index -= end->end_offset + 1;
        while (position_index >= end->end_loop_length)
          position_index -= end->end_loop_length;

        const unsigned next_index = end->next_start_segment_index;
        const GOSoundAudioSection::StartSegment *next
          = &audio_section->GetStartSegment(next_index);

        /* Find a suitable end segment */
        const unsigned next_end_segment_index
          = audio_section->PickEndSegment(next_index);
        const GOSoundAudioSection::EndSegment *next_end
          = &audio_section->GetEndSegment(next_end_segment_index);

        position_index += next->start_offset;
        ptr = audio_section->GetData();
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
      unsigned len
        = ((read_end - position_index) << UPSAMPLE_BITS) / increment_fraction;
      if (len == 0)
        len = 1;
      len = std::min(len, n_blocks);
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

  memset(
    history, 0, sizeof(history[0][0]) * BLOCK_HISTORY * MAX_OUTPUT_CHANNELS);
  if (position_index >= transition_position)
    for (unsigned i = 0; i < BLOCK_HISTORY; i++)
      for (uint8_t j = 0; j < nChannels; j++)
        history[i][j] = audio_section->GetSampleData(
          end_ptr, position_index - transition_position + i, j);
  else if (!audio_section->IsCompressed())
    for (unsigned i = 0; i < BLOCK_HISTORY; i++)
      for (uint8_t j = 0; j < nChannels; j++)
        history[i][j]
          = audio_section->GetSampleData(ptr, position_index + i, j);
  else {
    DecompressionCache cache = cache;

    for (unsigned i = 0; i < BLOCK_HISTORY; i++) {
      for (uint8_t j = 0; j < nChannels; j++)
        history[i][j] = cache.value[j];
      DecompressionStep(
        cache, nChannels, audio_section->GetBitsPerSample() >= 20);
    }
  }
}
