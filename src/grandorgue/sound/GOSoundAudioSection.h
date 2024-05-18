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

class GOSoundAudioSection {
public:
  struct StartSegment {
    /* Sample offset into entire audio section where data begins. */
    unsigned start_offset;

    DecompressionCache cache;
  };

  struct EndSegment {
    /* Sample offset where the loop ends and needs to jump into the next
     * start segment. */
    unsigned end_offset;

    /* Sample offset where the uncompressed end data blob begins (must be less
     * than end_offset). */
    unsigned transition_offset;

    /* Uncompressed ending data blob. This data must start before
     * sample_offset*/
    unsigned char *end_data;
    unsigned char *end_ptr;
    unsigned read_end;
    unsigned end_pos;
    unsigned end_size;
    unsigned end_loop_length;

    /* Index of the next section segment to be played (-1 indicates the
     * end of the blob. */
    int next_start_segment_index;
  };

  struct Stream;

private:
  typedef void (*DecodeBlockFunction)(
    Stream *stream, float *output, unsigned int n_blocks);

public:
  struct Stream {
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
    const EndSegment *end_seg;
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
  };

private:
  template <class T>
  static void MonoUncompressedLinear(
    Stream *stream, float *output, unsigned int n_blocks);
  template <class T>
  static void StereoUncompressedLinear(
    Stream *stream, float *output, unsigned int n_blocks);
  template <class T>
  static void MonoUncompressedPolyphase(
    Stream *stream, float *output, unsigned int n_blocks);
  template <class T>
  static void StereoUncompressedPolyphase(
    Stream *stream, float *output, unsigned int n_blocks);
  template <bool format16>
  static void MonoCompressedLinear(
    Stream *stream, float *output, unsigned int n_blocks);
  template <bool format16>
  static void StereoCompressedLinear(
    Stream *stream, float *output, unsigned int n_blocks);

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
    unsigned fade_length,
    unsigned loop_length,
    unsigned length);

  std::vector<StartSegment> m_StartSegments;
  std::vector<EndSegment> m_EndSegments;

  /* Pointer to (size) bytes of data encoded in the format (type) */
  unsigned char *m_data;

  /* If this is a release section, it may contain an alignment table */
  GOSoundReleaseAlignTable *m_ReleaseAligner;
  unsigned m_ReleaseStartSegment;

  /* Number of significant bits in the decoded sample data */
  unsigned m_SampleFracBits;

  unsigned m_SampleCount;
  unsigned m_SampleRate;

  /* Type of the data which is stored in the data pointer */
  uint8_t m_BitsPerSample;
  uint8_t m_BytesPerSample;
  uint8_t m_channels;

  GOBool3 m_WaveTremulantStateFor;
  bool m_IsCompressed;

  /* Size of the section in BYTES */
  GOMemoryPool &m_Pool;
  unsigned m_AllocSize;

  unsigned m_MaxAmplitude;
  int m_MaxAbsAmplitude;
  int m_MaxAbsDerivative;
  unsigned m_ReleaseCrossfadeLength; // in ms

  void ClearData();

  template <typename T>
  inline static int getSampleData(
    const T *data, unsigned position, uint8_t channels, uint8_t channel) {
    return data[position * channels + channel];
  }

  inline static int getSampleData(
    const unsigned char *sampleData,
    unsigned position,
    uint8_t channels,
    uint8_t channel,
    uint8_t bitsPerSample) {
    int res;

    if (bitsPerSample <= 8)
      res = getSampleData(
        (const GOInt8 *)sampleData, position, channels, channel);
    else if (bitsPerSample <= 16)
      res = getSampleData(
        (const GOInt16 *)sampleData, position, channels, channel);
    else if (bitsPerSample <= 24)
      res = getSampleData(
        (const GOInt24 *)sampleData, position, channels, channel);
    else {
      assert(0 && "broken sampler type");
      res = 0;
    }
    return res;
  }

  inline int GetSampleData(
    const unsigned char *sampleData, unsigned position, uint8_t channel) const {
    return getSampleData(
      sampleData, position, m_channels, channel, m_BitsPerSample);
  }

  template <typename T>
  inline static void setSampleData(
    T *data, unsigned position, uint8_t channels, uint8_t channel, int value) {
    data[position * channels + channel] = value;
  }

  inline static void setSampleData(
    unsigned char *sampleData,
    unsigned position,
    uint8_t channels,
    uint8_t channel,
    uint8_t bitsPerSample,
    int value) {
    if (bitsPerSample <= 8)
      setSampleData((GOInt8 *)sampleData, position, channels, channel, value);
    else if (bitsPerSample <= 16)
      setSampleData((GOInt16 *)sampleData, position, channels, channel, value);
    else if (bitsPerSample <= 24)
      setSampleData((GOInt24 *)sampleData, position, channels, channel, value);
    else
      assert(0 && "broken sampler type");
  }

  inline void SetSampleData(
    unsigned char *sampleData, unsigned position, uint8_t channel, int value) {
    setSampleData(
      sampleData, position, m_channels, channel, m_BitsPerSample, value);
  }

public:
  GOSoundAudioSection(GOMemoryPool &pool);
  ~GOSoundAudioSection() { ClearData(); }

  inline uint8_t GetChannels() const { return m_channels; }

  inline GOBool3 GetWaveTremulantStateFor() const {
    return m_WaveTremulantStateFor;
  }

  inline unsigned GetLength() const { return m_SampleCount; }

  unsigned GetReleaseCrossfadeLength() const {
    return m_ReleaseCrossfadeLength;
  }

  bool LoadCache(GOCache &cache);
  bool SaveCache(GOCacheWriter &cache) const;

  /* Initialize a stream to play this audio section and seek into it using
   * release alignment if available. */
  void InitAlignedStream(Stream *stream, const Stream *existing_stream) const;

  /* Initialize a stream to play this audio section */
  void InitStream(
    const struct resampler_coefs_s *resampler_coefs,
    Stream *stream,
    float sample_rate_adjustment) const;

  /* Read an audio buffer from an audio section stream */
  static bool ReadBlock(Stream *stream, float *buffer, unsigned int n_blocks);
  static void GetHistory(
    const Stream *stream, int history[BLOCK_HISTORY][MAX_OUTPUT_CHANNELS]);

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

  inline bool IsOneshot() const {
    return (m_EndSegments.size() == 1)
      && (m_EndSegments[0].next_start_segment_index < 0);
  }

  inline int GetSample(
    unsigned position,
    unsigned channel,
    DecompressionCache *cache = nullptr) const {
    if (!m_IsCompressed) {
      return GetSampleData(m_data, position, channel);
    } else {
      DecompressionCache tmp;
      if (!cache) {
        cache = &tmp;
        InitDecompressionCache(*cache);
      }

      assert(m_BitsPerSample >= 12);
      DecompressTo(
        *cache, position, m_data, m_channels, (m_BitsPerSample >= 20));
      return cache->value[channel];
    }
  }

  inline float GetNormGain() const {
    return scalbnf(1.0f, -((int)m_SampleFracBits));
  }

  inline unsigned GetSampleRate() const { return m_SampleRate; }

  inline bool SupportsStreamAlignment() const { return (m_ReleaseAligner); }

  void SetupStreamAlignment(
    const std::vector<const GOSoundAudioSection *> &joinables,
    unsigned start_index);

  GOSampleStatistic GetStatistic();
};

#endif /* GOSOUNDAUDIOSECTION_H_ */
