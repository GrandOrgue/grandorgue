/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOSOUNDPORTFACTORY_H
#define GOSOUNDPORTFACTORY_H

#include "settings/GOPortFactory.h"
#include "settings/GOPortsConfig.h"
#include "sound/GOSoundDevInfo.h"
#include "GOSoundPort.h"

class GOSoundPortFactory: public GOPortFactory
{
public:
  class NameParser
  {
  private:
    const wxString &m_Name;
    size_t m_Pos;

  public:
    NameParser(const wxString &name): m_Name(name), m_Pos(name.IsEmpty() ? wxString::npos : 0) { }
    NameParser(const NameParser &src): m_Name(src.m_Name), m_Pos(src.m_Pos) { }

    const wxString &GetOrigName() const { return m_Name; }
    bool hasMore() const { return m_Pos != wxString::npos; }

    const wxString GetRestName() const
    { return hasMore() ? m_Name.substr(m_Pos) : wxT(""); }

    wxString nextComp();
  };

  const std::vector<wxString>& GetPortNames() const;
  const std::vector<wxString>& GetPortApiNames(const wxString & portName) const;

  static wxString composeDeviceName(
    wxString const& portName,
    wxString const& apiName,
    wxString const& devName
  );
  
  static std::vector<GOSoundDevInfo> getDeviceList(const GOPortsConfig &portsConfig);
  static GOSoundPort* create(const GOPortsConfig &portsConfig, GOSound* sound, wxString name);

  static GOSoundPortFactory& getInstance();
  static void terminate();
};

#endif /* GOSOUNDPORTFACTORY_H */

