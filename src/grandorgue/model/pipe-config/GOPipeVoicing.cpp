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
#include <set>

#include "GOPipeConfigNode.h"

static const float MIN_GAIN_DB = -120.0f;
static const float MAX_GAIN_DB = 40.0f;
static const float MAX_TUNING_CENTS = 1800.0f;
static const float MAX_TONE_BALANCE = 100.0f;

void GOPipeVoicing::appendSubtreePipeNodes(
  GOPipeConfigNode &subtreeRoot,
  std::vector<GOPipeConfigNode *> &ioMpPipeNodes) {
  if (subtreeRoot.GetMidiNoteNumber())
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
    value = pipeConfig.GetToneBalanceValue();
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
  applySettingValue(SETTING_TONE_BALANCE, settings.toneBalance, ioPipeConfig);
}

void GOPipeVoicing::SetVoicedNodes(
  const std::vector<GOPipeConfigNode *> &mpSubtreeRoots) {
  std::vector<GOPipeConfigNode *> mpPipeNodes;
  std::set<GOPipeConfigNode *> mpSeenPipeNodes;

  m_VoicedPipeRefs.clear();
  m_VoicedPipeRefIndicesByMidiNote.clear();
  for (GOPipeConfigNode *pSubtreeRoot : mpSubtreeRoots) {
    assert(pSubtreeRoot);
    appendSubtreePipeNodes(*pSubtreeRoot, mpPipeNodes);
  }
  for (GOPipeConfigNode *pPipeNode : mpPipeNodes) {
    if (mpSeenPipeNodes.insert(pPipeNode).second) {
      const unsigned midiNote = *pPipeNode->GetMidiNoteNumber();

      m_VoicedPipeRefIndicesByMidiNote[midiNote].push_back(
        m_VoicedPipeRefs.size());
      m_VoicedPipeRefs.push_back(
        {*pPipeNode, takePipeSettings(pPipeNode->GetPipeConfig())});
    }
  }
}

bool GOPipeVoicing::IsAnyVoicedPipeModified() const {
  bool isModified = false;

  for (const VoicedPipeRef &voicedPipeRef : m_VoicedPipeRefs)
    isModified = isModified
      || voicedPipeRef.savedSettings
        != takePipeSettings(voicedPipeRef.pipeNode.GetPipeConfig());
  return isModified;
}

std::vector<unsigned> GOPipeVoicing::GetVoicedMidiNotes() const {
  std::vector<unsigned> midiNotes;

  for (const auto &entry : m_VoicedPipeRefIndicesByMidiNote)
    midiNotes.push_back(entry.first);
  return midiNotes;
}

std::optional<float> GOPipeVoicing::TakeSettingValueFromMidiNote(
  unsigned midiNote, ChangedSettingSelector selector) const {
  const auto it = m_VoicedPipeRefIndicesByMidiNote.find(midiNote);
  std::optional<float> value;

  assert(it != m_VoicedPipeRefIndicesByMidiNote.end());
  if (it != m_VoicedPipeRefIndicesByMidiNote.end()) {
    const std::vector<unsigned> &pipeRefIndices = it->second;
    const float firstValue = takeSettingValue(
      m_VoicedPipeRefs[pipeRefIndices.front()].pipeNode.GetPipeConfig(),
      selector);
    bool isSameForAllPipes = true;

    for (unsigned pipeRefI : pipeRefIndices)
      isSameForAllPipes = isSameForAllPipes
        && takeSettingValue(
             m_VoicedPipeRefs[pipeRefI].pipeNode.GetPipeConfig(), selector)
          == firstValue;
    if (isSameForAllPipes)
      value = firstValue;
  }
  return value;
}

void GOPipeVoicing::ApplySettingValueToMidiNote(
  unsigned midiNote, ChangedSettingSelector selector, float value) {
  const auto it = m_VoicedPipeRefIndicesByMidiNote.find(midiNote);

  assert(it != m_VoicedPipeRefIndicesByMidiNote.end());
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

  for (const auto &entry : m_VoicedPipeRefIndicesByMidiNote) {
    const unsigned midiNote = entry.first;

    if (midiNote >= firstMidiNote && midiNote <= lastMidiNote) {
      const float ratio
        = noteSpan > 0.0f ? (float)(midiNote - firstMidiNote) / noteSpan : 0.0f;

      ApplySettingValueToMidiNote(
        midiNote, selector, firstValue + (lastValue - firstValue) * ratio);
    }
  }
}

void GOPipeVoicing::RevertVoicedPipeSettings() {
  for (const VoicedPipeRef &voicedPipeRef : m_VoicedPipeRefs)
    applyPipeSettings(
      voicedPipeRef.savedSettings, voicedPipeRef.pipeNode.GetPipeConfig());
}
