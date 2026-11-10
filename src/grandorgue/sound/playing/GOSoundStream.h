/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDSTREAM_H
#define GOSOUNDSTREAM_H

#include "sound/dsp-kernels/GOSoundResample.h"

#include "GOSoundCompressionCache.h"

class GOSoundAudioSection;
class GOSoundBufferMutable;

class GOSoundStream {
private:
  /* Maximum number of continous samples required for resampling of one target
   * sample. The polyphase algorithm requires the longest sample vector */
  static constexpr unsigned MAX_WINDOW_LEN = GOSoundResample::POLYPHASE_POINTS;
  /* Maximum number of audio chanels in the source samples */
  static constexpr unsigned MAX_INPUT_CHANNELS = 2;

  static constexpr unsigned DECODE_CHUNK = 32;
  static constexpr unsigned BANDLIMITED_RING_FRAMES = 128;
  static_assert(
    BANDLIMITED_RING_FRAMES
    >= GOSoundResample::BANDLIMITED_POINTS + DECODE_CHUNK);
  static_assert(BANDLIMITED_RING_FRAMES % DECODE_CHUNK == 0);

  const GOSoundAudioSection *audio_section;
  const GOSoundResample *resample;

  template <class SampleT, uint8_t nChannels> class StreamPtrWindow;

  template <bool format16, uint8_t nChannels> class StreamCacheWindow;

  template <bool format16, unsigned windowLen, uint8_t nChannels>
  class StreamCacheReadAheadWindow;

  typedef void (GOSoundStream::*DecodeBlockFunction)(
    unsigned resamplerPosIncrement, float *pOut, unsigned nOutSamples);

  /* Method used to decode stream */
  DecodeBlockFunction decode_call;

  /* Cached method used to decode the data in end_ptr. This is just the
   * uncompressed version of decode_call */
  DecodeBlockFunction end_decode_call;

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

  /* for decoding compressed format */
  GOSoundCompressionCache cache;

  /* A ring buffer for resampling of compressed samples. It has double
   * MAX_WINDOW_LEN length for having a continous memory region of
   * MAX_WINDOW_LEN samples */
  int m_ReadAheadBuffer[MAX_INPUT_CHANNELS * MAX_WINDOW_LEN * 2];

  bool m_IsBandlimited;
  // The decoder runs at unity rate. This cursor tracks the audible position
  // on its continuous timeline, in the resampler's fixed-point units.
  uint64_t m_BandlimitedPosition;
  uint64_t m_DecodedEnd;
  uint64_t m_SourceEnd;
  // The mirror keeps a filter window contiguous when it spans the ring seam.
  float m_DecodedRing[2 * BANDLIMITED_RING_FRAMES * MAX_OUTPUT_CHANNELS];

  /* The block decode functions should provide whatever the normal resolution of
   * the audio is. The fade engine should ensure that this data is always
   * brought into the correct range. */
  template <class ResamplerT, class WindowT>
  void DecodeBlock(
    unsigned resamplerPosIncrement, float *pOut, unsigned nOutSamples);

  /* These functions must be static class members rather than file-scope
   * functions because they reference the private typedef DecodeBlockFunction
   * and the private nested window classes (StreamPtrWindow, StreamCacheWindow,
   * StreamCacheReadAheadWindow). */
  static DecodeBlockFunction getDecodeBlockFunction(
    uint8_t nChannels,
    uint8_t nBitsPerSoundItem,
    bool isCompressed,
    GOSoundResample::InterpolationType interpolationType);

  /**
   * Returns a decode block function for the given audio section,
   * compression flag and interpolation type.
   */
  static DecodeBlockFunction getDecodeBlockFunctionFor(
    const GOSoundAudioSection *pSection,
    bool isCompressed,
    GOSoundResample::InterpolationType interpolationType);

  /** Reads the source frames at the audible position for release alignment. */
  void GetHistory(int history[BLOCK_HISTORY][MAX_OUTPUT_CHANNELS]);

  /** Resets the decoder and, for bandlimited playback, its centered window. */
  void InitPosition(unsigned startOffset, unsigned fraction);

  /** Fills the buffer using the selected decoder, returning valid frames. */
  unsigned ReadDecodedBlock(
    unsigned resamplerPosIncrement, GOSoundBufferMutable &outBuffer);

  /** Decodes through the requested window or pads the source end with silence.
   */
  void EnsureDecodedWindow(uint64_t center, unsigned nLookAheadFrames);

  /** Renders a block from the decoded ring, advancing the audible cursor. */
  bool ReadBandlimitedBlock(
    unsigned resamplerPosIncrement, GOSoundBufferMutable &outBuffer);

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
   * @param pExistingStream the attack stream, whose decoder may read ahead
   *   for alignment without advancing its audible position
   */
  void InitAlignedStream(
    const GOSoundAudioSection *pSection,
    GOSoundResample::InterpolationType interpolationType,
    GOSoundStream *pExistingStream);

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
