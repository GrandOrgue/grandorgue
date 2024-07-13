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
  static const unsigned getMaxReadAhead();

  struct StartSegment {
    /* Sample offset into entire audio section where data begins. */
    unsigned start_offset;

    DecompressionCache cache;
  };

  /**
   * This segment contains copy of samples from a loop end and then from the
   * loop start. It is necessary for read-ahead when resampling
   */
  struct EndSegmentDescription {
    /* A position after the last copied sample in the end segment.
     * An end segment has more MAX_READAHEAD samples after end_pos:
     *     - A loop end segment contains a copy of samples from the loop start
     *     - A release end segment contains zero samples
     */
    unsigned end_pos;

    /* Sample offset where the uncompressed end data blob begins (must be less
     * than end_pos). */
    unsigned transition_offset;

    // The size of the end data in bytes
    unsigned end_size;

    /* Index of the next section segment to be played (-1 indicates the
     * end of the blob. */
    int next_start_segment_index;
  };

  struct EndSegment : public EndSegmentDescription {
    /* Uncompressed ending data blob. This data must start before
     * sample_offset*/
    unsigned char *end_data;

    // A virtual pointer to end_data. end_data has transition_offset to end_ptr
    unsigned char *end_ptr;
  };

private:
  void Compress(bool format16);

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
  inline static unsigned limitedDiff(unsigned a, unsigned b) {
    return (a > b) ? a - b : 0;
  }

  GOSoundAudioSection(GOMemoryPool &pool);
  ~GOSoundAudioSection() { ClearData(); }

  GOSoundReleaseAlignTable *GetReleaseAligner() const {
    return m_ReleaseAligner;
  }
  unsigned GetReleaseStartSegment() const { return m_ReleaseStartSegment; }
  unsigned GetSampleRate() const { return m_SampleRate; }
  uint8_t GetBitsPerSample() const { return m_BitsPerSample; }
  uint8_t GetBytesPerSample() const { return m_BytesPerSample; }
  inline uint8_t GetChannels() const { return m_channels; }
  bool IsCompressed() const { return m_IsCompressed; }

  inline GOBool3 GetWaveTremulantStateFor() const {
    return m_WaveTremulantStateFor;
  }

  inline unsigned GetLength() const { return m_SampleCount; }

  unsigned GetReleaseCrossfadeLength() const {
    return m_ReleaseCrossfadeLength;
  }

  const StartSegment &GetStartSegment(unsigned index) const {
    return m_StartSegments[index];
  }

  const EndSegment &GetEndSegment(unsigned index) const {
    return m_EndSegments[index];
  }

  const unsigned char *GetData() const { return m_data; }

  inline int GetSampleData(
    const unsigned char *sampleData, unsigned position, uint8_t channel) const {
    return getSampleData(
      sampleData, position, m_channels, channel, m_BitsPerSample);
  }

  bool LoadCache(GOCache &cache);
  bool SaveCache(GOCacheWriter &cache) const;

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

  unsigned PickEndSegment(unsigned start_segment_index) const;

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

  inline bool SupportsStreamAlignment() const { return (m_ReleaseAligner); }

  void SetupStreamAlignment(
    const std::vector<const GOSoundAudioSection *> &joinables,
    unsigned start_index);

  GOSampleStatistic GetStatistic();
};

#endif /* GOSOUNDAUDIOSECTION_H_ */
