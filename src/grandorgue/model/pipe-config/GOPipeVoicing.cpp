/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOPipeVoicing.h"

#include <algorithm>
#include <cmath>
#include <set>

#include "GOPipeConfigNode.h"

static const float MIN_GAIN_DB = -120.0f;
static const float MAX_GAIN_DB = 40.0f;

float GOPipeVoicing::getOffsetOf(
  const GOPipeConfig &config, Parameter parameter) {
  float offset = 0.0f;

  switch (parameter) {
  case GAIN:
    offset = config.GetGain() - config.GetDefaultGain();
    break;
  case MANUAL_TUNING:
    offset = config.GetManualTuning();
    break;
  case AUTO_TUNING_CORRECTION:
    offset = config.GetAutoTuningCorrection();
    break;
  case TONE_BALANCE:
    offset = config.GetToneBalanceValue();
    break;
  }
  return offset;
}

void GOPipeVoicing::setOffsetOf(
  GOPipeConfig &config, Parameter parameter, float offset) {
  if (getOffsetOf(config, parameter) != offset) {
    switch (parameter) {
    case GAIN:
      config.SetGain(
        std::clamp(config.GetDefaultGain() + offset, MIN_GAIN_DB, MAX_GAIN_DB));
      break;
    case MANUAL_TUNING:
      config.SetManualTuning(offset);
      break;
    case AUTO_TUNING_CORRECTION:
      config.SetAutoTuningCorrection(offset);
      break;
    case TONE_BALANCE:
      config.SetToneBalanceValue(
        (int8_t)std::clamp(std::lround(offset), -100l, 100l));
      break;
    }
  }
}

void GOPipeVoicing::CollectPipes(
  GOPipeConfigNode &node, std::vector<GOPipeConfigNode *> &outPNodes) const {
  if (node.GetMidiNoteNumber() >= 0)
    outPNodes.push_back(&node);
  for (unsigned nChilds = node.GetChildCount(), childI = 0; childI < nChilds;
       childI++)
    CollectPipes(*node.GetChild(childI), outPNodes);
}

void GOPipeVoicing::Select(const std::vector<GOPipeConfigNode *> &mpNodes) {
  std::vector<GOPipeConfigNode *> mpPipeNodes;
  std::set<GOPipeConfigNode *> mpSeen;

  m_Pipes.clear();
  m_PipeIndicesByNote.clear();
  for (GOPipeConfigNode *pNode : mpNodes)
    if (pNode)
      CollectPipes(*pNode, mpPipeNodes);
  for (GOPipeConfigNode *pPipeNode : mpPipeNodes) {
    if (mpSeen.insert(pPipeNode).second) {
      const GOPipeConfig &config = pPipeNode->GetPipeConfig();

      m_PipeIndicesByNote[pPipeNode->GetMidiNoteNumber()].push_back(
        m_Pipes.size());
      m_Pipes.push_back(
        {pPipeNode,
         config.GetGain(),
         config.GetManualTuning(),
         config.GetAutoTuningCorrection(),
         config.GetToneBalanceValue()});
    }
  }
}

std::vector<int> GOPipeVoicing::GetNotes() const {
  std::vector<int> notes;

  for (const auto &entry : m_PipeIndicesByNote)
    notes.push_back(entry.first);
  return notes;
}

float GOPipeVoicing::GetOffset(Parameter parameter, int midiNoteNumber) const {
  const auto it = m_PipeIndicesByNote.find(midiNoteNumber);

  return it != m_PipeIndicesByNote.end()
    ? getOffsetOf(m_Pipes[it->second.front()].pNode->GetPipeConfig(), parameter)
    : 0.0f;
}

void GOPipeVoicing::SetOffset(
  Parameter parameter, int midiNoteNumber, float offset) {
  const auto it = m_PipeIndicesByNote.find(midiNoteNumber);

  if (it != m_PipeIndicesByNote.end())
    for (unsigned pipeI : it->second)
      setOffsetOf(m_Pipes[pipeI].pNode->GetPipeConfig(), parameter, offset);
}

void GOPipeVoicing::SetAllOffsets(Parameter parameter, float offset) {
  for (PipeEntry &entry : m_Pipes)
    setOffsetOf(entry.pNode->GetPipeConfig(), parameter, offset);
}

void GOPipeVoicing::SetRamp(
  Parameter parameter,
  int firstNote,
  int lastNote,
  float firstOffset,
  float lastOffset) {
  const float noteSpan = (float)(lastNote - firstNote);

  for (const auto &entry : m_PipeIndicesByNote) {
    const int note = entry.first;

    if (note >= firstNote && note <= lastNote) {
      const float ratio = noteSpan > 0 ? (note - firstNote) / noteSpan : 0.0f;

      SetOffset(
        parameter, note, firstOffset + (lastOffset - firstOffset) * ratio);
    }
  }
}

bool GOPipeVoicing::IsModified() const {
  bool isModified = false;

  for (const PipeEntry &entry : m_Pipes) {
    const GOPipeConfig &config = entry.pNode->GetPipeConfig();

    isModified = isModified || config.GetGain() != entry.gain
      || config.GetManualTuning() != entry.manualTuning
      || config.GetAutoTuningCorrection() != entry.autoTuningCorrection
      || config.GetToneBalanceValue() != entry.toneBalance;
  }
  return isModified;
}

void GOPipeVoicing::Reset() {
  for (PipeEntry &entry : m_Pipes) {
    GOPipeConfig &config = entry.pNode->GetPipeConfig();

    if (config.GetGain() != entry.gain)
      config.SetGain(entry.gain);
    if (config.GetManualTuning() != entry.manualTuning)
      config.SetManualTuning(entry.manualTuning);
    if (config.GetAutoTuningCorrection() != entry.autoTuningCorrection)
      config.SetAutoTuningCorrection(entry.autoTuningCorrection);
    if (config.GetToneBalanceValue() != entry.toneBalance)
      config.SetToneBalanceValue((int8_t)entry.toneBalance);
  }
}
