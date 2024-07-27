/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GODEVICENAMEPATTERN_H
#define GODEVICENAMEPATTERN_H

#include <wx/string.h>

class wxRegEx;

class GOConfigReader;
class GOConfigWriter;

class GODeviceNamePattern {
private:
  wxRegEx *m_CompiledRegEx = nullptr;

  wxString m_LogicalName;
  wxString m_RegEx;
  wxString m_PortName;
  wxString m_ApiName;

  // matchingResult. Not saved, only in memory
  wxString m_PhysicalName;

public:
  GODeviceNamePattern() {}

  GODeviceNamePattern(
    const wxString &logicalName,
    const wxString &regEx = wxEmptyString,
    const wxString &portName = wxEmptyString,
    const wxString &apiName = wxEmptyString,
    const wxString &physicalName = wxEmptyString);

  GODeviceNamePattern(const GODeviceNamePattern &src) {
    AssignNamePattern(src);
  }

  ~GODeviceNamePattern();

  GODeviceNamePattern &operator=(const GODeviceNamePattern &src) {
    AssignNamePattern(src);
    return *this;
  }

  bool IsFilled() const { return !m_LogicalName.IsEmpty(); }

  const wxString &GetLogicalName() const { return m_LogicalName; }
  void SetLogicalName(const wxString &name) { m_LogicalName = name; }

  const wxString &GetRegEx() const { return m_RegEx; }
  void SetRegEx(const wxString &regEx);

  const wxString &GetPortName() const { return m_PortName; }
  void SetPortName(const wxString &name) { m_PortName = name; }

  const wxString &GetApiName() const { return m_ApiName; }
  void SetApiName(const wxString &name) { m_ApiName = name; }

  const wxString &GetPhysicalName() const { return m_PhysicalName; }
  void SetPhysicalName(const wxString &name) { m_PhysicalName = name; }

  void LoadNamePattern(
    GOConfigReader &cfg,
    const wxString &group,
    const wxString &prefix,
    const wxString &nameKey = wxEmptyString);

  void SaveNamePattern(
    GOConfigWriter &cfg,
    const wxString &group,
    const wxString &prefix,
    const wxString &nameKey = wxEmptyString) const;

  void AssignNamePattern(const GODeviceNamePattern &src);

  bool DoesMatch(const wxString &physicalName) const;
};

#endif /* GODEVICENAMEPATTERN_H */
