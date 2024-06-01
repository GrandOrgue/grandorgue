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

  /* Used for both compressed and uncompressed decoding */
  const unsigned char *ptr;

  /* Derived from the start and end audio segments which were used to setup
   * this stream. */
  const GOSoundAudioSection::EndSegment *end_seg;
  const unsigned char *end_ptr;

  // How many last samples are reserved for resample vector
  unsigned margin;
  unsigned transition_position;
  unsigned read_end;
  unsigned end_pos;

  GOSoundResample::ResamplingPosition resamplingPos;

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
