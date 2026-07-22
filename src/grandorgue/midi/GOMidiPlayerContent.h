/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIPLAYERCONTENT_H
#define GOMIDIPLAYERCONTENT_H

#include <wx/string.h>

#include <vector>

#include "config/GOConfig.h"

class GOMidiEvent;
class GOMidiMap;

class GOMidiPlayerContent {
private:
  std::vector<GOMidiEvent> m_Events;
  unsigned m_Pos;

  void SetupManual(GOMidiMap &map, unsigned channel, wxString ID);

public:
  GOMidiPlayerContent();
  virtual ~GOMidiPlayerContent();

  void Clear();
  void Reset();
  bool IsLoaded();

  /**
   * Checks whether events start with GrandOrgue's own MIDI_SYSEX_GO_CLEAR
   * setup header, i.e. the file was produced by GrandOrgue itself.
   * @param events the MIDI events read from the file, in time order
   * @return true iff events is non-empty and starts with MIDI_SYSEX_GO_CLEAR
   */
  static bool hasNativeHeader(const std::vector<GOMidiEvent> &events);

  /**
   * Checks whether midiInputNumbers can be used directly as MIDI channels.
   * @param midiInputNumbers see computeManualChannels()
   * @return true iff every value is in [1, 16] and all values are distinct
   */
  static bool isMidiInputNumberMappingUsable(
    const std::vector<int> &midiInputNumbers);

  /**
   * Computes the MIDI channel to bind to each manual for the given mapping
   * mode. Falls back to MIDI_PLAY_CHANNELS_PEDAL_LAST if mappingMode is
   * MIDI_PLAY_CHANNELS_USE_INPUT_NUMBER but isMidiInputNumberMappingUsable()
   * would return false for midiInputNumbers.
   * @param hasPedal whether the first element of midiInputNumbers is the
   *   pedal (rather than the first manual)
   * @param midiInputNumbers the manuals' ODF-configured MIDIInputNumber
   *   (1-based; -1 if the ODF doesn't configure one, see
   *   GOMidiReceivingSendingObject::GetMidiInputNumber()), ordered
   *   [pedal (if hasPedal), manual1, manual2, ...]
   * @param mappingMode the scheme to use to assign channels
   * @return the 1-based MIDI channel for each entry of midiInputNumbers, in
   *   the same order
   */
  static std::vector<unsigned> computeManualChannels(
    bool hasPedal,
    const std::vector<int> &midiInputNumbers,
    GOConfig::MidiFileChannelMapping mappingMode);

  /**
   * Loads events (already read from a MIDI file), injecting a synthetic
   * channel-mapping setup if events lacks GrandOrgue's own header.
   * @param events the MIDI events read from the file, in time order
   * @param map the MIDI map used to resolve recorder element names
   * @param hasPedal see computeManualChannels()
   * @param midiInputNumbers see computeManualChannels()
   * @param mappingMode see computeManualChannels()
   * @return true on success
   */
  bool Load(
    const std::vector<GOMidiEvent> &events,
    GOMidiMap &map,
    bool hasPedal,
    const std::vector<int> &midiInputNumbers,
    GOConfig::MidiFileChannelMapping mappingMode);

  const GOMidiEvent &GetCurrentEvent();
  bool Next();
};

#endif
