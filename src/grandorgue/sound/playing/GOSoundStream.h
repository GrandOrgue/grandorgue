/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDSTREAM_H
#define GOSOUNDSTREAM_H

#include <array>

#include "sound/dsp-kernels/GOSoundResample.h"

#include "GOSoundCompressionCache.h"
#include "GOSoundReleaseAlignTable.h"

class GOSoundAudioSection;
class GOSoundBufferMutable;

class GOSoundStream {
private:
  /* Maximum number of continous samples required for resampling of one target
   * sample. The polyphase algorithm requires the longest sample vector */
  static constexpr unsigned MAX_WINDOW_LEN = GOSoundResample::POLYPHASE_POINTS;
  /* Maximum number of audio chanels in the source samples */
  static constexpr unsigned MAX_INPUT_CHANNELS = 2;

  const GOSoundAudioSection *audio_section;
  const GOSoundResample *resample;

  template <class SampleT, uint8_t nChannels> class StreamPtrWindow;

  template <bool format16, uint8_t nChannels> class StreamCacheWindow;

  template <bool format16, unsigned windowLen, uint8_t nChannels>
  class StreamCacheReadAheadWindow;

  /**
   * Decodes a block from the current resampling position.
   * @param resamplerPosIncrement source advance per output frame
   * @param pOut interleaved stereo output frames
   * @param nOutSamples number of output frames
   */
  using DecodeBlockFunction = void (GOSoundStream::*)(
    unsigned resamplerPosIncrement, float *pOut, unsigned nOutSamples);

  /**
   * Reads one unscaled, unresampled source frame at its absolute index in
   * the audio section. For a compressed window, indices must be at or ahead
   * of the current position and must not decrease between calls. The window
   * may decode ahead, so use this on a copy of a playing stream. For the end
   * segment, first switch to its data with GoToEndSegment().
   * @param frameIndex index of the source frame to read
   * @return one item per channel, with unused channels zeroed
   */
  using ReadFrameFunction = std::array<int, MAX_OUTPUT_CHANNELS> (
    GOSoundStream::*)(unsigned frameIndex);

  /** Pairs the block decoder and frame reader of the same window. */
  struct Decoder {
    DecodeBlockFunction fmDecodeBlock;
    ReadFrameFunction fmReadFrame;
  };

  /** Decoder of the main audio section data. */
  Decoder m_decoder;
  /** Decoder of the uncompressed end segment. */
  Decoder m_EndDecoder;

  /* Current pointer. Used in the DecodeBlock functions. May be a real pointer
   * to the audio section data or an end segment vir tual pointer */
  const unsigned char *ptr;

  // A virtual pointer to the end segment. An real pointer has the
  // transition_position offset from this
  const unsigned char *end_ptr;

  // when to switch to the end segment
  unsigned transition_position;
  // loop end pos, when to switch to the next start segment
  unsigned end_pos;
  // -1 means it is not looped sample, otherwise the index of the start segment
  int m_NextStartSegmentIndex;

  GOSoundResample::ResamplingPosition m_ResamplingPos;

  /* The cache and ring are initialized so the stream can be copied before
   * any compressed data has been decoded. */
  GOSoundCompressionCache cache{};

  /* A ring buffer for resampling of compressed samples. It has double
   * MAX_WINDOW_LEN length for having a continous memory region of
   * MAX_WINDOW_LEN samples */
  int m_ReadAheadBuffer[MAX_INPUT_CHANNELS * MAX_WINDOW_LEN * 2]{};

  /* The block decode functions should provide whatever the normal resolution of
   * the audio is. The fade engine should ensure that this data is always
   * brought into the correct range. */
  template <class ResamplerT, class WindowT>
  void DecodeBlock(
    unsigned resamplerPosIncrement, float *pOut, unsigned nOutSamples);

  /**
   * Reads a source frame through the same window type as its block decoder.
   * @tparam WindowT source frame vector type
   * @param frameIndex absolute source frame index
   * @return the stored items of the frame with unused channels zeroed
   */
  template <class WindowT>
  std::array<int, MAX_OUTPUT_CHANNELS> ReadFrame(unsigned frameIndex);

  /**
   * Pairs decoding and frame reading for one window.
   * @tparam WindowT source frame vector type for both functions
   * @tparam ResamplerT resampler used for block decoding
   */
  template <class WindowT, class ResamplerT> static Decoder makeDecoder();

  /**
   * Returns the decoder for a source format and interpolation type.
   * It is a static class member because it uses the private Decoder type and
   * the private window classes.
   * @param nChannels number of channels, 1 or 2
   * @param nBitsPerSoundItem bit depth of the audio data
   * @param isCompressed whether the audio data is compressed
   * @param interpolationType resampler to use
   * @return the decoder, with both functions null for an unsupported format
   */
  static Decoder getDecoder(
    uint8_t nChannels,
    uint8_t nBitsPerSoundItem,
    bool isCompressed,
    GOSoundResample::InterpolationType interpolationType);

  /**
   * Returns the decoder for the audio data of a section.
   * @param pSection section providing the channel count and bit depth
   * @param isCompressed whether the data is compressed, false for end segments
   * @param interpolationType resampler to use
   * @return the decoder for the given section and interpolation type
   */
  static Decoder getDecoderFor(
    const GOSoundAudioSection *pSection,
    bool isCompressed,
    GOSoundResample::InterpolationType interpolationType);

  /**
   * Reads BLOCK_HISTORY source frames for release alignment on a copy of the
   * stream. An overshoot at the section end starts in the end-segment padding.
   * @param outHistory frames starting at the current source index, clamped
   *   to the section end, with unused channels zeroed
   */
  void GetHistory(GOSoundReleaseAlignTable::History &outHistory) const;

  /**
   * Switches the stream to the start segment with the given index.
   * Sets up ptr, cache, end segment fields and next segment index.
   * Returns start_offset of the start segment.
   */
  unsigned GoToStartSegment(unsigned startSegmentIndex);

  /** Switches the stream to the end segment. */
  void GoToEndSegment() { ptr = end_ptr; }

  /**
   * Initializes audio_section, sets up decode functions and switches
   * to the start segment with the given index.
   * Returns start_offset of the start segment.
   */
  unsigned InitFromSection(
    const GOSoundAudioSection *pSection,
    unsigned startSegmentIndex,
    GOSoundResample::InterpolationType interpolationType);

public:
  /**
   * Initializes a stream to play this audio section from its beginning. The
   * resampling rate is not stored in the stream: the owner passes it to
   * ReadBlock().
   */
  void InitStream(
    const GOSoundResample *pResample,
    const GOSoundAudioSection *pSection,
    GOSoundResample::InterpolationType interpolationType);

  /**
   * Initializes a release stream aligned to the existing attack stream.
   * Uses release alignment table if available. The fractional part of the
   * position is taken from the existing stream.
   */
  void InitAlignedStream(
    const GOSoundAudioSection *pSection,
    GOSoundResample::InterpolationType interpolationType,
    const GOSoundStream *pExistingStream);

  /**
   * @return the sample rate of the audio section the stream plays. It is
   *   used for computing the resampling increment of a stream that takes over
   *   the rate of another one
   */
  unsigned GetSampleRate() const;

  /**
   * Reads an audio buffer from an audio section stream.
   * @param resamplerPosIncrement the constant resampling increment, see
   *   GOSoundResample::rateToFractionIncrement(); must be non-zero
   * @param outBuffer the buffer to fill
   * @return whether the sample has not ended yet
   */
  bool ReadBlock(
    unsigned resamplerPosIncrement, GOSoundBufferMutable &outBuffer);
};

#endif /* GOSOUNDSTREAM_H */
