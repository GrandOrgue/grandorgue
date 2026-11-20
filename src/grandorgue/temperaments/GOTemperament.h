/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTEMPERAMENT_H
#define GOTEMPERAMENT_H

#include <wx/string.h>

class GOTemperament {
protected:
  wxString m_Group;
  wxString m_GroupTitle;
  wxString m_Name;
  wxString m_Title;

private:
  bool
    m_IsTemperamentOriginalBased; // Decides the base for GetOffset(): Original
                                  // temperament or Equal temperament

public:
  GOTemperament(
    wxString name, wxString group = wxEmptyString, bool isOriginalBased = true);
  GOTemperament(
    wxString name,
    wxString title,
    wxString group,
    wxString groupTitle,
    bool isOriginalBased = true);
  virtual ~GOTemperament();

  // Returns the offset in cents of a pipe with the MIDI note number
  // noteNumber. If m_IsTemperamentOriginalBased it is the deviation from the
  // original temperament else the deviation from the equal temperament
  virtual float GetOffset(unsigned noteNumber) const;
  // Whether the offsets may change while the temperament is selected (an
  // external tuning source), so they have to be re-read periodically
  virtual bool IsLive() const { return false; }
  // Whether the offsets give the absolute pitch (an external tuning source):
  // then the organ-wide and windchest pitch corrections do not apply, so the
  // pipes sound at the pitch of the source
  virtual bool IsAbsolutePitch() const { return false; }
  // A short text on the current state of the temperament, shown after its
  // title in the temperament label of the organ (e.g. whether an external
  // tuning source is connected); empty if the temperament has no state
  virtual wxString GetStatus() const { return wxEmptyString; }
  wxString GetName() const;
  wxString GetTitle() const;
  wxString GetGroup() const;
  wxString GetGroupTitle() const;
  bool IsTemperamentOriginalBased() const {
    return m_IsTemperamentOriginalBased;
  }
};

#endif
