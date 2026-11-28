/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOPipeVoicing.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <ranges>
#include <set>

#include "GOPipeConfigNode.h"

static const float MIN_GAIN_DB = -120.0f;
static const float MAX_GAIN_DB = 40.0f;
static const float MAX_TUNING_CENTS = 1800.0f;
static const float MAX_TONE_BALANCE = 100.0f;

void GOPipeVoicing::appendSubtreePipeNodes(
  GOPipeConfigNode &subtreeRoot,
  std::vector<GOPipeConfigNode *> &ioMpPipeNodes) {
  if (subtreeRoot.GetMidiNoteNumber()) // a pipe node
    ioMpPipeNodes.push_back(&subtreeRoot);
  for (unsigned n = subtreeRoot.GetChildCount(), i = 0; i < n; i++)
    appendSubtreePipeNodes(*subtreeRoot.GetChild(i), ioMpPipeNodes);
}

GOPipeVoicing::PipeSettings GOPipeVoicing::takePipeSettings(
  const GOPipeConfig &pipeConfig) {
  return {
    pipeConfig.GetGain(),
    pipeConfig.GetManualTuning(),
    pipeConfig.GetAutoTuningCorrection(),
    pipeConfig.GetToneBalanceValue()};
}

float GOPipeVoicing::takeSettingValue(
  const GOPipeConfig &pipeConfig, ChangedSettingSelector selector) {
  float value = 0.0f;

  switch (selector) {
  case SETTING_GAIN:
    value = pipeConfig.GetGain();
    break;
  case SETTING_MANUAL_TUNING:
    value = pipeConfig.GetManualTuning();
    break;
  case SETTING_AUTO_TUNING_CORRECTION:
    value = pipeConfig.GetAutoTuningCorrection();
    break;
  case SETTING_TONE_BALANCE:
    value = (float)pipeConfig.GetToneBalanceValue();
    break;
  }
  return value;
}

void GOPipeVoicing::applySettingValue(
  ChangedSettingSelector selector, float value, GOPipeConfig &ioPipeConfig) {
  switch (selector) {
  case SETTING_GAIN: {
    const float gain = std::clamp(value, MIN_GAIN_DB, MAX_GAIN_DB);

    if (gain != ioPipeConfig.GetGain())
      ioPipeConfig.SetGain(gain);
    break;
  }
  case SETTING_MANUAL_TUNING: {
    const float cents = std::clamp(value, -MAX_TUNING_CENTS, MAX_TUNING_CENTS);

    if (cents != ioPipeConfig.GetManualTuning())
      ioPipeConfig.SetManualTuning(cents);
    break;
  }
  case SETTING_AUTO_TUNING_CORRECTION: {
    const float cents = std::clamp(value, -MAX_TUNING_CENTS, MAX_TUNING_CENTS);

    if (cents != ioPipeConfig.GetAutoTuningCorrection())
      ioPipeConfig.SetAutoTuningCorrection(cents);
    break;
  }
  case SETTING_TONE_BALANCE: {
    const int8_t toneBalance = (int8_t)std::lround(
      std::clamp(value, -MAX_TONE_BALANCE, MAX_TONE_BALANCE));

    if (toneBalance != ioPipeConfig.GetToneBalanceValue())
      ioPipeConfig.SetToneBalanceValue(toneBalance);
    break;
  }
  }
}

void GOPipeVoicing::applyPipeSettings(
  const PipeSettings &settings, GOPipeConfig &ioPipeConfig) {
  applySettingValue(SETTING_GAIN, settings.gain, ioPipeConfig);
  applySettingValue(SETTING_MANUAL_TUNING, settings.manualTuning, ioPipeConfig);
  applySettingValue(
    SETTING_AUTO_TUNING_CORRECTION,
    settings.autoTuningCorrection,
    ioPipeConfig);
  applySettingValue(
    SETTING_TONE_BALANCE, (float)settings.toneBalance, ioPipeConfig);
}

void GOPipeVoicing::SetVoicedNodes(
  const std::vector<GOPipeConfigNode *> &mpSubtreeRoots) {
  std::vector<GOPipeConfigNode *> mpPipeNodes;

  m_VoicedPipeRefs.clear();
  m_VoicedPipeRefIndicesByMidiNote.clear();

  // Collect the pipe nodes of all subtrees, possibly with duplicates
  for (GOPipeConfigNode *pSubtreeRoot : mpSubtreeRoots) {
    assert(pSubtreeRoot);
    appendSubtreePipeNodes(*pSubtreeRoot, mpPipeNodes);
  }

  // Remove the duplicates
  const std::set<GOPipeConfigNode *> mpDistinctPipeNodes(
    mpPipeNodes.begin(), mpPipeNodes.end());

  // Save the settings of each distinct pipe and index it by its midi note
  for (GOPipeConfigNode *pPipeNode : mpDistinctPipeNodes) {
    const unsigned midiNote = *pPipeNode->GetMidiNoteNumber();

    // No other thread changes m_VoicedPipeRefs (see the class comment), so
    // its current size is the index of the pipe ref pushed back below
    m_VoicedPipeRefIndicesByMidiNote[midiNote].push_back(
      m_VoicedPipeRefs.size());
    m_VoicedPipeRefs.push_back(
      {*pPipeNode, takePipeSettings(pPipeNode->GetPipeConfig())});
  }
}

bool GOPipeVoicing::IsAnyVoicedPipeModified() const {
  return std::ranges::any_of(
    m_VoicedPipeRefs, [](const VoicedPipeRef &voicedPipeRef) {
      return takePipeSettings(voicedPipeRef.pipeNode.GetPipeConfig())
        != voicedPipeRef.savedSettings;
    });
}

std::vector<unsigned> GOPipeVoicing::GetVoicedMidiNotes() const {
  const auto midiNotes = std::views::keys(m_VoicedPipeRefIndicesByMidiNote);

  return std::vector<unsigned>(midiNotes.begin(), midiNotes.end());
}

std::optional<float> GOPipeVoicing::TakeSettingValueFromMidiNote(
  unsigned midiNote, ChangedSettingSelector selector) const {
  const auto it = m_VoicedPipeRefIndicesByMidiNote.find(midiNote);
  std::optional<float> value;

  // The note must be one of GetVoicedMidiNotes() (see the cdoc)
  assert(it != m_VoicedPipeRefIndicesByMidiNote.end());

  // Guards against a broken contract in a release build
  if (it != m_VoicedPipeRefIndicesByMidiNote.end()) {
    // Never empty: SetVoicedNodes() adds a note together with its first pipe
    const std::vector<unsigned> &pipeRefIndices = it->second;
    const float firstValue = takeSettingValue(
      m_VoicedPipeRefs[pipeRefIndices.front()].pipeNode.GetPipeConfig(),
      selector);

    // Compare the value of every pipe of the note with the first one
    const bool isSameForAllPipes = std::ranges::all_of(
      pipeRefIndices, [this, selector, firstValue](unsigned pipeRefI) {
        return takeSettingValue(
                 m_VoicedPipeRefs[pipeRefI].pipeNode.GetPipeConfig(), selector)
          == firstValue;
      });

    if (isSameForAllPipes)
      value = firstValue;
  }
  return value;
}

void GOPipeVoicing::ApplySettingValueToMidiNote(
  unsigned midiNote, ChangedSettingSelector selector, float value) {
  const auto it = m_VoicedPipeRefIndicesByMidiNote.find(midiNote);

  // The note must be one of GetVoicedMidiNotes() (see the cdoc)
  assert(it != m_VoicedPipeRefIndicesByMidiNote.end());

  // Guards against a broken contract in a release build
  if (it != m_VoicedPipeRefIndicesByMidiNote.end())
    for (unsigned pipeRefI : it->second)
      applySettingValue(
        selector, value, m_VoicedPipeRefs[pipeRefI].pipeNode.GetPipeConfig());
}

void GOPipeVoicing::ApplySettingValueToAllPipes(
  ChangedSettingSelector selector, float value) {
  for (const VoicedPipeRef &voicedPipeRef : m_VoicedPipeRefs)
    applySettingValue(selector, value, voicedPipeRef.pipeNode.GetPipeConfig());
}

void GOPipeVoicing::ApplySettingRampToMidiNotes(
  unsigned firstMidiNote,
  unsigned lastMidiNote,
  ChangedSettingSelector selector,
  float firstValue,
  float lastValue) {
  assert(firstMidiNote <= lastMidiNote);

  const float noteSpan = (float)(lastMidiNote - firstMidiNote);
  const float valueSpan = lastValue - firstValue;
  const auto pipeRefIndicesByMidiNoteSubmap = std::ranges::subrange(
    m_VoicedPipeRefIndicesByMidiNote.lower_bound(firstMidiNote),
    m_VoicedPipeRefIndicesByMidiNote.upper_bound(lastMidiNote));

  // Only the voiced notes from firstMidiNote to lastMidiNote, both inclusive
  for (const auto &[midiNote, pipeRefIndices] :
       pipeRefIndicesByMidiNoteSubmap) {
    // A single-note range has no span, its only note takes firstValue
    const float newValue = noteSpan > 0.0f
      ? firstValue + valueSpan * (float)(midiNote - firstMidiNote) / noteSpan
      : firstValue;

    ApplySettingValueToMidiNote(midiNote, selector, newValue);
  }
}

void GOPipeVoicing::RevertVoicedPipeSettings() {
  for (const VoicedPipeRef &voicedPipeRef : m_VoicedPipeRefs)
    applyPipeSettings(
      voicedPipeRef.savedSettings, voicedPipeRef.pipeNode.GetPipeConfig());
}
