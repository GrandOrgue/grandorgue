/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDENCLOSURESHELFMAPPER_H
#define GOSOUNDENCLOSURESHELFMAPPER_H

#include "sound/processing/GOSoundProcessingPrmMapper.h"

class GOEnclosure;
class GOSoundShelfFilterProcessor;

/**
 * Drives one GOSoundShelfFilterProcessor from one GOEnclosure's position and
 * shelf configuration (issue #717: swell-box modeling beyond pure
 * amplitude). Lives in sound/mappers/ (not sound/processing/, which must
 * stay model-agnostic) since it depends on model/GOEnclosure.
 */
class GOSoundEnclosureShelfMapper : public GOSoundProcessingPrmMapper {
private:
  const GOEnclosure &r_Enclosure;

  // Everything last pushed into the target processor (r_processor, cast to
  // GOSoundShelfFilterProcessor& where used), so EnsureParametersUpToDate()
  // only calls SetLowShelf()/SetHighShelf() (expensive - recomputes
  // coefficients) when something actually changed, per those methods'
  // documented cost contract. m_LastMidiValue starts at -1, an
  // impossible MIDI value, so the first call always pushes both bands.
  int m_LastMidiValue = -1;
  float m_LastLowShelfFrequency = 0;
  float m_LastLowShelfAttenuationDb = 0;
  float m_LastHighShelfFrequency = 0;
  float m_LastHighShelfAttenuationDb = 0;

public:
  GOSoundEnclosureShelfMapper(
    const GOEnclosure &enclosure, GOSoundShelfFilterProcessor &processor);

  /** Re-reads r_Enclosure's position and shelf config; pushes SetLowShelf()/
   * SetHighShelf() to the target processor only for whichever band actually
   * changed since the last call (position changing affects both bands;
   * editing only one band's frequency/attenuation via Organ Settings
   * affects only that band). */
  void EnsureParametersUpToDate() override;
};

#endif /* GOSOUNDENCLOSURESHELFMAPPER_H */
