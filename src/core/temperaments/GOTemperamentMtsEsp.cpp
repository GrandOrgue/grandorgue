/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTemperamentMtsEsp.h"

#include <libMTSClient.h>
#include <wx/arrstr.h>
#include <wx/intl.h>

GOTemperamentMtsEsp::GOTemperamentMtsEsp()
  : GOTemperamentMtsEsp(
    [this](unsigned noteNumber) {
      // channel -1: the master's tuning for all channels
      return MTS_RetuningInSemitones(
        EnsureClientRegistered(), (char)noteNumber, -1);
    },
    [this] { return MTS_HasMaster(EnsureClientRegistered()); },
    [this] {
      return wxString::FromUTF8(MTS_GetScaleName(EnsureClientRegistered()));
    }) {}

GOTemperamentMtsEsp::GOTemperamentMtsEsp(
  GetRetuningFunction fGetRetuning,
  HasMasterFunction fHasMaster,
  GetScaleNameFunction fGetScaleName)
  : GOTemperament(wxTRANSLATE("MTS-ESP"), wxEmptyString, false),
    f_GetRetuning(fGetRetuning),
    f_HasMaster(fHasMaster),
    f_GetScaleName(fGetScaleName),
    o_client(nullptr, MTS_DeregisterClient) {}

MTSClient *GOTemperamentMtsEsp::EnsureClientRegistered() {
  if (!o_client)
    o_client.reset(MTS_RegisterClient());
  return o_client.get();
}

unsigned GOTemperamentMtsEsp::foldNoteNumber(unsigned noteNumber) {
  // 116-127 is the highest octave of the MTS-ESP note range
  return noteNumber <= MAX_NOTE_NUMBER
    ? noteNumber
    : MAX_NOTE_NUMBER - 11 + (noteNumber - (MAX_NOTE_NUMBER - 11)) % 12;
}

wxString GOTemperamentMtsEsp::shortenLongWords(const wxString &name) {
  const wxString ellipsis = wxT("...");
  // '\0': no escape character, a backslash in the name stays as it is
  wxArrayString words = wxSplit(name, ' ', '\0');

  for (wxString &word : words)
    if (word.length() > MAX_WORD_LENGTH)
      word = word.Left(MAX_WORD_LENGTH - ellipsis.length()) + ellipsis;
  return wxJoin(words, ' ', '\0');
}

float GOTemperamentMtsEsp::GetOffset(unsigned noteNumber) const {
  return 100 * f_GetRetuning(foldNoteNumber(noteNumber));
}

wxString GOTemperamentMtsEsp::GetStatus() const {
  wxString status = _("no master");

  // the scale name is read only from a connected master: without one the
  // client gives the name of its own local table
  if (f_HasMaster()) {
    const wxString scaleName = f_GetScaleName();

    status = scaleName.IsEmpty() ? _("connected") : shortenLongWords(scaleName);
  }
  return status;
}
