/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestTemperament.h"

#include <cmath>
#include <format>

#include <wx/intl.h>

#include "temperaments/GOTemperament.h"
#include "temperaments/GOTemperamentList.h"
#include "temperaments/GOTemperamentMtsEsp.h"

#include "GOTestScope.h"

const std::string GOTestTemperament::TEST_NAME = "GOTestTemperament";

/* pipe note numbers of a rank may run up to 255 and beyond 127 */
static const unsigned N_NOTES = 256;

/* most of the 104 built-in cent temperaments differ from the equal one */
static const unsigned MIN_N_TEMPERAMENTS_WITH_OFFSETS = 100;

void GOTestTemperament::TestOriginalHasNoOffset() {
  GOTemperamentList temperaments;

  temperaments.InitTemperaments();

  const GOTemperament &original = temperaments.GetTemperament(0u);

  GOAssert(
    original.IsTemperamentOriginalBased(),
    "The first temperament should be the original-based one");
  for (unsigned noteNumber = 0; noteNumber < N_NOTES; ++noteNumber)
    GOAssert(
      original.GetOffset(noteNumber) == 0,
      std::format(
        "The original temperament should have no offset for note {}",
        noteNumber));
}

void GOTestTemperament::TestCentOffsetsRepeatEveryOctave() {
  GOTemperamentList temperaments;

  temperaments.InitTemperaments();

  const unsigned nTemperaments = temperaments.GetTemperamentCount();
  unsigned nWithOffsets = 0;

  for (unsigned temperamentI = 0; temperamentI < nTemperaments;
       ++temperamentI) {
    const GOTemperament &temperament
      = temperaments.GetTemperament(temperamentI);
    bool hasOffset = false;

    // a live temperament reads an external source (the MTS-ESP master of the
    // machine), so its offsets are not fixed
    if (!temperament.IsLive()) {
      for (unsigned noteNumber = 0; noteNumber < N_NOTES; ++noteNumber) {
        const float offset = temperament.GetOffset(noteNumber);

        GOAssert(
          offset == temperament.GetOffset(noteNumber % 12),
          std::format(
            "{}: the offset of note {} should equal the one of note {}",
            temperament.GetName().ToStdString(),
            noteNumber,
            noteNumber % 12));
        if (offset != 0)
          hasOffset = true;
      }
    }
    if (hasOffset)
      ++nWithOffsets;
  }
  // the loop above really compared non-zero offsets
  GOAssert(
    nWithOffsets >= MIN_N_TEMPERAMENTS_WITH_OFFSETS,
    std::format(
      "At least {} of {} temperaments should have offsets, got {}",
      MIN_N_TEMPERAMENTS_WITH_OFFSETS,
      nTemperaments,
      nWithOffsets));
}

void GOTestTemperament::TestCentOffsetsByPitchClass() {
  GOTemperamentList temperaments;

  temperaments.InitTemperaments();

  // the table of this temperament starts with C: C 10.26, C# -13.69,
  // D# 9.78, G# -17.11
  const GOTemperament &temperament
    = temperaments.GetTemperament(wxT("Mersenne's Improved Meantone 1"));
  const bool isC = std::abs(temperament.GetOffset(60) - 10.26f) < 1e-3;
  const bool isCSharp = std::abs(temperament.GetOffset(61) + 13.69f) < 1e-3;
  const bool isDSharp = std::abs(temperament.GetOffset(255) - 9.78f) < 1e-3;
  const bool isGSharp = std::abs(temperament.GetOffset(128) + 17.11f) < 1e-3;

  GOAssert(
    isC && isCSharp && isDSharp && isGSharp,
    "A cent temperament should give each note the offset of its pitch class "
    "from the table");
}

void GOTestTemperament::TestMtsEspIsListed() {
  GOTemperamentList temperaments;

  temperaments.InitTemperaments();

  const GOTemperament &temperament
    = temperaments.GetTemperament(wxT("MTS-ESP"));

  GOAssert(
    temperament.GetName() == wxT("MTS-ESP") && temperament.IsLive(),
    "The temperament list should contain the live MTS-ESP temperament");
  GOAssert(
    !temperament.IsTemperamentOriginalBased(),
    "The MTS-ESP temperament should be based on the equal temperament");
}

void GOTestTemperament::TestMtsEspOffsetsByNoteNumber() {
  // a fake master: every note is retuned by its note number in cents
  const GOTemperamentMtsEsp temperament(
    [](unsigned noteNumber) { return noteNumber / 100.0; });

  for (unsigned noteNumber = 0;
       noteNumber <= GOTemperamentMtsEsp::MAX_NOTE_NUMBER;
       ++noteNumber)
    GOAssert(
      std::abs(temperament.GetOffset(noteNumber) - noteNumber) < 1e-3,
      std::format(
        "Note {} should get the master's retuning of note {} in cents, got {}",
        noteNumber,
        noteNumber,
        temperament.GetOffset(noteNumber)));
  GOAssert(
    std::abs(temperament.GetOffset(128) - 116) < 1e-3
      && std::abs(temperament.GetOffset(139) - 127) < 1e-3
      && std::abs(temperament.GetOffset(140) - 116) < 1e-3
      && std::abs(temperament.GetOffset(255) - 123) < 1e-3,
    "A note above 127 should be folded down by octaves into 116-127");
  for (unsigned noteNumber = 0; noteNumber < N_NOTES; ++noteNumber) {
    const unsigned maxNoteNumber = GOTemperamentMtsEsp::MAX_NOTE_NUMBER;
    const unsigned folded = GOTemperamentMtsEsp::foldNoteNumber(noteNumber);
    const bool isInRange = folded <= maxNoteNumber;
    const bool isSamePitchClass = folded % 12 == noteNumber % 12;
    const bool isHighestOrSame = noteNumber > maxNoteNumber
      ? folded + 12 > maxNoteNumber
      : folded == noteNumber;

    GOAssert(
      isInRange && isSamePitchClass && isHighestOrSame,
      std::format(
        "Note {} should fold to the highest note 0-127 of its pitch class, "
        "got {}",
        noteNumber,
        folded));
  }
}

void GOTestTemperament::TestMtsEspStatus() {
  bool hasMaster = false;
  wxString scaleName = wxT("Meantone");
  const GOTemperamentMtsEsp temperament(
    [](unsigned) { return 0.0; },
    [&hasMaster] { return hasMaster; },
    [&scaleName] { return scaleName; });

  GOAssert(
    GOTemperament(wxT("Fixed")).GetStatus().IsEmpty(),
    "A temperament without a source should have no status");
  GOAssert(
    temperament.GetStatus() == _("no master"),
    "Without a master the status should say so, not the local scale name");
  hasMaster = true;
  GOAssert(
    temperament.GetStatus() == wxT("Meantone"),
    "With a master the status should be the name of its scale");
  scaleName = wxT("werckmeister_III_well_temperament");
  GOAssert(
    temperament.GetStatus() == wxT("werckmeister_II..."),
    "A word of the scale name too long for the label should be shortened, "
    "got: "
      + temperament.GetStatus().ToStdString());
  scaleName = wxEmptyString;
  GOAssert(
    temperament.GetStatus() == _("connected"),
    "With a master whose scale has no name the status should be connected");
}

void GOTestTemperament::TestMtsEspShortenLongWords() {
  const wxString maxWord(wxT('a'), GOTemperamentMtsEsp::MAX_WORD_LENGTH);
  const wxString longWord = maxWord + wxT("b");
  const wxString shortenedWord
    = maxWord.Left(GOTemperamentMtsEsp::MAX_WORD_LENGTH - 3) + wxT("...");

  GOAssert(
    GOTemperamentMtsEsp::shortenLongWords(wxT("Andreas Werckmeister 1691"))
      == wxT("Andreas Werckmeister 1691"),
    "Words that fit should be kept");
  GOAssert(
    GOTemperamentMtsEsp::shortenLongWords(maxWord) == maxWord,
    "A word of the maximal length should be kept");
  GOAssert(
    GOTemperamentMtsEsp::shortenLongWords(
      wxT("1/4 ") + longWord + wxT("  meantone\\x"))
      == wxT("1/4 ") + shortenedWord + wxT("  meantone\\x"),
    "Only a too long word should be shortened, to the maximal length, the "
    "spaces and backslashes kept");
}

void GOTestTemperament::run() {
  GO_RUN_TEST(TestOriginalHasNoOffset())
  GO_RUN_TEST(TestCentOffsetsRepeatEveryOctave())
  GO_RUN_TEST(TestCentOffsetsByPitchClass())
  GO_RUN_TEST(TestMtsEspIsListed())
  GO_RUN_TEST(TestMtsEspOffsetsByNoteNumber())
  GO_RUN_TEST(TestMtsEspStatus())
  GO_RUN_TEST(TestMtsEspShortenLongWords())
}
