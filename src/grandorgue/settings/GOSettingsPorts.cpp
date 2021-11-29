/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include <thread>

#include "GOSettingsPorts.h"

wxString GOSettingsPorts::GetPortItemName(
  const wxString &portName, const wxString &apiName
) const
{
  wxString itemName;

  if (m_PortFactory.IsToUsePortName())
    itemName = portName;

  if (! apiName.IsEmpty())
  {
    if (! itemName.IsEmpty())
      itemName += ": ";
    itemName += apiName;
  }
  return itemName;
}

void GOSettingsPorts::SetPortItemChecked(wxTreeListItem item, bool isChecked)
{
  if (isChecked)
    m_Ports->CheckItem(item);
  else
    m_Ports->UncheckItem(item);
}

bool GOSettingsPorts::GetPortItemChecked(
  const wxString &portName, const wxString& apiName
) const
{
  bool isChecked = true;
  const wxString itemText = GetPortItemName(portName, apiName);

  for (
    wxTreeListItem item = m_Ports->GetFirstItem();
    item.IsOk();
    item = m_Ports->GetNextItem(item)
  ) if (m_Ports->GetItemText(item, 0) == itemText)
  {
    isChecked = m_Ports->GetCheckedState(item);
    break;
  }
  return isChecked;
}

GOSettingsPorts::GOSettingsPorts(
  wxWindow* parent, const GOPortFactory &portFactory, const wxString& name
):
  m_PortFactory(portFactory)
{
  m_PortsSizer = new wxStaticBoxSizer(wxVERTICAL, parent, name);
  m_Ports = new wxTreeListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTL_SINGLE | wxTL_CHECKBOX | wxTL_NO_HEADER);
  m_Ports->AppendColumn(wxEmptyString);
  m_PortsSizer->Add(m_Ports, 1, wxEXPAND | wxALIGN_LEFT);
}

void GOSettingsPorts::FillPortsWith(const GOPortsConfig& config)
{
  m_PortsConfig = config;
  m_Ports->DeleteAllItems();
  
  for (const wxString &portName: m_PortFactory.GetPortNames())
  {
    const wxTreeListItem& rootItem = m_Ports->GetRootItem();
    const wxTreeListItem& portItem
      = m_PortFactory.IsToUsePortName()
      ? m_Ports->AppendItem(rootItem, GetPortItemName(portName))
      : rootItem;

    SetPortItemChecked(portItem, m_PortsConfig.IsConfigEnabled(portName));
    for (const wxString &apiName: m_PortFactory.GetPortApiNames(portName))
    {
      const wxTreeListItem portApiItem
	= m_Ports->AppendItem(portItem, GetPortItemName(portName, apiName));

      SetPortItemChecked(
	portApiItem, m_PortsConfig.IsConfigEnabled(portName, apiName)
      );
    }
    m_Ports->Expand(portItem);
  }
}

GOPortsConfig& GOSettingsPorts::RenewPortsConfig()
{
  for (const wxString &portName: m_PortFactory.GetPortNames())
  {
    m_PortsConfig.SetConfigEnabled(portName, GetPortItemChecked(portName));
    for (const wxString &apiName: m_PortFactory.GetPortApiNames(portName)) {
      m_PortsConfig.SetConfigEnabled(portName, apiName, GetPortItemChecked(portName, apiName));
    }
  }
  return m_PortsConfig;
}
