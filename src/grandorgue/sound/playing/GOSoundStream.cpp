/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundStream.h"

#include <algorithm>

#include <wx/log.h>

#include "sound/buffer/GOSoundBufferMutable.h"

#include "GOSoundAudioSection.h"

/* Block reading functions */

template <class SampleT, uint8_t nChannels>
class GOSoundStream::StreamPtrWindow
  : public GOSoundResample::PtrFrameVector<SampleT, int, nChannels> {
public:
  inline StreamPtrWindow(GOSoundStream &stream)
    : GOSoundResample::PtrFrameVector<SampleT, int, nChannels>(
      (SampleT *)stream.ptr) {}
};

template <bool format16, uint8_t nChannels>
class GOSoundStream::StreamCacheWindow
  : public GOSoundResample::FloatingFrameVector<nChannels> {
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

  inline int NextItem() {
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

  inline void NormalizePosition(GOSoundResample::ResamplingPosition &) const {}
};

template <bool format16, unsigned windowLen, uint8_t nChannels>
class GOSoundStream::StreamCacheReadAheadWindow
  : public GOSoundResample::PtrFrameVector<int, int, nChannels> {
private:
  static constexpr unsigned WINDOW_SAMPLES = nChannels * windowLen;
  static constexpr unsigned BUFFER_SAMPLES = WINDOW_SAMPLES * 2;

  GOSoundCompressionCache &r_cache;
  int *p_begin;
  int *p_end;

public:
  inline StreamCacheReadAheadWindow(GOSoundStream &stream)
    : GOSoundResample::PtrFrameVector<int, int, nChannels>(
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
        /* The sample about to be decoded has this index; DecompressionStep
         * increments m_position afterwards, so m_position itself no longer
         * identifies it. */
        unsigned lastDecompressedPos = r_cache.m_position;

        r_cache.DecompressionStep(nChannels, format16);

        /* fill the read ahead buffer. If lastDecompressedPos >= index we
          assume that the previous samples already present */
        if (lastDecompressedPos >= index) {
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
    GOSoundResample::PtrFrameVector<int, int, nChannels>::Seek(
      index % windowLen, channelN);
  }
};

/* The block decode functions should provide whatever the normal resolution of
 * the audio is. The fade engine should ensure that this data is always brought
 * into the correct range. */

template <class ResamplerT, class WindowT>
void GOSoundStream::DecodeBlock(
  unsigned resamplerPosIncrement, float *pOut, unsigned nOutSamples) {
  ResamplerT resampler(*resample);
  WindowT w(*this);

  resampler.template ResampleBlock<WindowT, 2>(
    resamplerPosIncrement, m_ResamplingPos, w, pOut, nOutSamples);
}

template <class WindowT>
std::array<int, MAX_OUTPUT_CHANNELS> GOSoundStream::ReadFrame(
  unsigned frameIndex) {
  static_assert(WindowT::m_NChannels <= MAX_OUTPUT_CHANNELS);

  WindowT w(*this);
  std::array<int, MAX_OUTPUT_CHANNELS> items{};
  int *pItem = items.data();

  for (uint8_t channelI = 0; channelI < WindowT::m_NChannels; ++channelI) {
    w.Seek(frameIndex, channelI);
    *(pItem++) = w.NextItem();
  }
  return items;
}

template <class WindowT, class ResamplerT>
GOSoundStream::Decoder GOSoundStream::makeDecoder() {
  return {
    &GOSoundStream::DecodeBlock<ResamplerT, WindowT>,
    &GOSoundStream::ReadFrame<WindowT>};
}

GOSoundStream::Decoder GOSoundStream::getDecoder(
  uint8_t nChannels,
  uint8_t nBitsPerSoundItem,
  bool isCompressed,
  GOSoundResample::InterpolationType interpolationType) {

  if (interpolationType == GOSoundResample::GO_POLYPHASE_INTERPOLATION) {
    if (isCompressed) {
      // Polyphase interpolation + compressed audio
      if (nChannels == 1) {
        if (nBitsPerSoundItem >= 20)
          return makeDecoder<
            StreamCacheReadAheadWindow<
              true,
              GOSoundResample::PolyphaseResampler::VECTOR_LENGTH,
              1>,
            GOSoundResample::PolyphaseResampler>();

        assert(nBitsPerSoundItem >= 12);
        return makeDecoder<
          StreamCacheReadAheadWindow<
            false,
            GOSoundResample::PolyphaseResampler::VECTOR_LENGTH,
            1>,
          GOSoundResample::PolyphaseResampler>();
      } else if (nChannels == 2) {
        if (nBitsPerSoundItem >= 20)
          return makeDecoder<
            StreamCacheReadAheadWindow<
              true,
              GOSoundResample::PolyphaseResampler::VECTOR_LENGTH,
              2>,
            GOSoundResample::PolyphaseResampler>();

        assert(nBitsPerSoundItem >= 12);
        return makeDecoder<
          StreamCacheReadAheadWindow<
            false,
            GOSoundResample::PolyphaseResampler::VECTOR_LENGTH,
            2>,
          GOSoundResample::PolyphaseResampler>();
      }
    } else {
      // Polyphase interpolation + uncompressed audio
      if (nChannels == 1) {
        if (nBitsPerSoundItem <= 8)
          return makeDecoder<
            StreamPtrWindow<GOInt8, 1>,
            GOSoundResample::PolyphaseResampler>();
        if (nBitsPerSoundItem <= 16)
          return makeDecoder<
            StreamPtrWindow<GOInt16, 1>,
            GOSoundResample::PolyphaseResampler>();
        if (nBitsPerSoundItem <= 24)
          return makeDecoder<
            StreamPtrWindow<GOInt24, 1>,
            GOSoundResample::PolyphaseResampler>();
      } else if (nChannels == 2) {
        if (nBitsPerSoundItem <= 8)
          return makeDecoder<
            StreamPtrWindow<GOInt8, 2>,
            GOSoundResample::PolyphaseResampler>();
        if (nBitsPerSoundItem <= 16)
          return makeDecoder<
            StreamPtrWindow<GOInt16, 2>,
            GOSoundResample::PolyphaseResampler>();
        if (nBitsPerSoundItem <= 24)
          return makeDecoder<
            StreamPtrWindow<GOInt24, 2>,
            GOSoundResample::PolyphaseResampler>();
      }
    }
  } else {
    // Linear interpolation
    if (isCompressed) {
      // Linear interpolation + compressed audio
      if (nChannels == 1) {
        if (nBitsPerSoundItem >= 20)
          return makeDecoder<
            StreamCacheWindow<true, 1>,
            GOSoundResample::LinearResampler>();

        assert(nBitsPerSoundItem >= 12);
        return makeDecoder<
          StreamCacheWindow<false, 1>,
          GOSoundResample::LinearResampler>();
      } else if (nChannels == 2) {
        if (nBitsPerSoundItem >= 20)
          return makeDecoder<
            StreamCacheWindow<true, 2>,
            GOSoundResample::LinearResampler>();

        assert(nBitsPerSoundItem >= 12);
        return makeDecoder<
          StreamCacheWindow<false, 2>,
          GOSoundResample::LinearResampler>();
      }
    } else {
      // Linear interpolation + uncompressed audio
      if (nChannels == 1) {
        if (nBitsPerSoundItem <= 8)
          return makeDecoder<
            StreamPtrWindow<GOInt8, 1>,
            GOSoundResample::LinearResampler>();
        if (nBitsPerSoundItem <= 16)
          return makeDecoder<
            StreamPtrWindow<GOInt16, 1>,
            GOSoundResample::LinearResampler>();
        if (nBitsPerSoundItem <= 24)
          return makeDecoder<
            StreamPtrWindow<GOInt24, 1>,
            GOSoundResample::LinearResampler>();
      } else if (nChannels == 2) {
        if (nBitsPerSoundItem <= 8)
          return makeDecoder<
            StreamPtrWindow<GOInt8, 2>,
            GOSoundResample::LinearResampler>();
        if (nBitsPerSoundItem <= 16)
          return makeDecoder<
            StreamPtrWindow<GOInt16, 2>,
            GOSoundResample::LinearResampler>();
        if (nBitsPerSoundItem <= 24)
          return makeDecoder<
            StreamPtrWindow<GOInt24, 2>,
            GOSoundResample::LinearResampler>();
      }
    }
  }

  assert(0 && "unsupported decoder configuration");
  return {};
}

GOSoundStream::Decoder GOSoundStream::getDecoderFor(
  const GOSoundAudioSection *pSection,
  bool isCompressed,
  GOSoundResample::InterpolationType interpolationType) {
  return getDecoder(
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
  if (audio_section->IsCompressed()) {
    cache = startSegment.cache;
    cache.m_ptr = pData + (intptr_t)cache.m_ptr;
  }

  // end segment
  transition_position = endSegment.transition_offset;
  end_ptr = endSegment.end_ptr;
  end_pos = endSegment.end_pos;

  // next start segment
  m_NextStartSegmentIndex = endSegment.next_start_segment_index;

  assert(end_pos >= startSegment.start_offset);

  return startSegment.start_offset;
}

unsigned GOSoundStream::InitFromSection(
  const GOSoundAudioSection *pSection,
  unsigned startSegmentIndex,
  GOSoundResample::InterpolationType interpolationType) {
  audio_section = pSection;
  m_decoder
    = getDecoderFor(pSection, pSection->IsCompressed(), interpolationType);
  // End segments are never compressed
  m_EndDecoder = getDecoderFor(pSection, false, interpolationType);

  return GoToStartSegment(startSegmentIndex);
}

void GOSoundStream::InitStream(
  const GOSoundResample *pResample,
  const GOSoundAudioSection *pSection,
  GOSoundResample::InterpolationType interpolationType) {
  resample = pResample;

  const unsigned startOffset = InitFromSection(pSection, 0, interpolationType);

  m_ResamplingPos.Init(startOffset);
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
    GOSoundReleaseAlignTable::History history;

    pExistingStream->GetHistory(history);
    startOffset
      = releaseAligner->GetPositionFor(history, pSection->GetChannels());
  }
  m_ResamplingPos.Init(
    startOffset, pExistingStream->m_ResamplingPos.GetFraction());
}

unsigned GOSoundStream::GetSampleRate() const {
  return audio_section->GetSampleRate();
}

bool GOSoundStream::ReadBlock(
  unsigned resamplerPosIncrement, GOSoundBufferMutable &outBuffer) {
  float *pData = outBuffer.GetData();
  unsigned nFrames = outBuffer.GetNFrames();
  bool res = true;

  while (nFrames > 0) {
    unsigned currSrcOffset = m_ResamplingPos.GetIndex();

    if (currSrcOffset < end_pos) { // the loop has not yet fully played
      // whether we are playing the start or the end segment
      bool isToPlayMain = currSrcOffset < transition_position;
      unsigned finishPos = isToPlayMain ? transition_position : end_pos;
      unsigned targetSamples = std::min(
        m_ResamplingPos.AvailableTargetSamples(
          finishPos, resamplerPosIncrement),
        nFrames);

      assert(targetSamples > 0);
      if (isToPlayMain) {
        assert(m_decoder.fmDecodeBlock);
        (this->*m_decoder.fmDecodeBlock)(
          resamplerPosIncrement, pData, targetSamples);
      } else {
        assert(m_EndDecoder.fmDecodeBlock);

        // Setup ptr and position required by the end-block
        GoToEndSegment();
        // we continue to use the same position as before because end_ptr is a
        // "virtual" pointer: end_data - transition_offset
        (this->*m_EndDecoder.fmDecodeBlock)(
          resamplerPosIncrement, pData, targetSamples);
      }
      pData += 2 * targetSamples;
      nFrames -= targetSamples;
      assert(!nFrames || m_ResamplingPos.GetIndex() >= finishPos);
    } else if (m_NextStartSegmentIndex >= 0) {
      // switch to the start of the loop.
      // overflowOffset must be computed BEFORE GoToStartSegment(): it
      // overwrites end_pos with the new segment's end position.
      const unsigned overflowOffset = currSrcOffset - end_pos;
      const unsigned startOffset = GoToStartSegment(m_NextStartSegmentIndex);
      const unsigned newSrcOffset = startOffset + overflowOffset;

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
      outBuffer.GetSubBuffer(outBuffer.GetNFrames() - nFrames, nFrames)
        .FillWithSilence();
      nFrames = 0;
      res = false;
    }
  }
  return res;
}

void GOSoundStream::GetHistory(
  GOSoundReleaseAlignTable::History &outHistory) const {
  // The window may decode ahead. Keep the playing stream's state intact.
  GOSoundStream stream = *this;
  unsigned frameIndex = std::min(m_ResamplingPos.GetIndex(), end_pos);

  static_assert(BLOCK_HISTORY <= GOSoundAudioSection::MAX_READAHEAD);
  for (auto &frame : outHistory) {
    const bool isInEndSegment = frameIndex >= transition_position;
    const ReadFrameFunction fmReadFrame
      = (isInEndSegment ? m_EndDecoder : m_decoder).fmReadFrame;

    if (isInEndSegment)
      stream.GoToEndSegment();
    assert(fmReadFrame);
    frame = (stream.*fmReadFrame)(frameIndex++);
  }
}
