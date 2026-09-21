/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDREVERB_H
#define GOSOUNDREVERB_H

#include <atomic>
#include <string>
#include <vector>

#include "ptrvector.h"

class Convproc;
class GOConfig;
class GOSoundBufferPlanarMutable;

class GOSoundReverb {
public:
  /**
   * @brief Reverb configuration, independent of engine runtime parameters.
   *
   * sampleRate and nSamplesPerBuffer are NOT part of this struct —
   * they are engine-level parameters passed directly to Setup().
   */
  struct ReverbConfig {
    bool isEnabled;
    bool isDirect;
    unsigned channel;
    unsigned startOffset;
    unsigned len;
    unsigned delay;
    float gain;
    std::string file;
  };

  /** Named constant meaning "reverb disabled". Used as default. */
  static const ReverbConfig CONFIG_REVERB_DISABLED;

  /** Creates a ReverbConfig from GOConfig. */
  static ReverbConfig createReverbConfig(const GOConfig &config);

  /**
   * The loaded and pre-processed impulse-response data for a ReverbConfig:
   * gain-applied, offset/length-trimmed, and resampled to the target sample
   * rate - exactly the bytes Convproc::impdata_create() needs, with no file
   * I/O left to do. Returned by loadIRData(); consumed by Setup() and by
   * sound/effects/GOSoundReverbProcessor's EnsureSetup()/CreateTypedState()
   * split (Stage 5).
   */
  struct IRData {
    /** Impulse response samples, already offset-trimmed, gain-applied, and
     * resampled to the target sample rate. */
    std::vector<float> data;
    /** Playback delay, in samples at the target sample rate. */
    unsigned delay;
    /** Whether to inject a direct (dry) impulse at index 0, ahead of data. */
    bool isDirect;
  };

  /**
   * Loads and pre-processes an impulse-response WAV file for config,
   * resampling it to sampleRate if needed. All file I/O and resampling -
   * everything expensive - happens here; the result is cheap to replay into
   * any number of Convproc instances via impdata_create().
   * @throws wxString on any load error (missing/invalid file, out of
   *   memory, resampling failure) - callers handle this the same way
   *   Setup() does below.
   */
  static IRData loadIRData(const ReverbConfig &config, unsigned sampleRate);

private:
  unsigned m_channels;
  // the frame count Process() must be called with, matching what was passed
  // to Convproc::configure() in Setup() - Process() asserts against this
  unsigned m_FramesPerBuffer = 0;
  ptr_vector<Convproc> m_engine;
  // Whether Process() has run the convolution engine since the last Reset():
  // a caller cannot otherwise tell if a decaying tail is still buffered
  // inside Convproc, since it exposes no "am I silent yet" query.
  std::atomic_bool m_HasContent{false};

  void Cleanup();

public:
  GOSoundReverb(unsigned channels);
  virtual ~GOSoundReverb();

  void Reset();
  void Setup(
    const ReverbConfig &config,
    unsigned nSamplesPerBuffer,
    unsigned sampleRate);

  /** @return whether Process() has run the convolution engine since the
   * last Reset() - i.e. whether a caller-visible tail may still be
   * buffered. Conservative: never cleared just because the tail has
   * numerically decayed to silence. */
  bool HasContent() const { return m_HasContent.load(); }

  /**
   * Runs the convolution reverb in place over a planar output buffer, one
   * channel of the Convproc engine per channel of the buffer.
   * @param buffer The output buffer to convolve in place
   */
  void Process(GOSoundBufferPlanarMutable &buffer);
};

#endif
