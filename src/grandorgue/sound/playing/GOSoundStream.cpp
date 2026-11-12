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
#include "GOSoundReleaseAlignTable.h"

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
  ResamplerT resampler(*resample, resamplerPosIncrement);
  WindowT w(*this);

  resampler.template ResampleBlock<WindowT, 2>(
    resamplerPosIncrement, m_ResamplingPos, w, pOut, nOutSamples);
}

template <class ResamplerT, uint8_t nChannels>
GOSoundStream::DecodeBlockFunction GOSoundStream::getDecodeBlockFunction(
  uint8_t nBitsPerSoundItem, bool isCompressed) {
  DecodeBlockFunction result = nullptr;

  if (isCompressed) {
    assert(nBitsPerSoundItem >= 12);
    if constexpr (ResamplerT::VECTOR_LENGTH == GOSoundResample::LINEAR_POINTS)
      result = nBitsPerSoundItem >= 20
        ? &GOSoundStream::
            DecodeBlock<ResamplerT, StreamCacheWindow<true, nChannels>>
        : &GOSoundStream::
            DecodeBlock<ResamplerT, StreamCacheWindow<false, nChannels>>;
    else
      result = nBitsPerSoundItem >= 20 ? &GOSoundStream::DecodeBlock<
                 ResamplerT,
                 StreamCacheReadAheadWindow<
                   true,
                   ResamplerT::VECTOR_LENGTH,
                   nChannels>>
                                       : &GOSoundStream::DecodeBlock<
                                         ResamplerT,
                                         StreamCacheReadAheadWindow<
                                           false,
                                           ResamplerT::VECTOR_LENGTH,
                                           nChannels>>;
  } else if (nBitsPerSoundItem <= 8)
    result = &GOSoundStream::
               DecodeBlock<ResamplerT, StreamPtrWindow<GOInt8, nChannels>>;
  else if (nBitsPerSoundItem <= 16)
    result = &GOSoundStream::
               DecodeBlock<ResamplerT, StreamPtrWindow<GOInt16, nChannels>>;
  else if (nBitsPerSoundItem <= 24)
    result = &GOSoundStream::
               DecodeBlock<ResamplerT, StreamPtrWindow<GOInt24, nChannels>>;

  assert(result && "unsupported decoder configuration");
  return result;
}

GOSoundStream::DecodeBlockFunction GOSoundStream::getDecodeBlockFunctionFor(
  const GOSoundAudioSection *pSection,
  bool isCompressed,
  GOSoundResample::InterpolationType interpolationType) {
  using Resample = GOSoundResample;
  const unsigned nChannels = pSection->GetChannels();
  const uint8_t nBitsPerSample = pSection->GetBitsPerSample();
  const auto getDecoder = [&]<class ResamplerT>() {
    return nChannels == 1
      ? getDecodeBlockFunction<ResamplerT, 1>(nBitsPerSample, isCompressed)
      : getDecodeBlockFunction<ResamplerT, 2>(nBitsPerSample, isCompressed);
  };
  DecodeBlockFunction result = nullptr;

  assert(nChannels == 1 || nChannels == 2);
  switch (interpolationType) {
  case Resample::GO_LINEAR_INTERPOLATION:
    result = getDecoder.operator()<Resample::LinearResampler>();
    break;
  case Resample::GO_POLYPHASE_INTERPOLATION:
    result = getDecoder.operator()<Resample::PolyphaseResampler>();
    break;
  case Resample::GO_BANDLIMITED_INTERPOLATION:
    result = getDecoder.operator()<Resample::BandlimitedResampler>();
    break;
  }
  assert(result);
  return result;
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

  m_WindowLength = GOSoundResample::getVectorLength(interpolationType);
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
    int history[BLOCK_HISTORY][MAX_OUTPUT_CHANNELS];

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
  assert(resamplerPosIncrement);
  assert(outBuffer.GetNChannels() == MAX_OUTPUT_CHANNELS);

  float *pData = outBuffer.GetData();
  unsigned nFrames = outBuffer.GetNFrames();

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
        assert(decode_call);
        (this->*decode_call)(resamplerPosIncrement, pData, targetSamples);
      } else {
        assert(end_decode_call);

        // Setup ptr and position required by the end-block
        GoToEndSegment();
        // we continue to use the same position as before because end_ptr is a
        // "virtual" pointer: end_data - transition_offset
        (this->*end_decode_call)(resamplerPosIncrement, pData, targetSamples);
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
      break;
    }
  }
  return nFrames == 0;
}

void GOSoundStream::GetHistory(
  int history[BLOCK_HISTORY][MAX_OUTPUT_CHANNELS]) const {
  const uint8_t nChannels = audio_section->GetChannels();
  const unsigned start = std::min(m_ResamplingPos.GetIndex(), end_pos);
  const bool isCompressed = audio_section->IsCompressed();
  GOSoundCompressionCache tmpCache;

  if (isCompressed)
    tmpCache = cache;

  memset(
    history, 0, sizeof(history[0][0]) * BLOCK_HISTORY * MAX_OUTPUT_CHANNELS);
  for (unsigned frameI = 0; frameI < BLOCK_HISTORY; ++frameI) {
    const unsigned index = start + frameI;

    if (index >= transition_position || !isCompressed) {
      // The end segment pointer is already offset to the source index origin.
      const auto *pData
        = index >= transition_position ? end_ptr : audio_section->GetData();

      for (unsigned channelI = 0; channelI < nChannels; ++channelI)
        history[frameI][channelI]
          = audio_section->GetSampleData(pData, index, channelI);
    } else if (
      m_WindowLength > GOSoundResample::LINEAR_POINTS
      && index < cache.m_position) {
      // The wider decoder keeps its decoded window in the mirrored buffer.
      const unsigned offset = (index % m_WindowLength) * nChannels;

      std::copy_n(m_ReadAheadBuffer + offset, nChannels, history[frameI]);
    } else {
      while (tmpCache.m_position <= index)
        tmpCache.DecompressionStep(
          nChannels, audio_section->GetBitsPerSample() >= 20);

      const int *pValues
        = tmpCache.m_position == index + 1 ? tmpCache.m_value : tmpCache.m_prev;

      std::copy_n(pValues, nChannels, history[frameI]);
    }
  }
}
