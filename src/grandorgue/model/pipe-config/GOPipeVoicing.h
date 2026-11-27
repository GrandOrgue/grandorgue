/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOPIPEVOICING_H
#define GOPIPEVOICING_H

#include <cstdint>
#include <map>
#include <optional>
#include <vector>

class GOPipeConfig;
class GOPipeConfigNode;

/**
 * Live voicing of the pipes in some subtrees of the pipe config tree (ranks,
 * windchests or the whole organ).
 *
 * Every change is applied to the pipes at once, so a playing note follows it
 * without any Apply. The settings of the pipes at SetVoicedNodes() are saved,
 * so that RevertVoicedPipeSettings() can restore them.
 *
 * The values are the own settings of the pipes as they are stored in their
 * configs. The voicing works on top of the values inherited from the rank,
 * the windchest and the organ: the gain and the tunings of a pipe are added
 * to the inherited ones, and a non-zero tone balance replaces the inherited
 * one. The inherited values themselves are never modified.
 *
 * The voicing keeps references to the voiced config nodes, so it must not
 * outlive them: its owner must destroy it before the organ is unloaded.
 */
class GOPipeVoicing {
public:
  /** Selects which pipe setting the voicing reads or changes */
  enum ChangedSettingSelector {
    SETTING_GAIN,                   // dB
    SETTING_MANUAL_TUNING,          // cents
    SETTING_AUTO_TUNING_CORRECTION, // cents
    SETTING_TONE_BALANCE            // -100..100, 0 means inherited
  };

private:
  /** The settings of a pipe that the voicing changes */
  struct PipeSettings {
    /** The gain of the pipe in dB */
    float gain;
    /** The manual tuning of the pipe in cents */
    float manualTuning;
    /** The auto-tuning correction of the pipe in cents */
    float autoTuningCorrection;
    /** The own tone balance of the pipe, -100..100; 0 means inherited */
    int8_t toneBalance;

    bool operator==(const PipeSettings &) const = default;
  };

  /**
   * A reference to a voiced pipe with its settings saved at SetVoicedNodes()
   */
  struct VoicedPipeRef {
    /** The config node of the pipe */
    GOPipeConfigNode &pipeNode;
    /** The settings of the pipe at SetVoicedNodes() */
    PipeSettings savedSettings;
  };

  /**
   * The references to all voiced pipes in the depth-first order of the
   * subtrees passed to SetVoicedNodes(), each pipe once
   */
  std::vector<VoicedPipeRef> m_VoicedPipeRefs;
  /**
   * The midi note number -> the indices in m_VoicedPipeRefs of the voiced pipes
   * with this note, in ascending order. Filled together with m_VoicedPipeRefs
   * by SetVoicedNodes(), so every index is valid
   */
  std::map<unsigned, std::vector<unsigned>> m_VoicedPipeRefIndicesByMidiNote;

  /**
   * Appends the config nodes of the pipes in a subtree, including its root, in
   * the depth-first order. Does not clear the vector
   * @param subtreeRoot the root of the subtree: a pipe, a rank, a windchest or
   *   the organ root
   * @param ioMpPipeNodes the vector to append the pipe nodes to
   */
  static void appendSubtreePipeNodes(
    GOPipeConfigNode &subtreeRoot,
    std::vector<GOPipeConfigNode *> &ioMpPipeNodes);

  /**
   * Takes the current values of the pipe settings that the voicing changes
   * @param pipeConfig the config of a pipe
   * @return the values as they are stored in the config
   */
  static PipeSettings takePipeSettings(const GOPipeConfig &pipeConfig);

  /**
   * Takes the value of one pipe setting
   * @param pipeConfig the config of a pipe
   * @param selector the setting to take
   * @return the value of the setting
   */
  static float takeSettingValue(
    const GOPipeConfig &pipeConfig, ChangedSettingSelector selector);

  /**
   * Applies the value of one pipe setting, clamped to the allowed range. Does
   * nothing if the setting already has this value, so that an unchanged
   * setting neither notifies the sound nor marks the organ modified
   * @param selector the setting to apply
   * @param value the new value of the setting
   * @param ioPipeConfig the config of a pipe to apply the value to
   */
  static void applySettingValue(
    ChangedSettingSelector selector, float value, GOPipeConfig &ioPipeConfig);

  /**
   * Applies the values of the pipe settings that differ from the current ones,
   * so that the unchanged settings neither notify the sound nor mark the organ
   * modified
   * @param settings the values to apply
   * @param ioPipeConfig the config of a pipe to apply the values to
   */
  static void applyPipeSettings(
    const PipeSettings &settings, GOPipeConfig &ioPipeConfig);

public:
  /**
   * Replaces the voiced pipes with the pipes in the subtrees of the nodes and
   * saves their current settings for IsAnyVoicedPipeModified() and
   * RevertVoicedPipeSettings(). A pipe in several of the subtrees is voiced
   * once. The previously voiced pipes are not reverted: their changes stay,
   * and RevertVoicedPipeSettings() can no longer restore them
   * @param mpSubtreeRoots the roots of the subtrees, not nullptr
   */
  void SetVoicedNodes(const std::vector<GOPipeConfigNode *> &mpSubtreeRoots);

  /** @return true if at least one pipe is voiced */
  bool HasVoicedPipes() const { return !m_VoicedPipeRefs.empty(); }

  /**
   * @return true if the settings of any voiced pipe differ from the ones saved
   *   at SetVoicedNodes()
   */
  bool IsAnyVoicedPipeModified() const;

  /** @return the midi note numbers of the voiced pipes in ascending order */
  std::vector<unsigned> GetVoicedMidiNotes() const;

  /**
   * Takes the value of a setting of the voiced pipes with the midi note
   * @param midiNote the midi note number, one of GetVoicedMidiNotes()
   * @param selector the setting to take
   * @return the value if all voiced pipes with the note have the same one,
   *   empty otherwise
   */
  std::optional<float> TakeSettingValueFromMidiNote(
    unsigned midiNote, ChangedSettingSelector selector) const;

  /**
   * Applies the value of a setting to every voiced pipe with the midi note
   * @param midiNote the midi note number, one of GetVoicedMidiNotes()
   * @param selector the setting to apply
   * @param value the new value, clamped to the allowed range
   */
  void ApplySettingValueToMidiNote(
    unsigned midiNote, ChangedSettingSelector selector, float value);

  /**
   * Applies the same value of a setting to all voiced pipes
   * @param selector the setting to apply
   * @param value the new value, clamped to the allowed range
   */
  void ApplySettingValueToAllPipes(
    ChangedSettingSelector selector, float value);

  /**
   * Applies the values of a setting growing linearly from firstValue at
   * firstMidiNote to lastValue at lastMidiNote to the voiced pipes with the
   * notes in this range, both ends inclusive. The notes without voiced pipes
   * are skipped
   * @param firstMidiNote the first midi note number of the range
   * @param lastMidiNote the last midi note number of the range,
   *   >= firstMidiNote
   * @param selector the setting to apply
   * @param firstValue the value at firstMidiNote, clamped to the allowed range
   * @param lastValue the value at lastMidiNote, clamped to the allowed range
   */
  void ApplySettingRampToMidiNotes(
    unsigned firstMidiNote,
    unsigned lastMidiNote,
    ChangedSettingSelector selector,
    float firstValue,
    float lastValue);

  /**
   * Reverts the settings of the voiced pipes to the ones saved at
   * SetVoicedNodes(). Only the differing settings are applied, so that the
   * unchanged ones neither notify the sound nor mark the organ modified
   */
  void RevertVoicedPipeSettings();
};

#endif /* GOPIPEVOICING_H */
