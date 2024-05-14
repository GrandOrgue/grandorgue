/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSettingsPorts.h"

#include <wx/treelist.h>

wxString GOSettingsPorts::GetPortItemName(
  const wxString &portName, const wxString &apiName) const {
  wxString itemName;

  if (m_PortFactory.IsToUsePortName())
    itemName = portName;

  if (!apiName.IsEmpty()) {
    if (!itemName.IsEmpty())
      itemName += ": ";
    itemName += apiName;
  }
  return itemName;
}

const wxTreeListItem GOSettingsPorts::AddPortItem(
  const wxTreeListItem &parentItem,
  const wxString &portName,
  const wxString &apiName) {
  return m_Ports->AppendItem(
    parentItem,
    GetPortItemName(portName, apiName),
    wxWithImages::NO_IMAGE,
    wxWithImages::NO_IMAGE,
    new PortItemData(portName, apiName));
}

bool GOSettingsPorts::GetPortItemChecked(
  const wxString &portName, const wxString &apiName) const {
  bool isChecked = true;

  for (wxTreeListItem item = m_Ports->GetFirstItem(); item.IsOk();
       item = m_Ports->GetNextItem(item))
    if (((PortItemData *)m_Ports->GetItemData(item))
          ->isItemForPortApi(portName, apiName)) {
      isChecked = m_Ports->GetCheckedState(item);
      break;
    }
  return isChecked;
}

void GOSettingsPorts::SetPortItemChecked(wxTreeListItem item, bool isChecked) {
  if (isChecked)
    m_Ports->CheckItem(item);
  else
    m_Ports->UncheckItem(item);
}

void GOSettingsPorts::OnPortItemChecked(wxTreeListEvent &event) {
  const wxTreeListItem &item = event.GetItem();
  const PortItemData *data = (PortItemData *)m_Ports->GetItemData(item);
  const bool oldIsChecked = event.GetOldCheckedState();
  const bool newIsChecked = m_Ports->GetCheckedState(item);

  OnPortChanged(data->m_PortName, data->m_ApiName, oldIsChecked, newIsChecked);
}

GOSettingsPorts::GOSettingsPorts(
  wxWindow *parent, const GOPortFactory &portFactory, const wxString &name)
  : m_PortFactory(portFactory) {
  m_PortsSizer = new wxStaticBoxSizer(wxHORIZONTAL, parent, name);
  m_Ports = new wxTreeListCtrl(
    parent,
    wxID_ANY,
    wxDefaultPosition,
    wxDefaultSize,
    wxTL_SINGLE | wxTL_CHECKBOX | wxTL_NO_HEADER);
  m_Ports->AppendColumn(wxEmptyString);
  m_Ports->SetColumnWidth(0, 200);
  m_Ports->Bind(
    wxEVT_TREELIST_ITEM_CHECKED, &GOSettingsPorts::OnPortItemChecked, this);
  m_PortsSizer->Add(m_Ports, 1, wxEXPAND);
}

GOSettingsPorts::~GOSettingsPorts() {
  if (m_Ports)
    m_Ports->Unbind(
      wxEVT_TREELIST_ITEM_CHECKED, &GOSettingsPorts::OnPortItemChecked, this);
}

void GOSettingsPorts::FillPortsWith(const GOPortsConfig &config) {
  m_PortsConfig = config;
  m_Ports->DeleteAllItems();

  for (const wxString &portName : m_PortFactory.GetPortNames()) {
    const wxTreeListItem &rootItem = m_Ports->GetRootItem();
    const wxTreeListItem &portItem = m_PortFactory.IsToUsePortName()
      ? AddPortItem(rootItem, portName)
      : rootItem;

    SetPortItemChecked(portItem, m_PortsConfig.IsConfigEnabled(portName));
    for (const wxString &apiName : m_PortFactory.GetPortApiNames(portName)) {
      const wxTreeListItem portApiItem
        = AddPortItem(portItem, portName, apiName);

      SetPortItemChecked(
        portApiItem, m_PortsConfig.IsConfigEnabled(portName, apiName));
    }
    m_Ports->Expand(portItem);
  }
}

GOPortsConfig &GOSettingsPorts::RenewPortsConfig() {
  for (const wxString &portName : m_PortFactory.GetPortNames()) {
    m_PortsConfig.SetConfigEnabled(portName, GetPortItemChecked(portName));
    for (const wxString &apiName : m_PortFactory.GetPortApiNames(portName)) {
      bool isChecked = GetPortItemChecked(portName, apiName);

      m_PortsConfig.SetConfigEnabled(portName, apiName, isChecked);
    }
  }
  return m_PortsConfig;
}
