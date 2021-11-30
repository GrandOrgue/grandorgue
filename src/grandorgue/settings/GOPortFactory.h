/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOPORTFACTORY_H
#define GOPORTFACTORY_H

#include <vector>

#include <wx/string.h>

class GOPortFactory
{
public:
  static const std::vector<wxString> c_NoApis; // empty Api list for subsystems not supporting them

  bool IsToUsePortName() const { return GetPortNames().size() > 1; }

  virtual const std::vector<wxString>& GetPortNames() const = 0;
  virtual const std::vector<wxString>& GetPortApiNames(const wxString & portName) const = 0;
};

#endif /* GOPORTFACTORY_H */

