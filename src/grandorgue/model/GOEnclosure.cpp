/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOEnclosure.h"

#include <wx/intl.h>
#include <wx/log.h>

#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"
#include "sound/dsp-kernels/GOSoundOnePoleFilter.h"

#include "GOOrganModel.h"

GOEnclosure::GOEnclosure(GOOrganModel &organModel)
  : GOMidiObjectWithShortcut(
    organModel,
    OBJECT_TYPE_ENCLOSURE,
    MIDI_SEND_ENCLOSURE,
    MIDI_RECV_ENCLOSURE,
    KEY_RECV_ENCLOSURE),
    m_IsOdfDefined(false),
    m_DefaultAmpMinimumLevel(0),
    m_Displayed1(false),
    m_Displayed2(false),
    m_AmpMinimumLevel(0),
    m_MIDIValue(0),
    m_DefaultLowShelfFrequency(0.0f),
    m_DefaultLowShelfAttenuationDb(0.0f),
    m_DefaultHighShelfFrequency(0.0f),
    m_DefaultHighShelfAttenuationDb(0.0f),
    m_LowShelfFrequency(0.0f),
    m_LowShelfAttenuationDb(0.0f),
    m_HighShelfFrequency(0.0f),
    m_HighShelfAttenuationDb(0.0f) {}

wxString GOEnclosure::ValidateShelfFrequency(double frequency) {
  return frequency > 0 && frequency <= MAX_SHELF_FREQUENCY
    ? wxString()
    : wxString::Format(
      _("The frequency must be a number greater than 0 and not greater than "
        "%g Hz"),
      MAX_SHELF_FREQUENCY);
}

wxString GOEnclosure::ValidateShelfAttenuationDb(double attenuationDb) {
  return attenuationDb >= 0 && attenuationDb <= MAX_SHELF_ATTENUATION_DB
    ? wxString()
    : wxString::Format(
      _("The attenuation must be a number from 0 to %g dB"),
      MAX_SHELF_ATTENUATION_DB);
}

wxString GOEnclosure::ValidateShelfPair(
  double frequency, double attenuationDb) {
  return attenuationDb <= 0 || frequency > 0
    ? wxString()
    : _("The frequency must be greater than 0 when the attenuation is greater "
        "than 0");
}

wxString GOEnclosure::ValidateLowShelfWith(
  std::optional<double> newFrequency,
  std::optional<double> newAttenuationDb) const {
  return ValidateShelfPair(
    newFrequency.value_or(m_LowShelfFrequency),
    newAttenuationDb.value_or(m_LowShelfAttenuationDb));
}

wxString GOEnclosure::ValidateHighShelfWith(
  std::optional<double> newFrequency,
  std::optional<double> newAttenuationDb) const {
  return ValidateShelfPair(
    newFrequency.value_or(m_HighShelfFrequency),
    newAttenuationDb.value_or(m_HighShelfAttenuationDb));
}

/**
 * Stores the new value only if it is present and differs from the current one.
 * @param newValue the value to be stored, if any
 * @param value the variable to be updated
 * @return whether the variable has got a different value
 */
static bool update_if_changed(std::optional<double> newValue, float &value) {
  const float resultValue = newValue.value_or(value);
  const bool isChanged = resultValue != value;

  if (isChanged)
    value = resultValue;
  return isChanged;
}

void GOEnclosure::SetAmpMinimumLevel(uint8_t v) {
  if (v != m_AmpMinimumLevel) {
    m_AmpMinimumLevel = v;
    r_OrganModel.SetOrganModelModified(true);
  }
}

void GOEnclosure::UpdateShelf(
  std::optional<double> newFrequency,
  std::optional<double> newAttenuationDb,
  float &frequency,
  float &attenuationDb) {
  const bool isFrequencyChanged = update_if_changed(newFrequency, frequency);
  const bool isAttenuationChanged
    = update_if_changed(newAttenuationDb, attenuationDb);

  if (isFrequencyChanged || isAttenuationChanged)
    r_OrganModel.SetOrganModelModified(true);
}

void GOEnclosure::ApplyLowShelf(
  std::optional<double> newFrequency, std::optional<double> newAttenuationDb) {
  UpdateShelf(
    newFrequency,
    newAttenuationDb,
    m_LowShelfFrequency,
    m_LowShelfAttenuationDb);
  WarnIfShelfIgnored(
    _("low shelf"), m_LowShelfFrequency, m_LowShelfAttenuationDb);
}

void GOEnclosure::ApplyHighShelf(
  std::optional<double> newFrequency, std::optional<double> newAttenuationDb) {
  UpdateShelf(
    newFrequency,
    newAttenuationDb,
    m_HighShelfFrequency,
    m_HighShelfAttenuationDb);
  WarnIfShelfIgnored(
    _("high shelf"), m_HighShelfFrequency, m_HighShelfAttenuationDb);
}

void GOEnclosure::WarnIfShelfIgnored(
  const wxString &shelfName, float frequency, float attenuationDb) const {
  const unsigned sampleRate = r_OrganModel.GetSampleRate();

  if (
    attenuationDb > 0
    && !GOSoundOnePoleFilter::isBelowNyquist(frequency, sampleRate))
    wxLogWarning(
      _("Enclosure '%s': the %s is ignored because its frequency %g Hz is not "
        "below the Nyquist frequency %g Hz of the sample rate %u Hz"),
      GetName(),
      shelfName,
      frequency,
      GOSoundOnePoleFilter::getNyquist(sampleRate),
      sampleRate);
}

void GOEnclosure::WarnIfShelvesIgnored() const {
  WarnIfShelfIgnored(
    _("low shelf"), m_LowShelfFrequency, m_LowShelfAttenuationDb);
  WarnIfShelfIgnored(
    _("high shelf"), m_HighShelfFrequency, m_HighShelfAttenuationDb);
}

void GOEnclosure::PreparePlayback() {
  GOMidiReceivingSendingObject::PreparePlayback();
  WarnIfShelvesIgnored();
}

static const wxString WX_AMP_MINIMUM_LEVEL = wxT("AmpMinimumLevel");
static const wxString WX_VALUE = wxT("Value");
static const wxString WX_LOW_SHELF_FREQUENCY = wxT("LowShelfFrequency");
static const wxString WX_LOW_SHELF_ATTENUATION_DB
  = wxT("LowShelfAttenuationDb");
static const wxString WX_HIGH_SHELF_FREQUENCY = wxT("HighShelfFrequency");
static const wxString WX_HIGH_SHELF_ATTENUATION_DB
  = wxT("HighShelfAttenuationDb");

/**
 * Reads the default frequency of a shelf from the ODF. The frequency is
 * required and must be positive only when the shelf has a non-zero
 * attenuation.
 * @param cfg the ODF reader
 * @param group the enclosure section
 * @param key the frequency key
 * @param attenuationDb the already read attenuation of the same shelf
 * @return the frequency in Hz, or 0 if the shelf has no attenuation
 * @throws wxString with the reason if the frequency is not valid together
 *   with the attenuation
 */
static float read_odf_shelf_frequency(
  GOConfigReader &cfg,
  const wxString &group,
  const wxString &key,
  float attenuationDb) {
  float frequency = 0.0f;

  if (attenuationDb > 0) {
    frequency = cfg.ReadFloat(
      ODFSetting, group, key, 0, GOEnclosure::MAX_SHELF_FREQUENCY, true);

    const wxString reason
      = GOEnclosure::ValidateShelfPair(frequency, attenuationDb);

    if (!reason.IsEmpty())
      throw wxString::Format(
        _("Section '%s' entry '%s': %s"), group, key, reason);
  }
  return frequency;
}

void GOEnclosure::LoadFromCmb(GOConfigReader &cfg, uint8_t defaultValue) {
  m_AmpMinimumLevel = cfg.ReadInteger(
    CMBSetting,
    m_group,
    WX_AMP_MINIMUM_LEVEL,
    0,
    100,
    false,
    m_DefaultAmpMinimumLevel);
  m_LowShelfFrequency = cfg.ReadFloat(
    CMBSetting,
    m_group,
    WX_LOW_SHELF_FREQUENCY,
    0,
    MAX_SHELF_FREQUENCY,
    false,
    m_DefaultLowShelfFrequency);
  m_LowShelfAttenuationDb = cfg.ReadFloat(
    CMBSetting,
    m_group,
    WX_LOW_SHELF_ATTENUATION_DB,
    0,
    MAX_SHELF_ATTENUATION_DB,
    false,
    m_DefaultLowShelfAttenuationDb);
  m_HighShelfFrequency = cfg.ReadFloat(
    CMBSetting,
    m_group,
    WX_HIGH_SHELF_FREQUENCY,
    0,
    MAX_SHELF_FREQUENCY,
    false,
    m_DefaultHighShelfFrequency);
  m_HighShelfAttenuationDb = cfg.ReadFloat(
    CMBSetting,
    m_group,
    WX_HIGH_SHELF_ATTENUATION_DB,
    0,
    MAX_SHELF_ATTENUATION_DB,
    false,
    m_DefaultHighShelfAttenuationDb);
  SetEnclosureValue(cfg.ReadInteger(
    CMBSetting, m_group, WX_VALUE, 0, MAX_MIDI_VALUE, false, defaultValue));
}

void GOEnclosure::Init(
  GOConfigReader &cfg,
  const wxString &group,
  const wxString &name,
  uint8_t defaultValue) {
  m_IsOdfDefined = false;
  GOMidiReceivingSendingObject::Init(cfg, group, name);
  m_DefaultAmpMinimumLevel = 0;
  m_DefaultLowShelfFrequency = 0.0f;
  m_DefaultLowShelfAttenuationDb = 0.0f;
  m_DefaultHighShelfFrequency = 0.0f;
  m_DefaultHighShelfAttenuationDb = 0.0f;
  LoadFromCmb(cfg, defaultValue);
}

void GOEnclosure::Load(GOConfigReader &cfg, const wxString &group) {
  m_IsOdfDefined = true;
  GOMidiReceivingSendingObject::Load(
    cfg, group, cfg.ReadStringNotEmpty(ODFSetting, group, wxT("Name")), true);
  m_Displayed1
    = cfg.ReadBoolean(ODFSetting, m_group, wxT("Displayed"), false, true);
  m_Displayed2
    = cfg.ReadBoolean(ODFSetting, m_group, wxT("Displayed"), false, false);
  m_DefaultAmpMinimumLevel
    = cfg.ReadInteger(ODFSetting, m_group, WX_AMP_MINIMUM_LEVEL, 0, 100);
  m_DefaultLowShelfAttenuationDb = cfg.ReadFloat(
    ODFSetting,
    m_group,
    WX_LOW_SHELF_ATTENUATION_DB,
    0,
    MAX_SHELF_ATTENUATION_DB,
    false,
    0);
  m_DefaultLowShelfFrequency = read_odf_shelf_frequency(
    cfg, m_group, WX_LOW_SHELF_FREQUENCY, m_DefaultLowShelfAttenuationDb);
  m_DefaultHighShelfAttenuationDb = cfg.ReadFloat(
    ODFSetting,
    m_group,
    WX_HIGH_SHELF_ATTENUATION_DB,
    0,
    MAX_SHELF_ATTENUATION_DB,
    false,
    0);
  m_DefaultHighShelfFrequency = read_odf_shelf_frequency(
    cfg, m_group, WX_HIGH_SHELF_FREQUENCY, m_DefaultHighShelfAttenuationDb);
  LoadFromCmb(cfg, MAX_MIDI_VALUE);
}

void GOEnclosure::Save(GOConfigWriter &cfg) {
  GOMidiReceivingSendingObject::Save(cfg);
  cfg.WriteInteger(m_group, WX_AMP_MINIMUM_LEVEL, m_AmpMinimumLevel);
  cfg.WriteInteger(m_group, WX_VALUE, m_MIDIValue);
  cfg.WriteFloat(m_group, WX_LOW_SHELF_FREQUENCY, m_LowShelfFrequency);
  cfg.WriteFloat(m_group, WX_LOW_SHELF_ATTENUATION_DB, m_LowShelfAttenuationDb);
  cfg.WriteFloat(m_group, WX_HIGH_SHELF_FREQUENCY, m_HighShelfFrequency);
  cfg.WriteFloat(
    m_group, WX_HIGH_SHELF_ATTENUATION_DB, m_HighShelfAttenuationDb);
}

void GOEnclosure::SetEnclosureValue(uint8_t n) {
  if (n != m_MIDIValue) {
    m_MIDIValue = n;
    SendCurrentMidiValue();
  }
  r_OrganModel.UpdateVolume();
  r_OrganModel.SendControlChanged(this);
}

void GOEnclosure::Scroll(bool scroll_up) {
  SetIntEnclosureValue(m_MIDIValue + (scroll_up ? 4 : -4));
}

void GOEnclosure::OnMidiReceived(
  const GOMidiEvent &event, GOMidiMatchType matchType, int key, int value) {
  if (matchType == MIDI_MATCH_CHANGE)
    SetEnclosureValue(value);
}

void GOEnclosure::OnShortcutKeyReceived(
  GOMidiShortcutReceiver::MatchType matchType, int key) {
  switch (matchType) {
  case GOMidiShortcutReceiver::KEY_MATCH:
    SetIntEnclosureValue(m_MIDIValue + 8);
    break;

  case GOMidiShortcutReceiver::KEY_MATCH_MINUS:
    SetIntEnclosureValue(m_MIDIValue - 8);
    break;
  default:
    break;
  }
}

bool GOEnclosure::IsDisplayed(bool new_format) {
  if (new_format)
    return m_Displayed2;
  else
    return m_Displayed1;
}

wxString GOEnclosure::GetElementStatus() {
  return wxString::Format(_("%.3f %%"), (m_MIDIValue * 100.0 / 127));
}

std::vector<wxString> GOEnclosure::GetElementActions() {
  std::vector<wxString> actions;
  actions.push_back(_("-"));
  actions.push_back(_("+"));
  return actions;
}

void GOEnclosure::TriggerElementActions(unsigned no) {
  if (no == 0)
    Scroll(false);
  if (no == 1)
    Scroll(true);
}
