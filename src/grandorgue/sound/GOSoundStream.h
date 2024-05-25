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
  unsigned margin;
  unsigned transition_position;
  unsigned read_end;
  unsigned end_pos;

  GOSoundResample::ResamplingPosition resamplingPos;

  /* for decoding compressed format */
  DecompressionCache cache;

  template <class SampleT, uint8_t nChannels>
  class StreamPtrWindow
    : public GOSoundResample::PointerWindow<SampleT, nChannels> {
  public:
    inline StreamPtrWindow(GOSoundStream &stream)
      : GOSoundResample::PointerWindow<SampleT, nChannels>(
        (SampleT *)stream.ptr, stream.end_pos) {}
  };

  template <bool format16, uint8_t nChannels>
  class StreamCacheWindow : public GOSoundResample::SampleWindow<nChannels> {
  private:
    DecompressionCache &r_cache;
    uint8_t m_ChannelN;
    enum { PREV, VALUE, ZERO } m_curr;

  public:
    inline StreamCacheWindow(GOSoundStream &stream) : r_cache(stream.cache) {}

    inline void Seek(unsigned index, uint8_t channelN) {
      while (r_cache.position <= index + 1) {
        DecompressionStep(r_cache, nChannels, format16);
      }
      m_ChannelN = channelN;
      m_curr = PREV;
    }

    inline float NextSample() {
      int res;

      if (m_curr == PREV) {
        res = r_cache.prev[m_ChannelN];
        m_curr = VALUE;
      } else if (m_curr == VALUE) {
        res = r_cache.value[m_ChannelN];
        m_curr = ZERO;
      } else
        res = 0;
      return (float)res;
    }
  };

  template <class ResamplerT, class WindowT>
  inline void DecodeBlock(float *pOut, unsigned nOutSamples);

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
