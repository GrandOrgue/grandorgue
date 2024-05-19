/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDSTREAM_H
#define GOSOUNDSTREAM_H

#include "GOSoundAudioSection.h"

class GOSoundStream {
private:
  const GOSoundAudioSection *audio_section;
  const struct GOSoundResample *resample_coefs;

  typedef void (GOSoundStream::*DecodeBlockFunction)(
    float *output, unsigned int n_blocks);

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
  unsigned margin;
  unsigned transition_position;
  unsigned read_end;
  unsigned end_pos;

  unsigned position_index;
  unsigned position_fraction;
  unsigned increment_fraction;

  /* for decoding compressed format */
  DecompressionCache cache;

  inline void NormalisePosition() {
    position_index += position_fraction >> UPSAMPLE_BITS;
    position_fraction = position_fraction & (UPSAMPLE_FACTOR - 1);
  }

  template <class T>
  void MonoUncompressedLinear(float *output, unsigned int n_blocks);
  template <class T>
  void StereoUncompressedLinear(float *output, unsigned int n_blocks);
  template <class T>
  void MonoUncompressedPolyphase(float *output, unsigned int n_blocks);
  template <class T>
  void StereoUncompressedPolyphase(float *output, unsigned int n_blocks);
  template <bool format16>
  void MonoCompressedLinear(float *output, unsigned int n_blocks);
  template <bool format16>
  void StereoCompressedLinear(float *output, unsigned int n_blocks);

  static DecodeBlockFunction getDecodeBlockFunction(
    unsigned channels,
    unsigned bits_per_sample,
    bool compressed,
    GOSoundResample::InterpolationType interpolation,
    bool is_end);

  void GetHistory(int history[BLOCK_HISTORY][MAX_OUTPUT_CHANNELS]) const;

public:
  /* Initialize a stream to play this audio section */
  void InitStream(
    const GOSoundAudioSection *pSection,
    const struct GOSoundResample *resampler_coefs,
    float sample_rate_adjustment);

  /* Initialize a stream to play this audio section and seek into it using
   * release alignment if available. */
  void InitAlignedStream(
    const GOSoundAudioSection *pSection, const GOSoundStream *existing_stream);

  /* Read an audio buffer from an audio section stream */
  bool ReadBlock(float *buffer, unsigned int n_blocks);
};

#endif /* GOSOUNDSTREAM_H */
