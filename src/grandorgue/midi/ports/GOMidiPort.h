/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOMIDIPORT_H
#define GOMIDIPORT_H

#include <wx/string.h>

class GOMidi;

class GOMidiPort
{
protected:
  GOMidi* m_midi;
  bool m_IsActive;
  wxString m_Name;
  wxString m_Prefix;
  unsigned m_ID;

  const wxString GetClientName() const;
  virtual const wxString GetPortName() const = 0;

public:
  GOMidiPort(GOMidi* midi, wxString prefix, wxString name);
  virtual ~GOMidiPort() {}

  const wxString& GetName() const { return m_Name; }
  unsigned GetID() const { return m_ID; }
  bool IsActive() const { return m_IsActive; }
};

#endif /* GOMIDIPORT_H */

