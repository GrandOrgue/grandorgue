/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
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
  GOSoundCompressionCache &r_cache;
  uint8_t m_ChannelN;
  enum { PREV, VALUE, ZERO } m_curr;

public:
  inline StreamCacheWindow(GOSoundStream &stream) : r_cache(stream.cache) {}

  inline void Seek(unsigned index, uint8_t channelN) {
    while (r_cache.m_position <= index + 1) {
      r_cache.DecompressionStep(nChannels, format16);
    }
    m_ChannelN = channelN;
    m_curr = PREV;
  }

  inline int NextSample() {
    int res;

    if (m_curr == PREV) {
      res = r_cache.m_prev[m_ChannelN];
      m_curr = VALUE;
    } else if (m_curr == VALUE) {
      res = r_cache.m_value[m_ChannelN];
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

  GOSoundCompressionCache &r_cache;
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

    if (r_cache.m_position < readAheadIndexTo) {
      unsigned writePosition = std::max(r_cache.m_position, index);
      // pWrite1 points somewhere in the first half of the buffer
      int *pWrite1 = p_begin + nChannels * (writePosition % windowLen);
      // pWrite2 points somewhere in the second half of the buffer
      int *pWrite2 = pWrite1 + WINDOW_SAMPLES;

      while (r_cache.m_position < readAheadIndexTo) {
        r_cache.DecompressionStep(nChannels, format16);

        /* fill the read ahead buffer. If r_cache.position > index we assume
          that the previous samples already present */
        if (r_cache.m_position >= index) {
          const int *pRead = r_cache.m_value;

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
  uint8_t nChannels,
  uint8_t nBitsPerSoundItem,
  bool isCompressed,
  GOSoundResample::InterpolationType interpolationType) {

  if (interpolationType == GOSoundResample::GO_POLYPHASE_INTERPOLATION) {
    if (isCompressed) {
      // Polyphase interpolation + compressed audio
      if (nChannels == 1) {
        if (nBitsPerSoundItem >= 20)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::PolyphaseResampler,
            StreamCacheReadAheadWindow<
              true,
              GOSoundResample::PolyphaseResampler::VECTOR_LENGTH,
              1>>;

        assert(nBitsPerSoundItem >= 12);
        return &GOSoundStream::DecodeBlock<
          GOSoundResample::PolyphaseResampler,
          StreamCacheReadAheadWindow<
            false,
            GOSoundResample::PolyphaseResampler::VECTOR_LENGTH,
            1>>;
      } else if (nChannels == 2) {
        if (nBitsPerSoundItem >= 20)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::PolyphaseResampler,
            StreamCacheReadAheadWindow<
              true,
              GOSoundResample::PolyphaseResampler::VECTOR_LENGTH,
              2>>;

        assert(nBitsPerSoundItem >= 12);
        return &GOSoundStream::DecodeBlock<
          GOSoundResample::PolyphaseResampler,
          StreamCacheReadAheadWindow<
            false,
            GOSoundResample::PolyphaseResampler::VECTOR_LENGTH,
            2>>;
      }
    } else {
      // Polyphase interpolation + uncompressed audio
      if (nChannels == 1) {
        if (nBitsPerSoundItem <= 8)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::PolyphaseResampler,
            StreamPtrWindow<GOInt8, 1>>;
        if (nBitsPerSoundItem <= 16)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::PolyphaseResampler,
            StreamPtrWindow<GOInt16, 1>>;
        if (nBitsPerSoundItem <= 24)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::PolyphaseResampler,
            StreamPtrWindow<GOInt24, 1>>;
      } else if (nChannels == 2) {
        if (nBitsPerSoundItem <= 8)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::PolyphaseResampler,
            StreamPtrWindow<GOInt8, 2>>;
        if (nBitsPerSoundItem <= 16)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::PolyphaseResampler,
            StreamPtrWindow<GOInt16, 2>>;
        if (nBitsPerSoundItem <= 24)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::PolyphaseResampler,
            StreamPtrWindow<GOInt24, 2>>;
      }
    }
  } else {
    // Linear interpolation
    if (isCompressed) {
      // Linear interpolation + compressed audio
      if (nChannels == 1) {
        if (nBitsPerSoundItem >= 20)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::LinearResampler,
            StreamCacheWindow<true, 1>>;

        assert(nBitsPerSoundItem >= 12);
        return &GOSoundStream::DecodeBlock<
          GOSoundResample::LinearResampler,
          StreamCacheWindow<false, 1>>;
      } else if (nChannels == 2) {
        if (nBitsPerSoundItem >= 20)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::LinearResampler,
            StreamCacheWindow<true, 2>>;

        assert(nBitsPerSoundItem >= 12);
        return &GOSoundStream::DecodeBlock<
          GOSoundResample::LinearResampler,
          StreamCacheWindow<false, 2>>;
      }
    } else {
      // Linear interpolation + uncompressed audio
      if (nChannels == 1) {
        if (nBitsPerSoundItem <= 8)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::LinearResampler,
            StreamPtrWindow<GOInt8, 1>>;
        if (nBitsPerSoundItem <= 16)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::LinearResampler,
            StreamPtrWindow<GOInt16, 1>>;
        if (nBitsPerSoundItem <= 24)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::LinearResampler,
            StreamPtrWindow<GOInt24, 1>>;
      } else if (nChannels == 2) {
        if (nBitsPerSoundItem <= 8)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::LinearResampler,
            StreamPtrWindow<GOInt8, 2>>;
        if (nBitsPerSoundItem <= 16)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::LinearResampler,
            StreamPtrWindow<GOInt16, 2>>;
        if (nBitsPerSoundItem <= 24)
          return &GOSoundStream::DecodeBlock<
            GOSoundResample::LinearResampler,
            StreamPtrWindow<GOInt24, 2>>;
      }
    }
  }

  assert(0 && "unsupported decoder configuration");
  return NULL;
}

GOSoundStream::DecodeBlockFunction GOSoundStream::getDecodeBlockFunctionFor(
  const GOSoundAudioSection *pSection,
  bool isCompressed,
  GOSoundResample::InterpolationType interpolationType) {
  return getDecodeBlockFunction(
    pSection->GetChannels(),
    pSection->GetBitsPerSample(),
    isCompressed,
    interpolationType);
}

unsigned GOSoundStream::GoToStartSegment(unsigned startSegmentIndex) {
  const GOSoundAudioSection::StartSegment &startSegment
    = audio_section->GetStartSegment(startSegmentIndex);
  const GOSoundAudioSection::EndSegment &endSegment
    = audio_section->GetEndSegment(
      audio_section->PickEndSegment(startSegmentIndex));
  const unsigned char *pData = audio_section->GetData();

  // start segment
  ptr = pData;
  cache = startSegment.cache;
  cache.m_ptr = pData + (intptr_t)cache.m_ptr;

  // end segment
  transition_position = endSegment.transition_offset;
  end_ptr = endSegment.end_ptr;
  end_pos = endSegment.end_pos;

  // next start segment
  m_NextStartSegmentIndex = endSegment.next_start_segment_index;

  return startSegment.start_offset;
}

unsigned GOSoundStream::InitFromSection(
  const GOSoundAudioSection *pSection,
  unsigned startSegmentIndex,
  GOSoundResample::InterpolationType interpolationType) {
  audio_section = pSection;

  decode_call = getDecodeBlockFunctionFor(
    pSection, pSection->IsCompressed(), interpolationType);
  // End segments are never compressed
  end_decode_call
    = getDecodeBlockFunctionFor(pSection, false, interpolationType);

  return GoToStartSegment(startSegmentIndex);
}

void GOSoundStream::InitStream(
  const GOSoundResample *pResample,
  const GOSoundAudioSection *pSection,
  GOSoundResample::InterpolationType interpolationType,
  float sampleRateAdjustment) {
  resample = pResample;

  const unsigned startOffset = InitFromSection(pSection, 0, interpolationType);

  m_ResamplingPos.Init(
    sampleRateAdjustment * pSection->GetSampleRate(), startOffset);
}

void GOSoundStream::InitAlignedStream(
  const GOSoundAudioSection *pSection,
  GOSoundResample::InterpolationType interpolationType,
  const GOSoundStream *pExistingStream) {
  // Release section
  GOSoundReleaseAlignTable *releaseAligner = pSection->GetReleaseAligner();

  resample = pExistingStream->resample;

  unsigned startOffset = InitFromSection(
    pSection, pSection->GetReleaseStartSegment(), interpolationType);

  if (releaseAligner) {
    int history[BLOCK_HISTORY][MAX_OUTPUT_CHANNELS];

    pExistingStream->GetHistory(history);
    startOffset = releaseAligner->GetPositionFor(history);
  }
  m_ResamplingPos.Init(
    (float)pSection->GetSampleRate()
      / pExistingStream->audio_section->GetSampleRate(),
    startOffset,
    &pExistingStream->m_ResamplingPos);
}

bool GOSoundStream::ReadBlock(float *buffer, unsigned int n_blocks) {
  bool res = true;

  while (n_blocks > 0) {
    unsigned currSrcOffset = m_ResamplingPos.GetIndex();

    if (currSrcOffset < end_pos) { // the loop has not yet fully played
      // whether we are playing the start or the end segment
      bool isToPlayMain = currSrcOffset < transition_position;
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
        GoToEndSegment();
        // we continue to use the same position as before because end_ptr is a
        // "virtual" pointer: end_data - transition_offset
        (this->*end_decode_call)(buffer, targetSamples);
      }
      buffer += 2 * targetSamples;
      n_blocks -= targetSamples;
      assert(!n_blocks || m_ResamplingPos.GetIndex() >= finishPos);
    } else if (m_NextStartSegmentIndex >= 0) {
      // switch to the start of the loop
      const unsigned startOffset = GoToStartSegment(m_NextStartSegmentIndex);
      const unsigned newSrcOffset = startOffset + (currSrcOffset - end_pos);

      assert(end_pos >= startOffset);

      m_ResamplingPos.SetIndex(newSrcOffset);
      if (newSrcOffset >= end_pos) { // invalid loop. Using it might cause
                                     // infinite iterations here
        wxLogError(
          "GOSoundStream::ReadBlock: Breaking invalid loop: start_offset=%d, "
          "end_pos=%d, new_pos=%d",
          startOffset,
          end_pos,
          newSrcOffset);
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
    GOSoundCompressionCache tmpCache = cache;

    for (unsigned i = 0; i < BLOCK_HISTORY; i++) {
      for (uint8_t j = 0; j < nChannels; j++)
        history[i][j] = tmpCache.m_value[j];
      tmpCache.DecompressionStep(
        nChannels, audio_section->GetBitsPerSample() >= 20);
    }
  }
}
