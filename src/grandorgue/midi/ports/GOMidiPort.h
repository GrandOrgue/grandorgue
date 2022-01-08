/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIPORT_H
#define GOMIDIPORT_H

#include <wx/string.h>

class GOMidi;

class GOMidiPort {
 protected:
  GOMidi* m_midi;
  bool m_IsActive;

  wxString m_PortName;
  wxString m_ApiName;
  wxString m_DeviceName;
  wxString m_FullName;

  unsigned m_ID;

  static const wxString GetClientName();
  virtual const wxString GetMyNativePortName() const = 0;

 public:
  GOMidiPort(GOMidi* midi, const wxString& portName, const wxString& apiName,
             const wxString& deviceName, const wxString& fullName);
  virtual ~GOMidiPort() {}

  const wxString& GetPortName() const { return m_PortName; }
  const wxString& GetApiName() const { return m_ApiName; }
  const wxString& GetDeviceName() const { return m_DeviceName; }
  const wxString& GetName() const { return m_FullName; }
  bool IsToUse() const;
  virtual const wxString GetDefaultLogicalName() const { return m_FullName; }
  virtual const wxString GetDefaultRegEx() const { return wxEmptyString; }
  bool IsEqualTo(const wxString& portName, const wxString& apiName,
                 const wxString& deviceName) const;
  unsigned GetID() const { return m_ID; }
  bool IsActive() const { return m_IsActive; }

  virtual bool Open() = 0;
  virtual void Close() = 0;
};

#endif /* GOMIDIPORT_H */
