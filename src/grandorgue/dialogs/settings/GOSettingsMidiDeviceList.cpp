/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSettingsMidiDeviceList.h"

#include "GOSettingsMidiMatchDialog.h"

GOSettingsMidiDeviceList::GOSettingsMidiDeviceList(
  const ptr_vector<GOMidiPort> &ports,
  GOMidiDeviceConfigList &configListPersist,
  wxWindow *parent,
  wxWindowID id)
  : m_parent(parent), m_Ports(ports), m_ConfList(configListPersist) {
  m_LbDevices
    = new wxCheckListBox(parent, id, wxDefaultPosition, wxSize(100, 100));
  m_LbDevices->Bind(
    wxEVT_CHECKLISTBOX, &GOSettingsMidiDeviceList::OnChecked, this);
  m_BMatching = new wxButton(parent, wxID_ANY, _("Matching..."));
  m_BMatching->Bind(
    wxEVT_BUTTON, &GOSettingsMidiDeviceList::OnMatchingClick, this);
  Init();
}

void GOSettingsMidiDeviceList::ClearDevices() {
  m_BMatching->Disable();
  // We cann't use lbDevices->Clear() because it disables the event handler
  for (int i = m_LbDevices->GetCount() - 1; i >= 0; i--)
    m_LbDevices->Delete(i);
  m_ListedConfs.clear();
}

void GOSettingsMidiDeviceList::Init() {
  ClearDevices();
  m_ConfListTmp.Clear();
}

void GOSettingsMidiDeviceList::RefreshDevices(
  const GOPortsConfig &portsConfig,
  const bool isToAutoEnable,
  const GOSettingsMidiDeviceList *pOutDevList) {
  ClearDevices();
  for (GOMidiPort *port : m_Ports) {
    const wxString &portName = port->GetPortName();
    const wxString &apiName = port->GetApiName();

    if (port->IsToUse() && portsConfig.IsEnabled(portName, apiName)) {
      const wxString physicalName = port->GetName();
      GOMidiDeviceConfig *const pConf
        = m_ConfList.FindByPhysicalName(physicalName, portName, apiName);
      GOMidiDeviceConfig *pConfTmp
        = m_ConfListTmp.FindByPhysicalName(physicalName, portName, apiName);

      if (!pConfTmp)
        pConfTmp = pConf ? m_ConfListTmp.Append(
                     *pConf, pOutDevList ? &pOutDevList->m_ConfListTmp : NULL)
                         : m_ConfListTmp.Append(
                           port->GetDefaultLogicalName(),
                           port->GetDefaultRegEx(),
                           portName,
                           apiName,
                           isToAutoEnable,
                           physicalName);

      m_ListedConfs.push_back(pConfTmp);

      const int i = m_LbDevices->Append(physicalName);

      if (pConfTmp->m_IsEnabled)
        m_LbDevices->Check(i);
    }
  }
}

unsigned GOSettingsMidiDeviceList::GetDeviceCount() const {
  return m_LbDevices->GetCount();
}

GOMidiDeviceConfig &GOSettingsMidiDeviceList::GetSelectedDeviceConf() const {
  return GetDeviceConf(m_LbDevices->GetSelection());
}

void GOSettingsMidiDeviceList::OnSelected(wxCommandEvent &event) {
  m_BMatching->Enable();
}

void GOSettingsMidiDeviceList::OnChecked(wxCommandEvent &event) {
  unsigned i = (unsigned)event.GetInt();

  GetDeviceConf(i).m_IsEnabled = m_LbDevices->IsChecked(i);
}

void GOSettingsMidiDeviceList::OnMatchingClick(wxCommandEvent &event) {
  GOMidiDeviceConfig &devConf = GetSelectedDeviceConf();
  GOSettingsMidiMatchDialog dlg(m_parent, &m_ListedConfs);

  dlg.FillWith(devConf);
  if (dlg.ShowModal() == wxID_OK) {
    dlg.SaveTo(devConf);
    m_ConfListTmp.RemoveByLogicalNameOutOf(
      devConf.m_LogicalName, m_ListedConfs);
  }
}

void GOSettingsMidiDeviceList::Save(
  const GOSettingsMidiDeviceList *pOutDevList) {
  for (GOMidiDeviceConfig *pDevConfTmp : m_ListedConfs) {
    GOMidiDeviceConfig *pDevConf = m_ConfList.FindByPhysicalName(
      pDevConfTmp->m_PhysicalName,
      pDevConfTmp->m_PortName,
      pDevConfTmp->m_ApiName);
    const GOMidiDeviceConfigList *pOutConfList
      = pOutDevList ? &pOutDevList->m_ConfList : NULL;

    if (pDevConf) {
      pDevConf->Assign(*pDevConfTmp);
      if (pOutConfList)
        pOutConfList->MapOutputDevice(*pDevConfTmp, *pDevConf);
    } else
      m_ConfList.Append(*pDevConfTmp, pOutConfList);

    // remove unlisted devices with the same logical name
    m_ConfList.RemoveByLogicalNameOutOf(
      pDevConfTmp->m_LogicalName, m_ListedConfs);
  }
}
