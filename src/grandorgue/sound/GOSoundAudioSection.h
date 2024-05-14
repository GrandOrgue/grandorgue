/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDAUDIOSECTION_H_
#define GOSOUNDAUDIOSECTION_H_

#include <assert.h>
#include <math.h>

#include "GOBool3.h"
#include "GOInt.h"
#include "GOSoundCompress.h"
#include "GOSoundDefs.h"
#include "GOSoundResample.h"
#include "GOWave.h"

class GOSoundAudioSection;
class GOCache;
class GOCacheObject;
class GOCacheWriter;
class GOLoaderFilename;
class GOMemoryPool;
class GOSoundReleaseAlignTable;
class GOSampleStatistic;

struct audio_section_stream_s;

typedef void (*DecodeBlockFunction)(
  audio_section_stream_s *stream, float *output, unsigned int n_blocks);

typedef struct audio_start_data_segment_s {
  /* Sample offset into entire audio section where data begins. */
  unsigned start_offset;

  DecompressionCache cache;

} audio_start_data_segment;

typedef struct audio_end_data_segment_s {
  /* Sample offset where the loop ends and needs to jump into the next
   * start segment. */
  unsigned end_offset;

  /* Sample offset where the uncompressed end data blob begins (must be less
   * than end_offset). */
  unsigned transition_offset;

  /* Uncompressed ending data blob. This data must start before sample_offset*/
  unsigned char *end_data;
  unsigned char *end_ptr;
  unsigned read_end;
  unsigned end_pos;
  unsigned end_size;
  unsigned end_loop_length;

  /* Index of the next section segment to be played (-1 indicates the
   * end of the blob. */
  int next_start_segment_index;

} audio_end_data_segment;

typedef struct audio_section_stream_s {
  const GOSoundAudioSection *audio_section;
  const struct resampler_coefs_s *resample_coefs;

  /* Method used to decode stream */
  DecodeBlockFunction decode_call;

  /* Cached method used to decode the data in end_ptr. This is just the
   * uncompressed version of decode_call */
  DecodeBlockFunction end_decode_call;

  /* Used for both compressed and uncompressed decoding */
  const unsigned char *ptr;

  /* Derived from the start and end audio segments which were used to setup
   * this stream. */
  const audio_end_data_segment *end_seg;
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
} audio_section_stream;

class GOSoundAudioSection {
private:
  template <class T>
  static void MonoUncompressedLinear(
    audio_section_stream *stream, float *output, unsigned int n_blocks);
  template <class T>
  static void StereoUncompressedLinear(
    audio_section_stream *stream, float *output, unsigned int n_blocks);
  template <class T>
  static void MonoUncompressedPolyphase(
    audio_section_stream *stream, float *output, unsigned int n_blocks);
  template <class T>
  static void StereoUncompressedPolyphase(
    audio_section_stream *stream, float *output, unsigned int n_blocks);
  template <bool format16>
  static void MonoCompressedLinear(
    audio_section_stream *stream, float *output, unsigned int n_blocks);
  template <bool format16>
  static void StereoCompressedLinear(
    audio_section_stream *stream, float *output, unsigned int n_blocks);

  static DecodeBlockFunction GetDecodeBlockFunction(
    unsigned channels,
    unsigned bits_per_sample,
    bool compressed,
    interpolation_type interpolation,
    bool is_end);
  static unsigned GetMargin(bool compressed, interpolation_type interpolation);

  void Compress(bool format16);

  unsigned PickEndSegment(unsigned start_segment_index) const;

  void GetMaxAmplitudeAndDerivative();

  void DoCrossfade(
    unsigned char *dest,
    unsigned dest_offset,
    const unsigned char *src,
    unsigned src_offset,
    unsigned channels,
    unsigned bits_per_sample,
    unsigned fade_length,
    unsigned loop_length,
    unsigned length);

  std::vector<audio_start_data_segment> m_StartSegments;
  std::vector<audio_end_data_segment> m_EndSegments;

  /* Pointer to (size) bytes of data encoded in the format (type) */
  unsigned char *m_Data;

  /* If this is a release section, it may contain an alignment table */
  GOSoundReleaseAlignTable *m_ReleaseAligner;
  unsigned m_ReleaseStartSegment;

  /* Number of significant bits in the decoded sample data */
  unsigned m_SampleFracBits;

  unsigned m_SampleCount;
  unsigned m_SampleRate;

  /* Type of the data which is stored in the data pointer */
  unsigned m_BitsPerSample;
  unsigned m_BytesPerSample;
  unsigned m_Channels;

  GOBool3 m_WaveTremulantStateFor;
  bool m_IsCompressed;

  /* Size of the section in BYTES */
  GOMemoryPool &m_Pool;
  unsigned m_AllocSize;

  unsigned m_MaxAmplitude;
  int m_MaxAbsAmplitude;
  int m_MaxAbsDerivative;
  unsigned m_ReleaseCrossfadeLength; // in ms

public:
  GOSoundAudioSection(GOMemoryPool &pool);
  ~GOSoundAudioSection();
  void ClearData();
  inline unsigned GetChannels() const { return m_Channels; }
  inline GOBool3 GetWaveTremulantStateFor() const {
    return m_WaveTremulantStateFor;
  }

  unsigned GetBytesPerSample() const;
  unsigned GetLength() const;
  unsigned GetReleaseCrossfadeLength() const {
    return m_ReleaseCrossfadeLength;
  }

  bool LoadCache(GOCache &cache);
  bool SaveCache(GOCacheWriter &cache) const;

  /* Initialize a stream to play this audio section and seek into it using
   * release alignment if available. */
  void InitAlignedStream(
    audio_section_stream *stream,
    const audio_section_stream *existing_stream) const;

  /* Initialize a stream to play this audio section */
  void InitStream(
    const struct resampler_coefs_s *resampler_coefs,
    audio_section_stream *stream,
    float sample_rate_adjustment) const;

  /* Read an audio buffer from an audio section stream */
  static bool ReadBlock(
    audio_section_stream *stream, float *buffer, unsigned int n_blocks);
  static void GetHistory(
    const audio_section_stream *stream,
    int history[BLOCK_HISTORY][MAX_OUTPUT_CHANNELS]);

  void Setup(
    const GOCacheObject *pObjectFor,
    const GOLoaderFilename *pLoaderFilename,
    const void *pcm_data,
    GOWave::SAMPLE_FORMAT pcm_data_format,
    unsigned pcm_data_channels,
    unsigned pcm_data_sample_rate,
    unsigned pcm_data_nb_samples,
    const std::vector<GOWaveLoop> *loop_points,
    GOBool3 waveTremulantStateFor,
    bool compress,
    unsigned loopCrossfadeLength,
    unsigned releaseCrossfadeLength);

  bool IsOneshot() const;

  static int GetSampleData(
    unsigned position,
    unsigned channel,
    unsigned bits_per_sample,
    unsigned channels,
    const unsigned char *data);
  static void SetSampleData(
    unsigned position,
    unsigned channel,
    unsigned bits_per_sample,
    unsigned channels,
    unsigned value,
    unsigned char *data);

  int GetSample(
    unsigned position,
    unsigned channel,
    DecompressionCache *cache = NULL) const;

  float GetNormGain() const;
  unsigned GetSampleRate() const;
  bool SupportsStreamAlignment() const;
  void SetupStreamAlignment(
    const std::vector<const GOSoundAudioSection *> &joinables,
    unsigned start_index);

  GOSampleStatistic GetStatistic();
};

inline unsigned GOSoundAudioSection::GetBytesPerSample() const {
  return (m_IsCompressed) ? 0 : (m_BitsPerSample / 8);
}

inline unsigned GOSoundAudioSection::GetLength() const { return m_SampleCount; }

inline bool GOSoundAudioSection::IsOneshot() const {
  return (m_EndSegments.size() == 1)
    && (m_EndSegments[0].next_start_segment_index < 0);
}

inline int GOSoundAudioSection::GetSampleData(
  unsigned position,
  unsigned channel,
  unsigned bits_per_sample,
  unsigned channels,
  const unsigned char *sample_data) {
  if (bits_per_sample <= 8) {
    GOInt8 *data = (GOInt8 *)sample_data;
    return data[position * channels + channel];
  }
  if (bits_per_sample <= 16) {
    GOInt16 *data = (GOInt16 *)sample_data;
    return data[position * channels + channel];
  }
  if (bits_per_sample <= 24) {
    GOInt24 *data = (GOInt24 *)sample_data;
    return data[position * channels + channel];
  }
  assert(0 && "broken sampler type");
  return 0;
}

inline int GOSoundAudioSection::GetSample(
  unsigned position, unsigned channel, DecompressionCache *cache) const {
  if (!m_IsCompressed) {
    return GetSampleData(
      position, channel, m_BitsPerSample, m_Channels, m_Data);
  } else {
    DecompressionCache tmp;
    if (!cache) {
      cache = &tmp;
      InitDecompressionCache(*cache);
    }

    assert(m_BitsPerSample >= 12);
    DecompressTo(*cache, position, m_Data, m_Channels, (m_BitsPerSample >= 20));
    return cache->value[channel];
  }
}

inline void GOSoundAudioSection::SetSampleData(
  unsigned position,
  unsigned channel,
  unsigned bits_per_sample,
  unsigned channels,
  unsigned value,
  unsigned char *sample_data) {
  if (bits_per_sample <= 8) {
    GOInt8 *data = (GOInt8 *)sample_data;
    data[position * channels + channel] = value;
    return;
  }
  if (bits_per_sample <= 16) {
    GOInt16 *data = (GOInt16 *)sample_data;
    data[position * channels + channel] = value;
    return;
  }
  if (bits_per_sample <= 24) {
    GOInt24 *data = (GOInt24 *)sample_data;
    data[position * channels + channel] = value;
    return;
  }
  assert(0 && "broken sampler type");
}

inline float GOSoundAudioSection::GetNormGain() const {
  return scalbnf(1.0f, -((int)m_SampleFracBits));
}

inline bool GOSoundAudioSection::SupportsStreamAlignment() const {
  return (m_ReleaseAligner != NULL);
}

#endif /* GOSOUNDAUDIOSECTION_H_ */
