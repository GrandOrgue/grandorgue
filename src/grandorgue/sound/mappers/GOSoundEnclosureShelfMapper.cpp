/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundEnclosureShelfMapper.h"

#include "model/GOEnclosure.h"

#include "sound/effects/GOSoundShelfFilterProcessor.h"

GOSoundEnclosureShelfMapper::GOSoundEnclosureShelfMapper(
  const GOEnclosure &enclosure, GOSoundShelfFilterProcessor &processor)
  : GOSoundProcessingPrmMapper(processor), r_Enclosure(enclosure) {}

void GOSoundEnclosureShelfMapper::EnsureParametersUpToDate() {
  GOSoundShelfFilterProcessor &processor
    = static_cast<GOSoundShelfFilterProcessor &>(r_processor);
  const int midiValue = r_Enclosure.GetEnclosureValue();
  const bool hasMidiValueChanged = midiValue != m_LastMidiValue;
  const float lowShelfFrequency = r_Enclosure.GetLowShelfFrequency();
  const float lowShelfAttenuationDb = r_Enclosure.GetLowShelfAttenuationDb();
  const float highShelfFrequency = r_Enclosure.GetHighShelfFrequency();
  const float highShelfAttenuationDb = r_Enclosure.GetHighShelfAttenuationDb();

  if (
    hasMidiValueChanged || lowShelfFrequency != m_LastLowShelfFrequency
    || lowShelfAttenuationDb != m_LastLowShelfAttenuationDb) {
    m_LastLowShelfFrequency = lowShelfFrequency;
    m_LastLowShelfAttenuationDb = lowShelfAttenuationDb;
    processor.SetLowShelf(
      lowShelfFrequency, r_Enclosure.GetCurrentLowShelfGainDb());
  }
  if (
    hasMidiValueChanged || highShelfFrequency != m_LastHighShelfFrequency
    || highShelfAttenuationDb != m_LastHighShelfAttenuationDb) {
    m_LastHighShelfFrequency = highShelfFrequency;
    m_LastHighShelfAttenuationDb = highShelfAttenuationDb;
    processor.SetHighShelf(
      highShelfFrequency, r_Enclosure.GetCurrentHighShelfGainDb());
  }
  m_LastMidiValue = midiValue;
}
