/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDDEVINFO_H
#define GOSOUNDDEVINFO_H

#include <wx/string.h>

class GOSoundDevInfo {
private:
  static const GOSoundDevInfo INVALID_DEVICE_INFO;

  wxString m_PortName;
  wxString m_ApiName;
  wxString m_name;
  wxString m_FullName;
  wxString m_DefaultLogicalName;
  wxString m_DefaultNameRegex;
  unsigned m_channels;
  bool m_IsDefault;

  GOSoundDevInfo(
    const wxString &portName,
    const wxString &apiName,
    const wxString &name,
    const wxString &fullName,
    unsigned channels,
    bool isDefault,
    void fillLogicalNameAndRegEx(GOSoundDevInfo &info));

public:
  GOSoundDevInfo(
    const wxString &portName,
    const wxString &apiName,
    const wxString &name,
    unsigned channels,
    bool isDefault,
    void fillLogicalNameAndRegEx(GOSoundDevInfo &info) = nullptr);

  const wxString &GetPortName() const { return m_PortName; }
  const wxString &GetApiName() const { return m_ApiName; }
  const wxString &GetName() const { return m_name; }
  const wxString &GetFullName() const { return m_FullName; }
  unsigned GetChannels() const { return m_channels; }
  bool IsValid() const { return m_channels > 0; }
  bool IsDefault() const { return m_IsDefault; }

  const wxString &GetDefaultLogicalName() const { return m_DefaultLogicalName; }
  void SetDefaultLogicalName(const wxString &name) {
    m_DefaultLogicalName = name;
  }

  const wxString &GetDefaultNameRegex() const { return m_DefaultNameRegex; }
  void SetDefaultNameRegex(const wxString &regEx) {
    m_DefaultNameRegex = regEx;
  }

  static const GOSoundDevInfo &getInvalideDeviceInfo() {
    return INVALID_DEVICE_INFO;
  }
};

#endif
