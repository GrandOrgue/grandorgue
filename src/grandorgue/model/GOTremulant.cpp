/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTremulant.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <utility>
#include <wx/intl.h>

#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"
#include "model/GOOrganModel.h"
#include "sound/providers/GOSoundProviderSynthedTrem.h"
#include "sound/providers/GOSoundTremulantProviderSynth.h"

#define DELETE_AND_NULL(x)                                                     \
  do {                                                                         \
    if (x) {                                                                   \
      delete x;                                                                \
      x = NULL;                                                                \
    }                                                                          \
  } while (0)

static const GOConfigEnum TREMULANT_TYPES({
  {wxT("Synth"), GOSynthTrem},
  {wxT("Wave"), GOWavTrem},
});

static const wxString WX_PERIOD_MS = wxT("PeriodMs");
static const wxString WX_START_TIME_MS = wxT("StartTimeMs");
static const wxString WX_STOP_TIME_MS = wxT("StopTimeMs");
static const wxString WX_AMP_MOD_DEPTH_PCNT = wxT("AmpModDepthPcnt");
static const wxString WX_PITCH_MOD_DEPTH_CENTS = wxT("PitchModDepthCents");
static const wxString WX_PITCH_MOD_PHASE_DEGREES = wxT("PitchModPhaseDegrees");

/* The deepest pitch modulation: at 2400 cents k = 1 - 2^-2 = 0.75, so the
 * lowest frequency deviation -0.75 is
 * GOSoundVibratoPitchRateCurve::MIN_DEVIATION (checked by GOTestTremulant:
 * exp2() is not constexpr, so no static_assert) */
static constexpr float MAX_PITCH_MOD_DEPTH_CENTS = 2400.0f;

const GOParamRange<int> GOTremulant::PERIOD_MS_RANGE{32, 441000};
const GOParamRange<int> GOTremulant::START_TIME_MS_RANGE{0, 10000};
const GOParamRange<int> GOTremulant::STOP_TIME_MS_RANGE{0, 10000};
const GOParamRange<int> GOTremulant::AMP_MOD_DEPTH_PCNT_RANGE{0, 100};
const GOParamRange<float> GOTremulant::PITCH_MOD_DEPTH_CENTS_RANGE{
  0, MAX_PITCH_MOD_DEPTH_CENTS};
const GOParamRange<int> GOTremulant::PITCH_MOD_PHASE_DEGREES_RANGE{-180, 359};

// The parameters of a tremulant that is not defined in the ODF
static constexpr int PERIOD_MS_DEFAULT = 200;
static constexpr int START_TIME_MS_DEFAULT = 300;
static constexpr int STOP_TIME_MS_DEFAULT = 300;
static constexpr int AMP_MOD_DEPTH_PCNT_DEFAULT = 50;
static constexpr float PITCH_MOD_DEPTH_CENTS_DEFAULT = 0.0f;
static constexpr int PITCH_MOD_PHASE_DEGREES_DEFAULT = 0;

GOTremulant::GOTremulant(GOOrganModel &organModel)
  : GODrawstop(organModel, OBJECT_TYPE_TREMULANT),
    r_sound(organModel),
    m_IsOdfDefined(false),
    m_TremulantType(GOSynthTrem),
    m_PeriodMs(0),
    m_StartRate(0),
    m_StopRate(0),
    m_StartTimeMs(0),
    m_StopTimeMs(0),
    m_AmpModDepthPcnt(0),
    m_PitchModDepthCents(0.0f),
    m_PitchModPhaseDegrees(0),
    m_TremProvider(NULL),
    m_PlaybackHandle(0),
    m_LastStop(0),
    m_TremulantN(0) {
  r_OrganModel.RegisterCacheObject(this);
}

GOTremulant::~GOTremulant() {
  r_OrganModel.UnRegisterCacheObject(this);
  DELETE_AND_NULL(m_TremProvider);
}

void GOTremulant::Initialize() {}

void GOTremulant::LoadData(const GOFileStore &fileStore, GOMemoryPool &pool) {
  InitSoundProvider(pool);
}

bool GOTremulant::LoadCache(GOMemoryPool &pool, GOCache &cache) {
  InitSoundProvider(pool);
  return true;
}

void GOTremulant::LoadFromCmb(GOConfigReader &cfg) {
  const wxString &group = GODrawstop::m_group;

  m_PeriodMs = cfg.ReadInteger(
    CMBSetting,
    group,
    WX_PERIOD_MS,
    PERIOD_MS_RANGE.m_min,
    PERIOD_MS_RANGE.m_max,
    false,
    PERIOD_MS_DEFAULT);
  m_StartTimeMs = cfg.ReadInteger(
    CMBSetting,
    group,
    WX_START_TIME_MS,
    START_TIME_MS_RANGE.m_min,
    START_TIME_MS_RANGE.m_max,
    false,
    START_TIME_MS_DEFAULT);
  m_StopTimeMs = cfg.ReadInteger(
    CMBSetting,
    group,
    WX_STOP_TIME_MS,
    STOP_TIME_MS_RANGE.m_min,
    STOP_TIME_MS_RANGE.m_max,
    false,
    STOP_TIME_MS_DEFAULT);
  m_AmpModDepthPcnt = cfg.ReadInteger(
    CMBSetting,
    group,
    WX_AMP_MOD_DEPTH_PCNT,
    AMP_MOD_DEPTH_PCNT_RANGE.m_min,
    AMP_MOD_DEPTH_PCNT_RANGE.m_max,
    false,
    AMP_MOD_DEPTH_PCNT_DEFAULT);
  m_PitchModDepthCents = cfg.ReadFloat(
    CMBSetting,
    group,
    WX_PITCH_MOD_DEPTH_CENTS,
    PITCH_MOD_DEPTH_CENTS_RANGE.m_min,
    PITCH_MOD_DEPTH_CENTS_RANGE.m_max,
    false,
    PITCH_MOD_DEPTH_CENTS_DEFAULT);
  m_PitchModPhaseDegrees = cfg.ReadInteger(
    CMBSetting,
    group,
    WX_PITCH_MOD_PHASE_DEGREES,
    PITCH_MOD_PHASE_DEGREES_RANGE.m_min,
    PITCH_MOD_PHASE_DEGREES_RANGE.m_max,
    false,
    PITCH_MOD_PHASE_DEGREES_DEFAULT);
}

/**
 * Assigns a new value to a parameter if it is present, clamped to the range.
 * @param newValue the new value, if the parameter is to be changed
 * @param range the allowed range of the parameter
 * @param ioField the field of the parameter
 * @return whether the value has changed
 */
template <typename T>
static bool assign_param(
  const std::optional<T> &newValue, const GOParamRange<T> &range, T &ioField) {
  bool isChanged = false;

  if (newValue) {
    const T clamped = range.Clamp(*newValue);

    isChanged = clamped != ioField;
    ioField = clamped;
  }
  return isChanged;
}

/**
 * Applies the depth to the deviation, in place: multiplies every item by it.
 * A zero depth needs no special case: the channel is not requested, so its
 * deviation is empty, and it stays empty.
 */
static void apply_depth(float depth, std::vector<float> &ioDeviations) {
  std::transform(
    ioDeviations.begin(),
    ioDeviations.end(),
    ioDeviations.begin(),
    [depth](float value) { return value * depth; });
}

/**
 * Advances the deviation cyclically: every item becomes the item that is
 * nShiftFrames ahead, that is the deviation is shifted to the left. A later
 * feature of the deviation thus comes nShiftFrames frames earlier: it leads.
 * @param nShiftFrames less than the length of the deviation (any if it is
 *   empty)
 */
static void advance_shape(
  unsigned nShiftFrames, std::vector<float> &ioDeviations) {
  if (nShiftFrames > 0 && !ioDeviations.empty()) {
    assert(nShiftFrames < ioDeviations.size());
    std::rotate(
      ioDeviations.begin(),
      ioDeviations.begin() + nShiftFrames,
      ioDeviations.end());
  }
}

/**
 * Converts a time to a number of frames, rounded to the nearest. Unlike
 * GOSoundVibratoProcessor.cpp (rounds up) and GOSoundSamplerPlayer::MsToSamples
 * (truncates) this is deliberate: the loop of a tremulant should be as close
 * as possible to the requested period. It takes a double because the period
 * can be fractional (the natural period of an LFO) and a float loses frames
 * at 441 s.
 */
static unsigned ms_to_frames(double ms, unsigned sampleRate) {
  return (unsigned)std::llround(ms * sampleRate / 1000.0);
}

bool GOTremulant::ApplySynthParamChanges(const SynthParamChanges &changes) {
  const bool isSynth = m_TremulantType == GOSynthTrem;
  bool isChanged = false;

  assert(isSynth);
  /* With correct use the type is always Synth here: this check never fires, it
   * only keeps the call harmless in a release build, where the assert above is
   * compiled out */
  if (isSynth) {
    isChanged = assign_param(changes.periodMs, PERIOD_MS_RANGE, m_PeriodMs);

    isChanged
      = assign_param(
          changes.ampModDepthPcnt, AMP_MOD_DEPTH_PCNT_RANGE, m_AmpModDepthPcnt)
      || isChanged;
    isChanged = assign_param(
                  changes.pitchModDepthCents,
                  PITCH_MOD_DEPTH_CENTS_RANGE,
                  m_PitchModDepthCents)
      || isChanged;
    isChanged = assign_param(
                  changes.pitchModPhaseDegrees,
                  PITCH_MOD_PHASE_DEGREES_RANGE,
                  m_PitchModPhaseDegrees)
      || isChanged;
    isChanged
      = assign_param(changes.startTimeMs, START_TIME_MS_RANGE, m_StartTimeMs)
      || isChanged;
    isChanged
      = assign_param(changes.stopTimeMs, STOP_TIME_MS_RANGE, m_StopTimeMs)
      || isChanged;
    if (isChanged)
      SetOrganModified();
  }
  return isChanged;
}

bool GOTremulant::ResetSynthParamsToDefault() {
  return ApplySynthParamChanges(
    {.periodMs = PERIOD_MS_DEFAULT,
     .ampModDepthPcnt = AMP_MOD_DEPTH_PCNT_DEFAULT,
     .pitchModDepthCents = PITCH_MOD_DEPTH_CENTS_DEFAULT,
     .pitchModPhaseDegrees = PITCH_MOD_PHASE_DEGREES_DEFAULT,
     .startTimeMs = START_TIME_MS_DEFAULT,
     .stopTimeMs = STOP_TIME_MS_DEFAULT});
}

void GOTremulant::Init(
  GOConfigReader &cfg, const wxString &group, const wxString &name) {
  m_IsOdfDefined = false;
  GODrawstop::Init(cfg, group, name);
  m_TremulantType = GOSynthTrem;
  o_provider = std::make_unique<GOSoundTremulantProviderSynth>();
  LoadFromCmb(cfg);
  r_OrganModel.RegisterCacheObject(this);
}

void GOTremulant::Load(
  GOConfigReader &cfg, const wxString &group, unsigned tremulantN) {
  m_IsOdfDefined = true;
  SetGroupAndPrefix(group, wxEmptyString);
  m_TremulantType = (GOTremulantType)cfg.ReadEnum(
    ODFSetting,
    group,
    wxT("TremulantType"),
    TREMULANT_TYPES,
    false,
    GOSynthTrem);
  if (m_TremulantType == GOSynthTrem) {
    m_TremProvider = new GOSoundProviderSynthedTrem();
    o_provider = std::make_unique<GOSoundTremulantProviderSynth>();
    m_PeriodMs = cfg.ReadLong(ODFSetting, group, wxT("Period"), 32, 441000);
    m_StartRate = cfg.ReadInteger(ODFSetting, group, wxT("StartRate"), 1, 100);
    m_StopRate = cfg.ReadInteger(ODFSetting, group, wxT("StopRate"), 1, 100);
    m_StartTimeMs = (int)std::lround(1000.0 / m_StartRate);
    m_StopTimeMs = (int)std::lround(1000.0 / m_StopRate);
    m_AmpModDepthPcnt
      = cfg.ReadInteger(ODFSetting, group, wxT("AmpModDepth"), 1, 100);
    m_TremulantN = tremulantN;
    m_PlaybackHandle = 0;
  }
  GODrawstop::Load(cfg, group);
}

void GOTremulant::SetupIsToStoreInCmb() {
  GODrawstop::SetupIsToStoreInCmb();
  m_IsToStoreInDivisional
    = m_IsToStoreInDivisional && r_OrganModel.DivisionalsStoreTremulants();
}

void GOTremulant::InitSoundProvider(GOMemoryPool &pool) {
  if (m_TremulantType == GOSynthTrem) {
    ((GOSoundProviderSynthedTrem *)m_TremProvider)
      ->Create(pool, m_PeriodMs, m_StartRate, m_StopRate, m_AmpModDepthPcnt);
    assert(!m_TremProvider->IsOneshot());
  }
}

GOTremulant::Description GOTremulant::Render(unsigned sampleRate) const {
  // A Wave tremulant has no provider and is not rendered
  assert(m_TremulantType != GOWavTrem);
  assert(o_provider);

  const unsigned nPeriodFrames = ms_to_frames(m_PeriodMs, sampleRate);
  const unsigned nStartFrames = ms_to_frames(m_StartTimeMs, sampleRate);
  const unsigned nStopFrames = ms_to_frames(m_StopTimeMs, sampleRate);

  assert(nPeriodFrames > 0);

  /* How the deviations are modified: the depths as multipliers and the pitch
   * phase. TODO calculate for lfo; until then an Lfo tremulant gets 0, that is
   * no modulation */
  const bool isSynth = m_TremulantType == GOSynthTrem;
  const float ampFactor = isSynth ? m_AmpModDepthPcnt / 100.0f : 0.0f;
  /* Linear in frequency: the pitch goes down by exactly m_PitchModDepthCents.
   * A zero depth is set to an exact 0, whatever the rounding of exp2() */
  const float pitchFactor = isSynth && m_PitchModDepthCents != 0.0f
    ? 1.0f - std::exp2(-m_PitchModDepthCents / 1200.0f)
    : 0.0f;

  assert(ampFactor >= 0.0f);
  assert(pitchFactor >= 0.0f);

  // A zero factor means no modulation: that deviation is not requested
  GOSoundTremulantProvider::Shapes shapes = o_provider->RenderOnePeriod(
    nPeriodFrames, ampFactor > 0.0f, pitchFactor > 0.0f);

  // The provider must honour its contract: every deviation is empty or one
  // period
  assert(
    shapes.ampDeviation.empty() || shapes.ampDeviation.size() == nPeriodFrames);
  assert(
    shapes.pitchDeviation.empty()
    || shapes.pitchDeviation.size() == nPeriodFrames);

  /* A zero depth has not requested the channel, so its deviation is empty and
   * the multiplication leaves it empty */
  apply_depth(ampFactor, shapes.ampDeviation);
  apply_depth(pitchFactor, shapes.pitchDeviation);

  if (isSynth) {
    /* The phase is in -180 .. 359: a negative value becomes its 0 .. 359
     * equivalent by adding 360 */
    assert(m_PitchModPhaseDegrees >= -180 && m_PitchModPhaseDegrees <= 359);

    const int phase = m_PitchModPhaseDegrees;
    const unsigned phaseDegrees = (unsigned)(phase < 0 ? phase + 360 : phase);
    /* The phase as a shift in frames: a positive phase makes the pitch lead.
     * It is rounded to the nearest frame, which is at most half a frame
     * (180 degrees / nPeriodFrames) off; the modulo keeps it below the period
     */
    const unsigned nPitchShiftFrames
      = (unsigned)(((uint64_t)phaseDegrees * nPeriodFrames + 180) / 360)
      % nPeriodFrames;

    advance_shape(nPitchShiftFrames, shapes.pitchDeviation);
  }

  return {
    .nPeriodFrames = nPeriodFrames,
    .nStartFrames = nStartFrames,
    .nStopFrames = nStopFrames,
    .ampDeviation = std::move(shapes.ampDeviation),
    .pitchDeviation = std::move(shapes.pitchDeviation)};
}

void GOTremulant::OnDrawstopStateChanged(bool on) {
  if (m_TremulantType == GOSynthTrem) {
    if (on) {
      assert(m_TremulantN > 0);
      m_PlaybackHandle = r_OrganModel.StartTremulantSample(
        m_TremProvider, m_TremulantN, m_LastStop);
    } else if (m_PlaybackHandle) {
      m_LastStop = r_OrganModel.StopSample(m_TremProvider, m_PlaybackHandle);
      m_PlaybackHandle = NULL;
    }
  }
  if (m_TremulantType == GOWavTrem)
    r_OrganModel.UpdateTremulant(this);
}

void GOTremulant::AbortPlayback() {
  m_PlaybackHandle = NULL;
  m_LastStop = 0;
  GOButtonControl::AbortPlayback();
}

void GOTremulant::StartPlayback() {
  GODrawstop::StartPlayback();

  if (IsEngaged() && m_TremulantType == GOSynthTrem) {
    assert(m_TremulantN > 0);
    r_sound.StartTremulantSample(m_TremProvider, m_TremulantN, 0);
  }
  if (m_TremulantType == GOWavTrem)
    r_OrganModel.UpdateTremulant(this);
}

GOTremulantType GOTremulant::GetTremulantType() const {
  return m_TremulantType;
}

void GOTremulant::SetOrganModified() {
  r_OrganModel.SetOrganModelModified(true);
}

void GOTremulant::Save(GOConfigWriter &cfg) {
  GODrawstop::Save(cfg);
  // An ODF tremulant takes its parameters only from the ODF
  if (!m_IsOdfDefined) {
    const wxString &group = GODrawstop::m_group;

    cfg.WriteInteger(group, WX_PERIOD_MS, m_PeriodMs);
    cfg.WriteInteger(group, WX_START_TIME_MS, m_StartTimeMs);
    cfg.WriteInteger(group, WX_STOP_TIME_MS, m_StopTimeMs);
    cfg.WriteInteger(group, WX_AMP_MOD_DEPTH_PCNT, m_AmpModDepthPcnt);
    cfg.WriteFloat(group, WX_PITCH_MOD_DEPTH_CENTS, m_PitchModDepthCents);
    cfg.WriteInteger(group, WX_PITCH_MOD_PHASE_DEGREES, m_PitchModPhaseDegrees);
  }
}
