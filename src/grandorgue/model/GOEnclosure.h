/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOENCLOSURE_H_
#define GOENCLOSURE_H_

#include <algorithm>
#include <cstdint>
#include <optional>

#include <wx/string.h>

#include "control/GOControl.h"
#include "midi/objects/GOMidiObjectWithShortcut.h"

class GOConfigReader;
class GOConfigWriter;
class GOOrganModel;

class GOEnclosure : public GOControl, public GOMidiObjectWithShortcut {
public:
  static const wxString WX_MIDI_TYPE_CODE;
  static const wxString WX_MIDI_TYPE_NAME;

private:
  bool m_IsOdfDefined;
  uint8_t m_DefaultAmpMinimumLevel;
  bool m_Displayed1;
  bool m_Displayed2;

  uint8_t m_AmpMinimumLevel;
  uint8_t m_MIDIValue;

  // The ODF-authored (or, for internal enclosures, always-0) shelf
  // parameters. Serve two purposes, same as m_DefaultAmpMinimumLevel: (1)
  // the fallback LoadFromCmb() substitutes when a .cmb has no saved
  // override for the current-value fields below, since LoadFromCmb() runs
  // unconditionally for both ODF-defined and internal enclosures; (2) the
  // value GOOrganSettingsEnclosuresTab::ResetToDefault() restores the
  // current fields to.
  float m_DefaultLowShelfFrequency;
  float m_DefaultLowShelfAttenuationDb;
  float m_DefaultHighShelfFrequency;
  float m_DefaultHighShelfAttenuationDb;

  // The current, effective shelf parameters: loaded from .cmb (falling back
  // to m_Default* above), editable via Organ Settings only for internal
  // enclosures (IsOdfDefined() == false). Frequency is in Hz; attenuation
  // is a non-negative dB cut (0..121), distinct from m_AmpMinimumLevel's
  // unitless linear fraction - GetCurrentLowShelfGainDb()/
  // GetCurrentHighShelfGainDb() convert it to the signed gain
  // GOSoundShelfFilterProcessor::SetLowShelf()/SetHighShelf() expects.
  float m_LowShelfFrequency;
  float m_LowShelfAttenuationDb;
  float m_HighShelfFrequency;
  float m_HighShelfAttenuationDb;

  /** Linearly interpolates a quantity by this enclosure's current MIDI
   * position: atClosed when m_MIDIValue == 0, atOpen when m_MIDIValue ==
   * MAX_MIDI_VALUE, linear in between. The open endpoint is returned exactly
   * (not computed), even with -ffast-math: a shelf filter is bypassed only
   * when its gain is exactly 0. Shared by GetAttenuation() and the shelf-EQ
   * effective gain getters - they differ only in their two endpoint
   * values. */
  float InterpolateByMidiValue(float atClosed, float atOpen) const {
    return m_MIDIValue == MAX_MIDI_VALUE
      ? atOpen
      : atClosed + (atOpen - atClosed) * m_MIDIValue / 127.0f;
  }

  void OnMidiReceived(
    const GOMidiEvent &event,
    GOMidiMatchType matchType,
    int key,
    int value) override;
  void OnShortcutKeyReceived(
    GOMidiShortcutReceiver::MatchType matchType, int key) override;

  // Load all customizable values from the .cmb file
  void LoadFromCmb(GOConfigReader &cfg, uint8_t defaultValue);
  void Save(GOConfigWriter &cfg) override;

  void SendCurrentMidiValue() override { SendMidiValue(m_MIDIValue); }
  void SendEmptyMidiValue() override { SendMidiValue(0); }

  void SetIntEnclosureValue(int n) { SetEnclosureValue(std::clamp(n, 0, 127)); }

  /**
   * Logs a warning if a shelf is ignored because of its frequency.
   * @param shelfName the translated name of the shelf for the message
   * @param frequency the shelf frequency, Hz
   * @param attenuationDb the shelf attenuation, dB
   */
  void WarnIfShelfIgnored(
    const wxString &shelfName, float frequency, float attenuationDb) const;

  /**
   * Stores the given values into the shelf fields (an absent one stays as it
   * is) and marks the organ model as modified if any value changed.
   * @param newFrequency the frequency to be set, if any
   * @param newAttenuationDb the attenuation to be set, if any
   * @param frequency the shelf frequency field to be updated, Hz
   * @param attenuationDb the shelf attenuation field to be updated, dB
   */
  void UpdateShelf(
    std::optional<double> newFrequency,
    std::optional<double> newAttenuationDb,
    float &frequency,
    float &attenuationDb);

public:
  static constexpr uint8_t MAX_MIDI_VALUE = 127;

  /**
   * The upper bound of a shelf frequency (Hz) accepted from an ODF, a CMB or
   * the Organ Settings dialog. It is deliberately far above any Nyquist
   * frequency: a corner at or above sampleRate / 2 is rejected by the DSP
   * (GOSoundOnePoleFilter::computeCoeffs()), because the sample rate can
   * change.
   */
  static constexpr float MAX_SHELF_FREQUENCY = 100000.0f;

  /** The upper bound of a shelf attenuation (dB) */
  static constexpr float MAX_SHELF_ATTENUATION_DB = 121.0f;

  /**
   * Every Validate* method returns a translated explanation of why a value
   * is not valid, and an EMPTY STRING if the value is valid.
   *
   * @param frequency a shelf corner frequency, Hz
   * @return why the frequency is not greater than 0 and not greater than
   *   MAX_SHELF_FREQUENCY, or an empty string
   */
  static wxString ValidateShelfFrequency(double frequency);

  /**
   * @param attenuationDb a shelf attenuation, dB
   * @return why the attenuation is not in the range from 0 to
   *   MAX_SHELF_ATTENUATION_DB, or an empty string
   */
  static wxString ValidateShelfAttenuationDb(double attenuationDb);

  /**
   * A shelf with an attenuation greater than 0 needs a frequency greater than
   * 0, otherwise it has no corner and cannot work.
   * @param frequency a shelf corner frequency, Hz
   * @param attenuationDb the attenuation of the same shelf, dB
   * @return why the frequency and the attenuation are inconsistent, or an
   *   empty string
   */
  static wxString ValidateShelfPair(double frequency, double attenuationDb);

  GOEnclosure(GOOrganModel &organModel);

  bool IsOdfDefined() const { return m_IsOdfDefined; }

  using GOMidiObjectWithShortcut::Init; // for avoiding a warning
  void Init(
    GOConfigReader &cfg,
    const wxString &group,
    const wxString &name,
    uint8_t defValue);
  using GOMidiObjectWithShortcut::Load; // for avoiding a warning
  void Load(GOConfigReader &cfg, const wxString &group);

  uint8_t GetDefaultAmpMinimumLevel() const { return m_DefaultAmpMinimumLevel; }
  uint8_t GetAmpMinimumLevel() const { return m_AmpMinimumLevel; }
  /**
   * Sets the minimum amplitude level. The organ model is marked as modified
   * if the value differs from the current one. Not used by Load() and
   * LoadFromCmb(), which set the field directly, so loading never marks the
   * organ as modified.
   * @param v the new minimum amplitude level in percent
   */
  void SetAmpMinimumLevel(uint8_t v);
  void SetEnclosureValue(uint8_t n);
  int GetEnclosureValue() const { return m_MIDIValue; }
  float GetAttenuation() {
    return InterpolateByMidiValue(m_AmpMinimumLevel / 100.0f, 1.0f);
  }

  float GetDefaultLowShelfFrequency() const {
    return m_DefaultLowShelfFrequency;
  }
  float GetDefaultLowShelfAttenuationDb() const {
    return m_DefaultLowShelfAttenuationDb;
  }
  float GetDefaultHighShelfFrequency() const {
    return m_DefaultHighShelfFrequency;
  }
  float GetDefaultHighShelfAttenuationDb() const {
    return m_DefaultHighShelfAttenuationDb;
  }

  float GetLowShelfFrequency() const { return m_LowShelfFrequency; }
  void SetLowShelfFrequency(float v) { m_LowShelfFrequency = v; }
  float GetLowShelfAttenuationDb() const { return m_LowShelfAttenuationDb; }
  void SetLowShelfAttenuationDb(float v) { m_LowShelfAttenuationDb = v; }
  float GetHighShelfFrequency() const { return m_HighShelfFrequency; }
  void SetHighShelfFrequency(float v) { m_HighShelfFrequency = v; }
  float GetHighShelfAttenuationDb() const { return m_HighShelfAttenuationDb; }
  void SetHighShelfAttenuationDb(float v) { m_HighShelfAttenuationDb = v; }

  /**
   * Checks whether the low shelf would be consistent after setting the given
   * values; an absent value stays as it is now.
   * @param newFrequency the frequency to be set, if any
   * @param newAttenuationDb the attenuation to be set, if any
   * @return the result of ValidateShelfPair() for the resulting values: why
   *   they are inconsistent, or an empty string if they are valid
   */
  wxString ValidateLowShelfWith(
    std::optional<double> newFrequency,
    std::optional<double> newAttenuationDb) const;

  /**
   * The same as ValidateLowShelfWith() but for the high shelf.
   * @return why the resulting values are inconsistent, or an empty string if
   *   they are valid
   */
  wxString ValidateHighShelfWith(
    std::optional<double> newFrequency,
    std::optional<double> newAttenuationDb) const;

  /**
   * Sets the given low shelf values (an absent one stays as it is) and then
   * logs a warning if the resulting shelf is ignored because of its
   * frequency, see WarnIfShelvesIgnored(). The organ model is marked as
   * modified only if a value actually changes. Not used by Load() and
   * LoadFromCmb(), so loading never marks the organ as modified. The caller
   * must validate the values with ValidateLowShelfWith() first.
   * @param newFrequency the frequency to be set, if any
   * @param newAttenuationDb the attenuation to be set, if any
   */
  void ApplyLowShelf(
    std::optional<double> newFrequency, std::optional<double> newAttenuationDb);

  /** The same as ApplyLowShelf() but for the high shelf. */
  void ApplyHighShelf(
    std::optional<double> newFrequency, std::optional<double> newAttenuationDb);

  /**
   * Logs a warning for each shelf that has an attenuation greater than 0 but
   * is ignored by the sound engine because its frequency is not below the
   * Nyquist frequency of the current sample rate (the DSP leaves such a shelf
   * inactive). The warning names this enclosure and the ignored shelf. It is
   * the only place that checks it. Must not be called from the audio thread.
   */
  void WarnIfShelvesIgnored() const;

  /** Warns about ignored shelves because the sample rate may have changed. */
  void PreparePlayback() override;

  /** @return the low shelf's current effective gain in dB, ready to pass
   * straight into GOSoundShelfFilterProcessor::SetLowShelf(): 0 (no-op) at
   * fully open, -m_LowShelfAttenuationDb at fully closed, linear in
   * between. */
  float GetCurrentLowShelfGainDb() const {
    return InterpolateByMidiValue(-m_LowShelfAttenuationDb, 0.0f);
  }
  /** Same contract as GetCurrentLowShelfGainDb(), for the high band. */
  float GetCurrentHighShelfGainDb() const {
    return InterpolateByMidiValue(-m_HighShelfAttenuationDb, 0.0f);
  }

  bool IsKeyboardInputUsed() const override {
    return GOMidiObjectWithShortcut::IsKeyboardInputUsed();
  }

  void Scroll(bool scroll_up);
  bool IsDisplayed(bool new_format);

  wxString GetElementStatus() override;
  std::vector<wxString> GetElementActions() override;
  void TriggerElementActions(unsigned no) override;
};

#endif /* GOENCLOSURE_H_ */
