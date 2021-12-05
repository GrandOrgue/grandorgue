/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOSETTINGSPORTS_H
#define GOSETTINGSPORTS_H

#include <vector>

#include <wx/sizer.h>
#include <wx/string.h>
#include <wx/treelist.h>

#include "GOPortFactory.h"
#include "GOPortsConfig.h"

class GOSettingsPorts
{
private:
  const GOPortFactory& m_PortFactory;
  GOPortsConfig m_PortsConfig;
  wxSizer* m_PortsSizer;
  wxTreeListCtrl* m_Ports;

  wxString GetPortItemName(
    const wxString &portName, const wxString &apiName = wxEmptyString
  ) const;
  bool GetPortItemChecked(
    const wxString &portName, const wxString& apiName = wxEmptyString
  ) const;
  void SetPortItemChecked(wxTreeListItem item, bool isChecked);
  
public:
  GOSettingsPorts(wxWindow* parent, const GOPortFactory& portFactory, const wxString& name);
  wxSizer* GetPortsBox() { return m_PortsSizer; }

  void FillPortsWith(const GOPortsConfig& config);
  GOPortsConfig& RenewPortsConfig();
};

#endif /* GOSETTINGSPORTS_H */
