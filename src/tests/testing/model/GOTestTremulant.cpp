/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestTremulant.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <numeric>

#include "config/GOConfigFileReader.h"
#include "config/GOConfigFileWriter.h"
#include "config/GOConfigWriter.h"
#include "model/GOTremulant.h"
#include "sound/effects/GOSoundVibratoPitchRateCurve.h"

#include "GOSaveableObject.h"

#include "GOTestScope.h"
#include "GOTestTremulantFixture.h"

const std::string GOTestTremulant::TEST_NAME = "GOTestTremulant";

static constexpr float TOLERANCE = 1e-4f;

static bool are_close(float a, float b) {
  return std::fabs(a - b) <= TOLERANCE;
}

static float max_of(const std::vector<float> &values) {
  return *std::max_element(values.begin(), values.end());
}

static float min_of(const std::vector<float> &values) {
  return *std::min_element(values.begin(), values.end());
}

/**
 * @return the pitch shift in cents of a relative frequency deviation df/f
 */
static double deviation_to_cents(double deviation) {
  return 1200.0 * std::log2(1.0 + deviation);
}

/**
 * @return the average pitch in cents of one period of relative frequency
 *   deviations df/f, every frame counted as a separate pitch
 */
static double mean_cents_of(const std::vector<float> &deviations) {
  const double sum = std::accumulate(
    deviations.begin(), deviations.end(), 0.0, [](double sum, float deviation) {
      return sum + deviation_to_cents(deviation);
    });

  return deviations.empty() ? 0.0 : sum / deviations.size();
}

/**
 * @return whether loading the ODF text throws
 */
static bool does_load_throw(const std::string &odfText) {
  GOTestTremulantModel model;
  bool isThrown = false;

  try {
    load_tremulant_from_text(model.organModel, odfText, "");
  } catch (const wxString &) {
    isThrown = true;
  }
  return isThrown;
}

void GOTestTremulant::TestLoadRateConversion() {
  GOTestTremulantModel model;
  GOTremulant::Description desc
    = load_tremulant(model.organModel, "Synth", 100, 4, 8, 20)->Render(48000);

  GOAssert(desc.nStartFrames == 12000, "StartRate 4 must be 250 ms");
  GOAssert(desc.nStopFrames == 6000, "StopRate 8 must be 125 ms");

  desc
    = load_tremulant(model.organModel, "Synth", 100, 3, 3, 20)->Render(48000);
  GOAssert(desc.nStartFrames == 15984, "StartRate 3 must be 333 ms (rounded)");
}

void GOTestTremulant::TestLoadRequiredKeys() {
  const std::string head = "[Tremulant001]\nName=Test\n";

  GOAssert(
    !does_load_throw(
      head + "Period=100\nStartRate=4\nStopRate=8\nAmpModDepth=20\n"),
    "all the keys given must load");
  GOAssert(
    does_load_throw(head + "StartRate=4\nStopRate=8\nAmpModDepth=20\n"),
    "a missing Period must throw");
  GOAssert(
    does_load_throw(head + "Period=100\nStopRate=8\nAmpModDepth=20\n"),
    "a missing StartRate must throw");
  GOAssert(
    does_load_throw(head + "Period=100\nStartRate=4\nAmpModDepth=20\n"),
    "a missing StopRate must throw");
  GOAssert(
    does_load_throw(head + "Period=100\nStartRate=4\nStopRate=8\n"),
    "a missing AmpModDepth must throw");
}

void GOTestTremulant::TestLoadTypes() {
  GOTestTremulantModel model;

  GOAssert(
    load_tremulant_from_text(
      model.organModel,
      "[Tremulant001]\nName=Test\nPeriod=100\nStartRate=4\nStopRate=8\n"
      "AmpModDepth=20\n",
      "")
        ->GetTremulantType()
      == GOSynthTrem,
    "the default type must be Synth");
  GOAssert(
    load_tremulant(model.organModel, "Synth", 100, 4, 8, 20)->GetTremulantType()
      == GOSynthTrem,
    "Synth must be recognised");
  GOAssert(
    load_tremulant_from_text(
      model.organModel, "[Tremulant001]\nName=Test\nTremulantType=Wave\n", "")
        ->GetTremulantType()
      == GOWavTrem,
    "Wave must be recognised");
}

/**
 * Saves a tremulant to a temporary .cmb file and reads the file back.
 */
static void save_and_read(
  GOTremulant &tremulant, GOConfigFileReader &outReader) {
  GOConfigFileWriter file;
  GOConfigWriter writer(file, false);
  GOSaveableObject &saveable = tremulant;
  const std::filesystem::path path
    = std::filesystem::temp_directory_path() / "GOTestTremulant-save.cmb";

  saveable.Save(writer);
  file.Save(wxString(path.string()), false);
  outReader.Read(wxString(path.string()));
  std::filesystem::remove(path);
}

void GOTestTremulant::TestInitDefaults() {
  GOTestTremulantModel model;
  std::unique_ptr<GOTremulant> oTremulant
    = init_tremulant_from_cmb(model.organModel, "");
  const GOTremulant::Description desc = oTremulant->Render(48000);

  GOAssert(!oTremulant->IsOdfDefined(), "Init() must make it internal");
  GOAssert(oTremulant->GetTremulantType() == GOSynthTrem, "the type is Synth");
  GOAssert(desc.nPeriodFrames == 9600, "the default period is 200 ms");
  GOAssert(
    desc.ampDeviation.size() == 9600, "the amplitude has the period length");
  GOAssert(desc.nStartFrames == 14400, "the default start time is 300 ms");
  GOAssert(desc.nStopFrames == 14400, "the default stop time is 300 ms");
  GOAssert(
    are_close(min_of(desc.ampDeviation), -0.5f)
      && are_close(max_of(desc.ampDeviation), 0.5f),
    "the default amplitude depth is 50%");
  GOAssert(desc.pitchDeviation.empty(), "the default has no pitch modulation");
}

void GOTestTremulant::TestLoadFromCmbFallback() {
  GOTestTremulantModel model;

  // Only two keys: the others get their defaults
  const std::string partialCmb = "PeriodMs=50\nAmpModDepthPcnt=10\n";
  GOTremulant::Description desc
    = init_tremulant_from_cmb(model.organModel, partialCmb)->Render(48000);

  GOAssert(desc.nPeriodFrames == 2400, "the .cmb period must be used");
  GOAssert(
    desc.ampDeviation.size() == 2400,
    "the amplitude must have the .cmb period length");
  GOAssert(
    are_close(max_of(desc.ampDeviation), 0.1f), "the .cmb depth must be used");
  GOAssert(desc.nStartFrames == 14400, "a missing start time is the default");
  GOAssert(desc.nStopFrames == 14400, "a missing stop time is the default");
  GOAssert(desc.pitchDeviation.empty(), "a missing pitch depth is 0");

  // All the keys
  const std::string allCmb
    = "PeriodMs=50\nStartTimeMs=100\nStopTimeMs=200\nAmpModDepthPcnt=10\n"
      "PitchModDepthCents=25\nPitchModPhaseDegrees=90\n";

  desc = init_tremulant_from_cmb(model.organModel, allCmb)->Render(48000);
  GOAssert(desc.nStartFrames == 4800, "the .cmb start time must be used");
  GOAssert(desc.nStopFrames == 9600, "the .cmb stop time must be used");
  GOAssert(desc.pitchDeviation.size() == 2400, "the .cmb pitch must be used");
  GOAssert(
    are_close(desc.pitchDeviation[0], 1.0f - std::exp2(-25.0f / 1200.0f)),
    "the .cmb phase must be used: at 90 degrees the pitch starts at the peak");
}

void GOTestTremulant::TestInitDoesNotMarkModified() {
  GOTestTremulantModel model;

  init_tremulant_from_cmb(
    model.organModel, "PeriodMs=50\nPitchModDepthCents=25\n");
  GOAssert(
    !model.organModel.IsOrganModelModified(),
    "Init() must not mark the organ model as modified");
}

void GOTestTremulant::TestInitSaveRoundTrip() {
  GOTestTremulantModel model;
  std::unique_ptr<GOTremulant> oTremulant
    = init_tremulant_from_cmb(model.organModel, "");

  oTremulant->ApplySynthParamChanges(
    {.periodMs = 70,
     .ampModDepthPcnt = 15,
     .pitchModDepthCents = 30.0f,
     .pitchModPhaseDegrees = -45,
     .startTimeMs = 120,
     .stopTimeMs = 80});

  GOConfigFileReader reader;

  save_and_read(*oTremulant, reader);
  GOAssert(
    reader.getEntry(TEST_TREMULANT_GROUP, wxT("PeriodMs")) == wxT("70"),
    "PeriodMs must be saved");
  GOAssert(
    reader.getEntry(TEST_TREMULANT_GROUP, wxT("AmpModDepthPcnt")) == wxT("15"),
    "AmpModDepthPcnt must be saved");
  GOAssert(
    reader.getEntry(TEST_TREMULANT_GROUP, wxT("PitchModPhaseDegrees"))
      == wxT("-45"),
    "PitchModPhaseDegrees must be saved");

  // Initialising from the saved values gives the same wave form
  const std::string savedCmb
    = "PeriodMs=70\nStartTimeMs=120\nStopTimeMs=80\nAmpModDepthPcnt=15\n"
      "PitchModDepthCents=30\nPitchModPhaseDegrees=-45\n";
  const GOTremulant::Description descOrig = oTremulant->Render(48000);
  const GOTremulant::Description descLoaded
    = init_tremulant_from_cmb(model.organModel, savedCmb)->Render(48000);

  GOAssert(
    descOrig.ampDeviation == descLoaded.ampDeviation,
    "the amplitude must round-trip");
  GOAssert(
    descOrig.pitchDeviation == descLoaded.pitchDeviation,
    "the pitch must round-trip");
  GOAssert(
    descOrig.nStartFrames == descLoaded.nStartFrames
      && descOrig.nStopFrames == descLoaded.nStopFrames,
    "the ramps must round-trip");
}

void GOTestTremulant::TestOdfIgnoresCmb() {
  GOTestTremulantModel model;
  std::unique_ptr<GOTremulant> oTremulant = load_tremulant_from_text(
    model.organModel,
    "[Tremulant001]\nName=Test\nPeriod=100\nStartRate=4\nStopRate=8\n"
    "AmpModDepth=20\n",
    "[Tremulant001]\nPeriodMs=50\nStartTimeMs=100\nStopTimeMs=200\n"
    "AmpModDepthPcnt=10\nPitchModDepthCents=25\nPitchModPhaseDegrees=90\n");

  GOAssert(oTremulant->IsOdfDefined(), "Load() must make it ODF-defined");

  const GOTremulant::Description desc = oTremulant->Render(48000);

  GOAssert(desc.nPeriodFrames == 4800, "the ODF period must be used");
  GOAssert(
    are_close(max_of(desc.ampDeviation), 0.2f), "the ODF depth must be used");
  GOAssert(desc.nStartFrames == 12000, "the ODF start rate must be used");
  GOAssert(desc.nStopFrames == 6000, "the ODF stop rate must be used");
  GOAssert(desc.pitchDeviation.empty(), "the .cmb pitch must be ignored");

  // The parameters are not saved either
  GOConfigFileReader reader;

  save_and_read(*oTremulant, reader);
  GOAssert(
    reader.getEntry(TEST_TREMULANT_GROUP, wxT("PeriodMs")).IsEmpty()
      && reader.getEntry(TEST_TREMULANT_GROUP, wxT("StartTimeMs")).IsEmpty()
      && reader.getEntry(TEST_TREMULANT_GROUP, wxT("StopTimeMs")).IsEmpty()
      && reader.getEntry(TEST_TREMULANT_GROUP, wxT("AmpModDepthPcnt")).IsEmpty()
      && reader.getEntry(TEST_TREMULANT_GROUP, wxT("PitchModDepthCents"))
           .IsEmpty()
      && reader.getEntry(TEST_TREMULANT_GROUP, wxT("PitchModPhaseDegrees"))
           .IsEmpty(),
    "an ODF tremulant must not save its parameters");
}

void GOTestTremulant::TestLoadDoesNotMarkModified() {
  GOTestTremulantModel model;

  load_tremulant(model.organModel, "Synth", 100, 4, 8, 20);
  GOAssert(
    !model.organModel.IsOrganModelModified(),
    "loading must not mark the organ model as modified");
}

void GOTestTremulant::TestApplyMarksModified() {
  GOTestTremulantModel model;
  std::unique_ptr<GOTremulant> oTremulant
    = load_tremulant(model.organModel, "Synth", 100, 4, 8, 20);

  // The same values as loaded
  GOAssert(
    !oTremulant->ApplySynthParamChanges(
      {.periodMs = 100,
       .ampModDepthPcnt = 20,
       .pitchModDepthCents = 0.0f,
       .pitchModPhaseDegrees = 0,
       .startTimeMs = 250,
       .stopTimeMs = 125}),
    "setting the same values must not report a change");
  GOAssert(
    !model.organModel.IsOrganModelModified(),
    "setting the same values must not mark the organ model as modified");

  GOAssert(
    oTremulant->ApplySynthParamChanges({.periodMs = 101}), "a new periodMs");
  GOAssert(
    model.organModel.IsOrganModelModified(), "periodMs must mark modified");
  model.organModel.ResetOrganModelModified();
  GOAssert(
    oTremulant->ApplySynthParamChanges({.ampModDepthPcnt = 21}),
    "a new ampModDepthPcnt");
  GOAssert(
    model.organModel.IsOrganModelModified(),
    "ampModDepthPcnt must mark modified");
  model.organModel.ResetOrganModelModified();
  GOAssert(
    oTremulant->ApplySynthParamChanges({.pitchModDepthCents = 1.0f}),
    "a new pitchModDepthCents");
  GOAssert(
    model.organModel.IsOrganModelModified(),
    "pitchModDepthCents must mark modified");
  model.organModel.ResetOrganModelModified();
  GOAssert(
    oTremulant->ApplySynthParamChanges({.pitchModPhaseDegrees = 1}),
    "a new pitchModPhaseDegrees");
  GOAssert(
    model.organModel.IsOrganModelModified(),
    "pitchModPhaseDegrees must mark modified");
  model.organModel.ResetOrganModelModified();
  GOAssert(
    oTremulant->ApplySynthParamChanges({.startTimeMs = 251}),
    "a new startTimeMs");
  GOAssert(
    model.organModel.IsOrganModelModified(), "startTimeMs must mark modified");
  model.organModel.ResetOrganModelModified();
  GOAssert(
    oTremulant->ApplySynthParamChanges({.stopTimeMs = 126}),
    "a new stopTimeMs");
  GOAssert(
    model.organModel.IsOrganModelModified(), "stopTimeMs must mark modified");
}

void GOTestTremulant::TestGettersReturnSetValues() {
  GOTestTremulantModel model;
  std::unique_ptr<GOTremulant> oTremulant
    = init_tremulant_from_cmb(model.organModel, "");

  oTremulant->ApplySynthParamChanges(
    {.periodMs = 123,
     .ampModDepthPcnt = 33,
     .pitchModDepthCents = 44.0f,
     .pitchModPhaseDegrees = -55,
     .startTimeMs = 11,
     .stopTimeMs = 22});
  GOAssert(oTremulant->GetPeriodMs() == 123, "GetPeriodMs");
  GOAssert(oTremulant->GetStartTimeMs() == 11, "GetStartTimeMs");
  GOAssert(oTremulant->GetStopTimeMs() == 22, "GetStopTimeMs");
  GOAssert(oTremulant->GetAmpModDepthPcnt() == 33, "GetAmpModDepthPcnt");
  GOAssert(oTremulant->GetPitchModDepthCents() == 44.0f, "GetPitchModDepth");
  GOAssert(oTremulant->GetPitchModPhaseDegrees() == -55, "GetPitchModPhase");
}

void GOTestTremulant::TestParamRanges() {
  const GOParamRange<int> range{-2, 5};

  GOAssert(range.Contains(-2) && range.Contains(5), "both ends are included");
  GOAssert(range.Contains(0), "an inner value is contained");
  GOAssert(!range.Contains(-3) && !range.Contains(6), "outside is not");

  // A value of another type is compared as is; a NaN is never contained
  const double nan = std::nan("");
  const GOParamRange<float> floatRange{0.0f, 10.0f};

  GOAssert(
    range.Contains(2.5) && floatRange.Contains(10.0)
      && !floatRange.Contains(10.5),
    "a double value");
  GOAssert(
    !range.Contains(nan) && !floatRange.Contains(nan), "NaN is not contained");

  // Clamp() limits the value to the range and keeps the inner values
  GOAssert(
    range.Clamp(-3) == -2 && range.Clamp(6) == 5 && range.Clamp(0) == 0
      && range.Clamp(-2) == -2 && range.Clamp(5) == 5,
    "Clamp must limit the value to the range");

  // The .cmb accepts the borders of the ranges
  GOTestTremulantModel model;
  std::unique_ptr<GOTremulant> oMin = init_tremulant_from_cmb(
    model.organModel,
    "PeriodMs=32\nStartTimeMs=0\nStopTimeMs=0\nAmpModDepthPcnt=0\n"
    "PitchModDepthCents=0\nPitchModPhaseDegrees=-180\n");
  std::unique_ptr<GOTremulant> oMax = init_tremulant_from_cmb(
    model.organModel,
    "PeriodMs=441000\nStartTimeMs=10000\nStopTimeMs=10000\n"
    "AmpModDepthPcnt=100\nPitchModDepthCents=2400\nPitchModPhaseDegrees=359\n");

  GOAssert(
    oMin->GetPeriodMs() == GOTremulant::PERIOD_MS_RANGE.m_min
      && oMax->GetPeriodMs() == GOTremulant::PERIOD_MS_RANGE.m_max,
    "period");
  GOAssert(
    oMin->GetStartTimeMs() == GOTremulant::START_TIME_MS_RANGE.m_min
      && oMax->GetStartTimeMs() == GOTremulant::START_TIME_MS_RANGE.m_max,
    "start time");
  GOAssert(
    oMin->GetStopTimeMs() == GOTremulant::STOP_TIME_MS_RANGE.m_min
      && oMax->GetStopTimeMs() == GOTremulant::STOP_TIME_MS_RANGE.m_max,
    "stop time");
  GOAssert(
    oMin->GetAmpModDepthPcnt() == GOTremulant::AMP_MOD_DEPTH_PCNT_RANGE.m_min
      && oMax->GetAmpModDepthPcnt()
        == GOTremulant::AMP_MOD_DEPTH_PCNT_RANGE.m_max,
    "amplitude depth");
  GOAssert(
    oMin->GetPitchModDepthCents()
        == GOTremulant::PITCH_MOD_DEPTH_CENTS_RANGE.m_min
      && oMax->GetPitchModDepthCents()
        == GOTremulant::PITCH_MOD_DEPTH_CENTS_RANGE.m_max,
    "pitch depth");
  GOAssert(
    oMin->GetPitchModPhaseDegrees()
        == GOTremulant::PITCH_MOD_PHASE_DEGREES_RANGE.m_min
      && oMax->GetPitchModPhaseDegrees()
        == GOTremulant::PITCH_MOD_PHASE_DEGREES_RANGE.m_max,
    "pitch phase");
}

void GOTestTremulant::TestApplySynthParamChanges() {
  GOTestTremulantModel model;
  std::unique_ptr<GOTremulant> oTremulant
    = init_tremulant_from_cmb(model.organModel, "");
  GOTremulant::SynthParamChanges changes;

  GOAssert(
    !oTremulant->ApplySynthParamChanges(changes)
      && !model.organModel.IsOrganModelModified(),
    "no values present: nothing changes");

  changes.periodMs = oTremulant->GetPeriodMs();
  changes.pitchModDepthCents = oTremulant->GetPitchModDepthCents();
  GOAssert(
    !oTremulant->ApplySynthParamChanges(changes)
      && !model.organModel.IsOrganModelModified(),
    "the same values: nothing changes");

  changes.periodMs = 300;
  changes.stopTimeMs = 77;
  GOAssert(
    oTremulant->ApplySynthParamChanges(changes)
      && model.organModel.IsOrganModelModified(),
    "different values: changed and modified");
  GOAssert(
    oTremulant->GetPeriodMs() == 300 && oTremulant->GetStopTimeMs() == 77,
    "the present values are applied");
  GOAssert(
    oTremulant->GetStartTimeMs() == 300
      && oTremulant->GetAmpModDepthPcnt() == 50
      && oTremulant->GetPitchModDepthCents() == 0.0f
      && oTremulant->GetPitchModPhaseDegrees() == 0,
    "the absent values are kept");
}

void GOTestTremulant::TestApplyClampsToRanges() {
  GOTestTremulantModel model;
  std::unique_ptr<GOTremulant> oTremulant
    = init_tremulant_from_cmb(model.organModel, "");

  GOAssert(
    oTremulant->ApplySynthParamChanges(
      {.periodMs = 1,
       .ampModDepthPcnt = -5,
       .pitchModDepthCents = 3000.0f,
       .pitchModPhaseDegrees = 400,
       .startTimeMs = -1,
       .stopTimeMs = 20000}),
    "the clamped values differ from the defaults");
  GOAssert(
    oTremulant->GetPeriodMs() == GOTremulant::PERIOD_MS_RANGE.m_min
      && oTremulant->GetAmpModDepthPcnt() == 0
      && oTremulant->GetPitchModDepthCents() == 2400.0f
      && oTremulant->GetPitchModPhaseDegrees() == 359
      && oTremulant->GetStartTimeMs() == 0
      && oTremulant->GetStopTimeMs() == 10000,
    "the out-of-range values must be clamped to the ranges");
  model.organModel.ResetOrganModelModified();
  GOAssert(
    !oTremulant->ApplySynthParamChanges({.periodMs = 0})
      && !model.organModel.IsOrganModelModified(),
    "a value that clamps to the current one changes nothing");
}

void GOTestTremulant::TestResetSynthParamsToDefault() {
  GOTestTremulantModel model;
  std::unique_ptr<GOTremulant> oTremulant
    = init_tremulant_from_cmb(model.organModel, "");
  const GOTremulant::Description defaultDesc = oTremulant->Render(48000);

  GOAssert(
    !oTremulant->ResetSynthParamsToDefault()
      && !model.organModel.IsOrganModelModified(),
    "the defaults: nothing changes");

  oTremulant->ApplySynthParamChanges(
    {.periodMs = 300,
     .ampModDepthPcnt = 3,
     .pitchModDepthCents = 4.0f,
     .pitchModPhaseDegrees = 5,
     .startTimeMs = 1,
     .stopTimeMs = 2});
  model.organModel.ResetOrganModelModified();
  GOAssert(oTremulant->ResetSynthParamsToDefault(), "changed parameters");
  GOAssert(model.organModel.IsOrganModelModified(), "the organ is modified");
  GOAssert(
    !oTremulant->ResetSynthParamsToDefault(),
    "the second call changes nothing");

  const GOTremulant::Description desc = oTremulant->Render(48000);

  GOAssert(
    desc.ampDeviation == defaultDesc.ampDeviation
      && desc.pitchDeviation == defaultDesc.pitchDeviation
      && desc.nStartFrames == defaultDesc.nStartFrames
      && desc.nStopFrames == defaultDesc.nStopFrames,
    "the same wave form as after Init()");
}

void GOTestTremulant::TestRenderPeriodLength() {
  GOTestTremulantModel model;
  GOTremulant::Description desc
    = load_tremulant(model.organModel, "Synth", 100, 4, 8, 20)->Render(48000);

  GOAssert(desc.nPeriodFrames == 4800, "100 ms at 48 kHz must be 4800 frames");
  GOAssert(
    desc.ampDeviation.size() == 4800, "the amplitude must have the period");
  desc
    = load_tremulant(model.organModel, "Synth", 100, 4, 8, 20)->Render(44100);
  GOAssert(
    desc.nPeriodFrames == 4410, "100 ms at 44.1 kHz must be 4410 frames");
  desc = load_tremulant(model.organModel, "Synth", 33, 4, 8, 20)->Render(44100);
  GOAssert(
    desc.nPeriodFrames == 1455, "33 ms at 44.1 kHz must be rounded to 1455");
  GOAssert(
    desc.ampDeviation.size() == 1455, "the amplitude must have the period");
}

void GOTestTremulant::TestRenderAmplitude() {
  GOTestTremulantModel model;
  GOTremulant::Description desc
    = load_tremulant(model.organModel, "Synth", 100, 4, 8, 20)->Render(48000);

  GOAssert(
    are_close(max_of(desc.ampDeviation), 0.2f), "depth 20 must peak at +0.2");
  GOAssert(
    are_close(min_of(desc.ampDeviation), -0.2f), "depth 20 must dip to -0.2");

  desc
    = load_tremulant(model.organModel, "Synth", 100, 4, 8, 50)->Render(48000);
  GOAssert(
    are_close(max_of(desc.ampDeviation), 0.5f), "depth 50 must peak at +0.5");
  GOAssert(
    are_close(min_of(desc.ampDeviation), -0.5f), "depth 50 must dip to -0.5");
}

void GOTestTremulant::TestRenderPitch() {
  GOTestTremulantModel model;
  std::unique_ptr<GOTremulant> oTremulant
    = load_tremulant(model.organModel, "Synth", 100, 4, 8, 20);
  const unsigned nPeriodFrames = 4800;
  const float depthCents = 25.0f;
  // The pitch goes down by exactly this relative frequency deviation
  const float depth = 1.0f - std::exp2(-depthCents / 1200.0f);
  GOTremulant::Description desc = oTremulant->Render(48000);

  GOAssert(desc.pitchDeviation.empty(), "the pitch must be empty by default");

  oTremulant->ApplySynthParamChanges({.pitchModDepthCents = depthCents});
  desc = oTremulant->Render(48000);
  GOAssert(desc.pitchDeviation.size() == nPeriodFrames, "the pitch length");
  GOAssert(
    are_close(max_of(desc.pitchDeviation), depth), "the pitch upward peak");
  GOAssert(
    are_close(min_of(desc.pitchDeviation), -depth), "the pitch downward peak");
  GOAssert(are_close(desc.pitchDeviation[0], 0.0f), "phase 0 starts at 0");

  oTremulant->ApplySynthParamChanges({.pitchModPhaseDegrees = 90});
  desc = oTremulant->Render(48000);
  GOAssert(
    are_close(desc.pitchDeviation[0], depth), "phase 90 must lead to the peak");

  oTremulant->ApplySynthParamChanges({.pitchModPhaseDegrees = -90});
  desc = oTremulant->Render(48000);

  const std::vector<float> minus90 = desc.pitchDeviation;

  oTremulant->ApplySynthParamChanges({.pitchModPhaseDegrees = 270});
  desc = oTremulant->Render(48000);
  GOAssert(minus90 == desc.pitchDeviation, "phase -90 must equal 270");

  oTremulant->ApplySynthParamChanges({.pitchModPhaseDegrees = -180});
  desc = oTremulant->Render(48000);

  const std::vector<float> minus180 = desc.pitchDeviation;

  oTremulant->ApplySynthParamChanges({.pitchModPhaseDegrees = 180});
  desc = oTremulant->Render(48000);
  GOAssert(minus180 == desc.pitchDeviation, "phase -180 must equal 180");
}

void GOTestTremulant::TestRenderPitchTable() {
  struct TableCase {
    float depthCents;
    double expectedDownCents;
    double expectedUpCents;
    double expectedMeanCents;
    double meanTolerance;
  };
  /* The rows of the table in the help, in the help's order. The average pitch
   * is not 0, because the sine is linear in frequency: the average frequency
   * is the nominal one, which lies slightly below 0 in cents */
  const TableCase cases[] = {
    {15.0f, -15.0, 14.87, -0.03, 0.01},
    {50.0f, -50.0, 48.60, -0.35, 0.01},
    {2400.0f, -2400.0, 968.8, -321.0, 0.1}};

  for (const TableCase &tableCase : cases) {
    GOTestTremulantModel model;
    std::unique_ptr<GOTremulant> oTremulant
      = init_tremulant_from_cmb(model.organModel, "");

    oTremulant->ApplySynthParamChanges(
      {.pitchModDepthCents = tableCase.depthCents});
    const GOTremulant::Description desc = oTremulant->Render(48000);

    const double downCents = deviation_to_cents(min_of(desc.pitchDeviation));
    const double upCents = deviation_to_cents(max_of(desc.pitchDeviation));
    const double meanCents = mean_cents_of(desc.pitchDeviation);

    GOAssert(
      std::fabs(downCents - tableCase.expectedDownCents) <= 0.01,
      "the pitch must go down exactly by the depth in cents");
    GOAssert(
      std::fabs(upCents - tableCase.expectedUpCents) <= 0.05,
      "the upward peak must be the value of the help table");
    GOAssert(
      std::fabs(meanCents - tableCase.expectedMeanCents)
        <= tableCase.meanTolerance,
      "the average pitch must be the value of the help table");
  }
}

void GOTestTremulant::TestRenderPitchDepthBound() {
  // The deepest pitch modulation is the lowest deviation the curve accepts
  GOAssert(
    1.0f - std::exp2(-2400.0f / 1200.0f)
      == -GOSoundVibratoPitchRateCurve::MIN_DEVIATION,
    "2400 cents must give the lowest deviation of the vibrato curve");
}

void GOTestTremulant::TestRenderZeroDepth() {
  GOTestTremulantModel model;
  std::unique_ptr<GOTremulant> oTremulant
    = init_tremulant_from_cmb(model.organModel, "");

  oTremulant->ApplySynthParamChanges(
    {.ampModDepthPcnt = 0, .pitchModDepthCents = 0.0f});
  const GOTremulant::Description desc = oTremulant->Render(48000);

  GOAssert(
    desc.ampDeviation.empty(),
    "a zero amplitude depth must empty the amplitude");
  GOAssert(
    desc.pitchDeviation.empty(), "a zero pitch depth must empty the pitch");
  GOAssert(desc.nPeriodFrames == 9600, "the period stays");
}

void GOTestTremulant::TestRenderZeroMean() {
  GOTestTremulantModel model;
  std::unique_ptr<GOTremulant> oTremulant
    = load_tremulant(model.organModel, "Synth", 100, 4, 8, 20);

  oTremulant->ApplySynthParamChanges({.pitchModDepthCents = 25.0f});
  const GOTremulant::Description desc = oTremulant->Render(48000);

  const double ampMean
    = std::accumulate(desc.ampDeviation.begin(), desc.ampDeviation.end(), 0.0)
    / desc.ampDeviation.size();
  const double pitchMean
    = std::accumulate(
        desc.pitchDeviation.begin(), desc.pitchDeviation.end(), 0.0)
    / desc.pitchDeviation.size();

  GOAssert(std::fabs(ampMean) < 1e-5, "the mean amplitude must be 0");
  GOAssert(std::fabs(pitchMean) < 1e-5, "the mean pitch must be 0");
}

void GOTestTremulant::TestRenderUsesParams() {
  GOTestTremulantModel model;
  std::unique_ptr<GOTremulant> oTremulant
    = load_tremulant(model.organModel, "Synth", 100, 4, 8, 20);

  oTremulant->ApplySynthParamChanges(
    {.periodMs = 50, .ampModDepthPcnt = 0, .startTimeMs = 10, .stopTimeMs = 0});
  const GOTremulant::Description desc = oTremulant->Render(48000);

  GOAssert(desc.nPeriodFrames == 2400, "periodMs");
  GOAssert(desc.nStartFrames == 480, "startTimeMs");
  GOAssert(desc.nStopFrames == 0, "a zero stop time must give no ramp");
  GOAssert(desc.ampDeviation.empty(), "a zero depth must give no amplitude");
}

void GOTestTremulant::run() {
  GO_RUN_TEST(TestLoadRateConversion())
  GO_RUN_TEST(TestLoadRequiredKeys())
  GO_RUN_TEST(TestLoadTypes())
  GO_RUN_TEST(TestInitDefaults())
  GO_RUN_TEST(TestLoadFromCmbFallback())
  GO_RUN_TEST(TestInitDoesNotMarkModified())
  GO_RUN_TEST(TestInitSaveRoundTrip())
  GO_RUN_TEST(TestOdfIgnoresCmb())
  GO_RUN_TEST(TestLoadDoesNotMarkModified())
  GO_RUN_TEST(TestApplyMarksModified())
  GO_RUN_TEST(TestGettersReturnSetValues())
  GO_RUN_TEST(TestParamRanges())
  GO_RUN_TEST(TestApplySynthParamChanges())
  GO_RUN_TEST(TestApplyClampsToRanges())
  GO_RUN_TEST(TestResetSynthParamsToDefault())
  GO_RUN_TEST(TestRenderPeriodLength())
  GO_RUN_TEST(TestRenderAmplitude())
  GO_RUN_TEST(TestRenderPitch())
  GO_RUN_TEST(TestRenderPitchTable())
  GO_RUN_TEST(TestRenderPitchDepthBound())
  GO_RUN_TEST(TestRenderZeroDepth())
  GO_RUN_TEST(TestRenderZeroMean())
  GO_RUN_TEST(TestRenderUsesParams())
}
