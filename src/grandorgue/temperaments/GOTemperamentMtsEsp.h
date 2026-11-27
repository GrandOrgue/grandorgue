/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTEMPERAMENTMTSESP_H
#define GOTEMPERAMENTMTSESP_H

#include <functional>
#include <memory>

#include "GOTemperament.h"

struct MTSClient;

/**
 * A temperament that follows an MTS-ESP tuning master (ODDSound MTS-ESP).
 *
 * The offset of a pipe is the master's retuning of the pipe's MIDI note number
 * from the equal temperament. Without a master it is 0 for every note, so the
 * organ plays in the equal temperament at A = 440 Hz. The offsets change
 * whenever the master changes its tuning, so the temperament is live. Its
 * status tells whether a master is connected and the name of its scale.
 *
 * The temperament reading the master owns the MTS-ESP client: it registers it
 * when it first reads a tuning and deregisters it when it is destroyed, while
 * the MTS-ESP library is still loaded (deregistering at process exit crashes
 * on Windows, where the library is already detached by then).
 *
 * The offsets are read on the main thread only (temperament selection, MIDI
 * events and the organ timer all run there), so the first reading registers
 * the client without locking.
 */
class GOTemperamentMtsEsp : public GOTemperament {
public:
  /**
   * Returns the retuning in semitones of a MIDI note number 0-127 from the
   * equal temperament
   */
  using GetRetuningFunction = std::function<double(unsigned noteNumber)>;

  /**
   * Returns whether a master is connected
   */
  using HasMasterFunction = std::function<bool()>;

  /**
   * Returns the name of the master's scale, empty if it has none
   */
  using GetScaleNameFunction = std::function<wxString()>;

  /**
   * The highest MIDI note number an MTS-ESP master gives a tuning for
   */
  static constexpr unsigned MAX_NOTE_NUMBER = 127;

  /**
   * The longest word of the master's scale name shown in the status: the
   * temperament label wraps at spaces only, so a longer word would overflow it
   */
  static constexpr unsigned MAX_WORD_LENGTH = 18;

private:
  using ClientPtr = std::unique_ptr<MTSClient, void (*)(MTSClient *)>;

  GetRetuningFunction f_GetRetuning;
  HasMasterFunction f_HasMaster;
  GetScaleNameFunction f_GetScaleName;
  // registered on the first reading of the master (GetOffset and GetStatus
  // are const, the readings go through the functions above); empty with a
  // given source
  ClientPtr o_client;

  /**
   * Returns the client, registering it first if it is not registered yet
   */
  MTSClient *EnsureClientRegistered();

public:
  /**
   * Creates the temperament reading the MTS-ESP master of this process
   */
  GOTemperamentMtsEsp();

  /**
   * Creates the temperament reading the given source (for tests). By default
   * the source has no master
   */
  explicit GOTemperamentMtsEsp(
    GetRetuningFunction fGetRetuning,
    HasMasterFunction fHasMaster = [] { return false; },
    GetScaleNameFunction fGetScaleName = [] { return wxString(); });

  // the reading of the master refers to this object, so it is never copied
  GOTemperamentMtsEsp(const GOTemperamentMtsEsp &) = delete;
  GOTemperamentMtsEsp &operator=(const GOTemperamentMtsEsp &) = delete;

  /**
   * Returns the master's retuning of the note in cents. A note number above
   * MAX_NOTE_NUMBER is folded down by octaves to the highest note with the same
   * pitch class
   */
  float GetOffset(unsigned noteNumber) const override;

  /**
   * The master may change the tuning at any time, so the organ re-reads it
   * periodically
   */
  bool IsLive() const override { return true; }

  /**
   * The organ follows the master's pitch, not its own organ-wide pitch
   */
  bool IsAbsolutePitch() const override { return true; }

  /**
   * Returns the name of the master's scale ("connected" if it has none), or
   * "no master". Words of the name longer than MAX_WORD_LENGTH are shortened
   */
  wxString GetStatus() const override;

  /**
   * Returns the note number in 0-MAX_NOTE_NUMBER that the tuning of the note is
   * read from
   */
  static unsigned foldNoteNumber(unsigned noteNumber);

  /**
   * Returns the name with every word longer than MAX_WORD_LENGTH cut to that
   * length, ending with "..."
   */
  static wxString shortenLongWords(const wxString &name);
};

#endif /* GOTEMPERAMENTMTSESP_H */
