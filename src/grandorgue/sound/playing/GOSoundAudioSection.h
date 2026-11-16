/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDAUDIOSECTION_H_
#define GOSOUNDAUDIOSECTION_H_

#include <algorithm>
#include <assert.h>
#include <limits>
#include <math.h>

#include "sound/dsp-kernels/GOSoundResample.h"

#include "GOBool3.h"
#include "GOFloat32.h"
#include "GOInt.h"
#include "GOSoundCompressionCache.h"
#include "GOSoundItemAccessor.h"
#include "GOWave.h"

class GOSoundAudioSection;
class GOCache;
class GOCacheObject;
class GOCacheWriter;
class GOLoaderFilename;
class GOMemoryPool;
class GOSoundReleaseAlignTable;
class GOSampleStatistic;

// The float sections store 32-bit IEEE floats and the compressed or Q23 samples
// are held in 32-bit ints
static_assert(
  sizeof(float) == 4 && std::numeric_limits<float>::is_iec559,
  "float must be a 32-bit IEEE float");
static_assert(sizeof(int) == 4, "int must be 32 bits");

class GOSoundAudioSection {
public:
  /* A section whose max single-channel amplitude never exceeds this many raw
   * PCM units (regardless of bit depth, since 1 LSB is always 1 in these raw
   * units) is considered essentially silent, e.g. a "BlankLoop" placeholder
   * attack containing only quantization noise. */
  static constexpr unsigned SILENT_AMPLITUDE_THRESHOLD = 4;

  /** Minimum number of stored frames starting at an end segment's end_pos. */
  static constexpr unsigned MAX_READAHEAD = GOSoundResample::MAX_POINTS;

  static const unsigned getMaxReadAhead();

  /**
   * Zero frames stored before the first PCM frame. The initial window starts
   * PROLOGUE_LENGTH - CENTRE_POINT frames in, so its centre is at PCM frame 0.
   */
  static constexpr unsigned PROLOGUE_LENGTH = GOSoundResample::MAX_CENTRE_POINT;

  /**
   * Bumped when the cache content or layout of a section changes without
   * changing sizeof(), because GOOrganController::GenerateCacheHash() relies on
   * sizeof() to detect layout changes. It is added to the hash so that the
   * caches created before such a change are not used.
   *
   * 1: the 8-bit samples are decoded as offset binary
   * 2: the sections of 32-bit float wave files are stored as floats and the
   *    m_IsFloat flag is serialized
   */
  static constexpr unsigned CACHE_VERSION = 2;

  struct StartSegment {
    /* Sample offset into entire audio section where data begins. */
    unsigned start_offset;

    GOSoundCompressionCache cache;
  };

  /**
   * This segment contains copy of samples from a loop end and then from the
   * loop start. It is necessary for read-ahead when resampling
   */
  struct EndSegmentDescription {
    /* The position after the last frame played from the end segment.
     * Starting at end_pos, the segment stores at least MAX_READAHEAD frames:
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
  /* The items passed to Setup() were IEEE floats rather than integers. The
   * stored bytes of a compressed float section are int32 raw units, so use
   * IsFloatData() to decide how to interpret them. */
  bool m_IsFloat;

  /* Size of the section in BYTES */
  GOMemoryPool &m_Pool;
  unsigned m_AllocSize;

  /* Max absolute value of any single channel, in raw PCM units at this
   * section's own m_BitsPerSample (not normalized across sections). */
  unsigned m_MaxAmplitude;
  unsigned m_MaxAbsAmplitude;
  unsigned m_MaxAbsDerivative;
  unsigned m_ReleaseCrossfadeLength; // in ms

  void ClearData();

  /**
   * Converts the end segments of a float section from floats to int32 raw
   * units at 2^-23 FS. It must be called only after Compress() succeeded: the
   * main block is int32 then, and the stream reads the main block and the end
   * segments with one gain.
   */
  void ConvertEndSegmentsToRawUnits();

  /**
   * Copies a range of section frames from PCM and the leading zero prologue.
   * @param pPcm the original PCM starting at its first sample frame
   * @param startFrame the first frame in padded section coordinates
   * @param nFrames how many frames to copy
   * @param outPData the destination for the copied frames
   */
  void CopyPaddedFrames(
    const unsigned char *pPcm,
    unsigned startFrame,
    unsigned nFrames,
    unsigned char *outPData) const;

  /**
   * Updates m_MaxAmplitude, m_MaxAbsAmplitude and m_MaxAbsDerivative.
   * @tparam FGetItem callable (unsigned frameI, unsigned channelI) -> int
   * @param fGetItem reads one item in raw units
   */
  template <typename FGetItem> void scanFrames(FGetItem &&fGetItem);

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

  /**
   * Whether the stored sample items are IEEE floats rather than integers. A
   * 32-bit section stores floats when it is uncompressed and int32 raw units
   * at 2^-23 FS when it is compressed.
   */
  bool IsFloatData() const { return m_IsFloat && !m_IsCompressed; }

  inline GOBool3 GetWaveTremulantStateFor() const {
    return m_WaveTremulantStateFor;
  }

  /** Length of the stored PCM in frames, excluding the zero prologue. */
  inline unsigned GetLength() const {
    return limitedDiff(m_SampleCount, PROLOGUE_LENGTH);
  }

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

  /**
   * @return a new accessor of the items of the stored data (m_data and the end
   *   segments). The accessor is not stored in the section because
   *   IsFloatData() changes when the section is compressed.
   */
  inline GOSoundItemAccessor CreateItemAccessor() const {
    return GOSoundItemAccessor(m_BitsPerSample, IsFloatData(), m_channels);
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

  /**
   * Reads one stored sample item in raw units, see GetNormGain().
   * @param position the frame index in the stored data, which starts with
   *   PROLOGUE_LENGTH zero frames: PCM frame k is at PROLOGUE_LENGTH + k
   * @param channel the channel index
   * @param cache the decompression cache of a compressed section; if null, a
   *   temporary one is used
   * @return the item; for an uncompressed float section, the float rounded to
   *   raw units of 2^-23 FS, not the stored float
   */
  inline int GetSample(
    unsigned position,
    unsigned channel,
    GOSoundCompressionCache *cache = nullptr) const {
    if (!m_IsCompressed) {
      return CreateItemAccessor().GetIntItem(m_data, position, channel);
    } else {
      GOSoundCompressionCache tmp;
      if (!cache) {
        cache = &tmp;
        cache->Init();
      }

      assert(m_BitsPerSample >= 12);
      cache->DecompressTo(
        position, m_data, m_channels, (m_BitsPerSample >= 20));
      return cache->m_value[channel];
    }
  }

  /**
   * @return the gain that brings the items read by GOSoundStream to full scale
   * (1.0). It is not the scale of GetSample(), which returns integer raw
   * units: for an uncompressed float section GetSample() is in units of 2^-23
   * FS, while the stream items are the floats themselves and the gain is 1.0.
   */
  inline float GetNormGain() const {
    return scalbnf(1.0f, -((int)m_SampleFracBits));
  }

  inline bool SupportsStreamAlignment() const { return (m_ReleaseAligner); }

  inline bool IsEssentiallySilent() const {
    return m_MaxAmplitude <= SILENT_AMPLITUDE_THRESHOLD;
  }

  void SetupStreamAlignment(
    const std::vector<const GOSoundAudioSection *> &joinables,
    unsigned start_index);

  GOSampleStatistic GetStatistic();
};

#endif /* GOSOUNDAUDIOSECTION_H_ */
