/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTINGSPORTS_H
#define GOSETTINGSPORTS_H

#include <wx/sizer.h>
#include <wx/string.h>
#include <wx/treelist.h>

#include <vector>

#include "config/GOPortFactory.h"
#include "config/GOPortsConfig.h"

class GOSettingsPorts {
private:
  const GOPortFactory &m_PortFactory;

  struct PortItemData : public wxClientData {
    wxString m_PortName;
    wxString m_ApiName;

    PortItemData(
      const wxString &portName, const wxString &apiName = wxEmptyString) {
      m_PortName = portName;
      m_ApiName = apiName;
    }

    bool isItemForPortApi(
      const wxString &portName, const wxString &apiName = wxEmptyString) {
      return m_PortName == portName && m_ApiName == apiName;
    }
  };

  GOPortsConfig m_PortsConfig;
  wxSizer *m_PortsSizer;
  wxTreeListCtrl *m_Ports;

  wxString GetPortItemName(
    const wxString &portName, const wxString &apiName = wxEmptyString) const;

  const wxTreeListItem AddPortItem(
    const wxTreeListItem &parentItem,
    const wxString &portName,
    const wxString &apiName = wxEmptyString);

  bool GetPortItemChecked(
    const wxString &portName, const wxString &apiName = wxEmptyString) const;
  void SetPortItemChecked(wxTreeListItem item, bool isChecked);

  void OnPortItemChecked(wxTreeListEvent &event);

protected:
  virtual void OnPortChanged(
    const wxString &portName,
    const wxString apiName,
    bool oldEnabled,
    bool newEnabled) {}

public:
  GOSettingsPorts(
    wxWindow *parent, const GOPortFactory &portFactory, const wxString &name);
  ~GOSettingsPorts();

  wxSizer *GetPortsBox() { return m_PortsSizer; }

  void FillPortsWith(const GOPortsConfig &config);
  GOPortsConfig &RenewPortsConfig();
};

#endif /* GOSETTINGSPORTS_H */
