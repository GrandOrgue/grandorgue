/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOPORTFACTORY_H
#define GOPORTFACTORY_H

#include <wx/string.h>

#include <vector>

class GOPortFactory {
public:
  static const std::vector<wxString>
    c_NoApis; // empty Api list for subsystems not supporting them
  static const wxString c_NameDelim;

  class NameParser {
  private:
    const wxString &m_Name;
    size_t m_Pos;

  public:
    NameParser(const wxString &name)
      : m_Name(name), m_Pos(name.IsEmpty() ? wxString::npos : 0) {}
    NameParser(const NameParser &src) : m_Name(src.m_Name), m_Pos(src.m_Pos) {}

    const wxString &GetOrigName() const { return m_Name; }
    bool hasMore() const { return m_Pos != wxString::npos; }

    const wxString GetRestName() const {
      return hasMore() ? m_Name.substr(m_Pos) : wxT("");
    }

    wxString nextComp();
  };

  bool IsToUsePortName() const { return GetPortNames().size() > 1; }

  virtual const std::vector<wxString> &GetPortNames() const = 0;
  virtual const std::vector<wxString> &GetPortApiNames(
    const wxString &portName) const = 0;

  wxString ComposeDeviceName(
    wxString const &portName, wxString const &apiName, wxString const &devName);
};

#endif /* GOPORTFACTORY_H */
