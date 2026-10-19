/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTREMULANT_H
#define GOTREMULANT_H

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>
#include <wx/string.h>

#include "GOCacheObject.h"
#include "GODrawstop.h"
#include "GOParamRange.h"

class GOConfigReader;
class GOConfigWriter;
class GOMemoryPool;
class GOSoundSamplerPlayerInterface;
class GOSoundProvider;
class GOSoundTremulantProvider;
struct GOSoundSampler;

typedef enum { GOSynthTrem, GOWavTrem } GOTremulantType;

class GOTremulant : public GODrawstop, private GOCacheObject {
public:
  // The allowed ranges of the parameters of a Synth tremulant, both ends
  // included. Defined in the .cpp so that this header does not include the
  // sound engine headers that the upper bound of the pitch depth comes from
  static const GOParamRange<int> PERIOD_MS_RANGE;
  static const GOParamRange<int> START_TIME_MS_RANGE;
  static const GOParamRange<int> STOP_TIME_MS_RANGE;
  static const GOParamRange<int> AMP_MOD_DEPTH_PCNT_RANGE;
  static const GOParamRange<float> PITCH_MOD_DEPTH_CENTS_RANGE;
  static const GOParamRange<int> PITCH_MOD_PHASE_DEGREES_RANGE;

  /**
   * The wave form of a tremulant as one period, in frames of a given sample
   * rate. Built by Render() from the parameters; read by GOSoundTremulantTask.
   * Each of ampDeviation and pitchDeviation is either empty (no such
   * modulation) or nPeriodFrames long.
   */
  struct Description {
    // the length of one period in frames: the length of every loop below
    unsigned nPeriodFrames;
    // the length of the depth ramp on engaging / disengaging, in frames
    unsigned nStartFrames;
    unsigned nStopFrames;
    // relative deviation of the level around 0: the gain is 1 + ampDeviation;
    // its mean is honoured (a non-zero mean changes the level); empty = no
    // amplitude modulation (the gain is 1)
    std::vector<float> ampDeviation;
    // relative deviation of the frequency df/f around 0: the ratio is
    // 1 + pitchDeviation; empty = no pitch modulation; the mean does not matter
    std::vector<float> pitchDeviation;
  };

  /**
   * The changes of the parameters of a Synth tremulant. A value is present
   * only if the parameter is to be changed.
   */
  struct SynthParamChanges {
    std::optional<int> periodMs;
    std::optional<int> ampModDepthPcnt;
    std::optional<float> pitchModDepthCents;
    std::optional<int> pitchModPhaseDegrees;
    std::optional<int> startTimeMs;
    std::optional<int> stopTimeMs;
  };

private:
  GOSoundSamplerPlayerInterface &r_sound;

  /**
   * Whether the tremulant is defined in the ODF. Its parameters then come only
   * from the ODF and are never saved to the .cmb. Otherwise it is an internal
   * tremulant and its parameters come only from the .cmb.
   */
  bool m_IsOdfDefined;

  GOTremulantType m_TremulantType;
  // The period of the modulation, in ms. Shared by Synth and Lfo
  int m_PeriodMs;
  int m_StartRate;
  int m_StopRate;
  // The lengths of the depth ramps on engaging / disengaging, in ms; 0 means an
  // instant ramp. Shared by Synth and Lfo
  int m_StartTimeMs;
  int m_StopTimeMs;
  // Synth only (an Lfo tremulant takes the depth from its wave file): the
  // amplitude modulation depth, percent
  int m_AmpModDepthPcnt;
  // Synth only: the depth of the pitch modulation (vibrato), in cents: how far
  // below the nominal pitch the lowest point of the vibrato lies (the highest
  // point lies slightly less above it); up to
  // PITCH_MOD_DEPTH_CENTS_RANGE.m_max; 0 means no pitch modulation. A deep or
  // slow modulation is limited by the delay ring of GOSoundVibratoProcessor, so
  // the effective depth may be smaller
  float m_PitchModDepthCents;
  // Synth only (the file of an Lfo tremulant carries its own phase relation):
  // the pitch phase against the amplitude, in degrees, in -180 .. 359 (a
  // negative value is equivalent to +360); positive: the pitch leads
  int m_PitchModPhaseDegrees;
  // Renders the loop of a Synth tremulant; nullptr for a Wave tremulant
  std::unique_ptr<GOSoundTremulantProvider> o_provider;
  GOSoundProvider *m_TremProvider;
  GOSoundSampler *m_PlaybackHandle;
  uint64_t m_LastStop;
  unsigned m_TremulantN;

  // Marks the organ model as modified. Defined in the .cpp so that this header
  // does not include GOOrganModel.h
  void SetOrganModified();

  /**
   * Reads the parameters of an internal tremulant from the .cmb. A missing key
   * gets its default value. Called only from Init().
   */
  void LoadFromCmb(GOConfigReader &cfg);

  void InitSoundProvider(GOMemoryPool &pool);
  void OnDrawstopStateChanged(bool on) override;
  void SetupIsToStoreInCmb() override;

  void Initialize() override;
  void LoadData(const GOFileStore &fileStore, GOMemoryPool &pool) override;
  bool LoadCache(GOMemoryPool &pool, GOCache &cache) override;
  bool SaveCache(GOCacheWriter &cache) const override { return true; }
  void UpdateHash(GOHash &hash) const override {}
  const wxString &GetLoadTitle() const override { return GetName(); };

  void AbortPlayback() override;
  void StartPlayback() override;
  void Save(GOConfigWriter &cfg) override;

public:
  GOTremulant(GOOrganModel &organModel);
  ~GOTremulant();

  /** @return whether the tremulant is defined in the ODF */
  bool IsOdfDefined() const { return m_IsOdfDefined; }

  /**
   * Initialises an internal (not defined in the ODF) tremulant: a Synth one
   * with the default parameters, which the .cmb may override.
   */
  void Init(
    GOConfigReader &cfg, const wxString &group, const wxString &name) override;

  using GODrawstop::Load; // Avoiding a compilation warning
  void Load(GOConfigReader &cfg, const wxString &group, unsigned tremulantN);
  GOTremulantType GetTremulantType() const;

  /**
   * Renders the loop and the ramp lengths for a sample rate. Called only from
   * the control thread; reads the parameters written by Load(), Init(),
   * ApplySynthParamChanges() and ResetSynthParamsToDefault(), which also run
   * only in the control thread. The audio thread never reads them.
   * Must not be called for a Wave tremulant.
   *
   * @param sampleRate the sample rate of the sound engine, in Hz
   * @return the rendered parameters of the wave form
   */
  Description Render(unsigned sampleRate) const;

  // The parameters are changed only by ApplySynthParamChanges() and
  // ResetSynthParamsToDefault(), which mark the organ model as modified. The
  // sound changes when the wave form is rendered again
  // (GOSoundOrganEngine::PrepareTremulantWaveForms)
  int GetPeriodMs() const { return m_PeriodMs; }
  int GetStartTimeMs() const { return m_StartTimeMs; }
  int GetStopTimeMs() const { return m_StopTimeMs; }
  int GetAmpModDepthPcnt() const { return m_AmpModDepthPcnt; }
  float GetPitchModDepthCents() const { return m_PitchModDepthCents; }
  int GetPitchModPhaseDegrees() const { return m_PitchModPhaseDegrees; }

  /**
   * Applies the changes to the parameters of a Synth tremulant and marks the
   * organ model as modified if anything has changed. A value outside its range
   * is clamped to it.
   *
   * Only a Synth tremulant is allowed: a debug build fails an assertion
   * otherwise; a release build changes nothing and returns false.
   * @param changes the parameters to change
   * @return whether any parameter has really changed
   */
  bool ApplySynthParamChanges(const SynthParamChanges &changes);

  /**
   * Sets the parameters of a Synth tremulant to their default values and marks
   * the organ model as modified if anything has changed. Only a Synth
   * tremulant is allowed, like ApplySynthParamChanges().
   * @return whether any parameter has really changed
   */
  bool ResetSynthParamsToDefault();
};

#endif /* GOTREMULANT_H_ */
