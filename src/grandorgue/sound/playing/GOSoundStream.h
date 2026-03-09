/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDSTREAM_H
#define GOSOUNDSTREAM_H

#include "GOSoundCompressionCache.h"
#include "GOSoundResample.h"

class GOSoundAudioSection;

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

  typedef void (GOSoundStream::*DecodeBlockFunction)(
    float *pOut, unsigned nOutSamples);

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

  /* The block decode functions should provide whatever the normal resolution of
   * the audio is. The fade engine should ensure that this data is always
   * brought into the correct range. */
  template <class ResamplerT, class WindowT>
  void DecodeBlock(float *pOut, unsigned nOutSamples);

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

  void GetHistory(int history[BLOCK_HISTORY][MAX_OUTPUT_CHANNELS]) const;

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
  /* Initialize a stream to play this audio section */
  void InitStream(
    const GOSoundResample *pResample,
    const GOSoundAudioSection *pSection,
    GOSoundResample::InterpolationType interpolationType,
    float sampleRateAdjustment);

  /**
   * Initializes a release stream aligned to the existing attack stream.
   * Uses release alignment table if available.
   */
  void InitAlignedStream(
    const GOSoundAudioSection *pSection,
    GOSoundResample::InterpolationType interpolationType,
    const GOSoundStream *pExistingStream);

  /* Read an audio buffer from an audio section stream */
  bool ReadBlock(float *buffer, unsigned int n_blocks);
};

#endif /* GOSOUNDSTREAM_H */
