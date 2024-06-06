/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDSTREAM_H
#define GOSOUNDSTREAM_H

#include "GOSoundAudioSection.h"
#include "GOSoundResample.h"

class GOSoundStream {
private:
  const GOSoundAudioSection *audio_section;
  const GOSoundResample *resample;

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
  DecompressionCache cache;

  /* The block decode functions should provide whatever the normal resolution of
   * the audio is. The fade engine should ensure that this data is always
   * brought into the correct range. */
  template <class ResamplerT, class WindowT>
  void DecodeBlock(float *pOut, unsigned nOutSamples);

  static DecodeBlockFunction getDecodeBlockFunction(
    uint8_t channels,
    uint8_t bits_per_sample,
    bool compressed,
    GOSoundResample::InterpolationType interpolation,
    bool is_end);

  void GetHistory(int history[BLOCK_HISTORY][MAX_OUTPUT_CHANNELS]) const;

public:
  const unsigned char *GetPtr() const { return ptr; }
  DecompressionCache &GetDecompressionCache() { return cache; }

  /* Initialize a stream to play this audio section */
  void InitStream(
    const GOSoundResample *pResample,
    const GOSoundAudioSection *pSection,
    GOSoundResample::InterpolationType interpolation,
    float sample_rate_adjustment);

  /* Initialize a stream to play this audio section and seek into it using
   * release alignment if available. */
  void InitAlignedStream(
    const GOSoundAudioSection *pSection,
    GOSoundResample::InterpolationType interpolation,
    const GOSoundStream *existing_stream);

  /* Read an audio buffer from an audio section stream */
  bool ReadBlock(float *buffer, unsigned int n_blocks);
};

#endif /* GOSOUNDSTREAM_H */
