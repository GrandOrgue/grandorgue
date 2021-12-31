/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOSettingsMidiDeviceList.h"

GOSettingsMidiDeviceList::GOSettingsMidiDeviceList(
  const ptr_vector<GOMidiPort>& ports,
  GOMidiDeviceConfigList& configListPersist,
  wxWindow* parent,
  wxWindowID id
):
  m_parent(parent), m_Ports(ports), m_ConfList(configListPersist)
{
  m_LbDevices = new wxCheckListBox(
    parent, id, wxDefaultPosition, wxSize(100, 100)
  );
  m_LbDevices->Bind(
    wxEVT_CHECKLISTBOX,
    &GOSettingsMidiDeviceList::OnChecked,
    this
  );
  Init();
}

void GOSettingsMidiDeviceList::ClearDevices()
{
  // We cann't use lbDevices->Clear() because it disables the event handler
  for (int i = m_LbDevices->GetCount() - 1; i >= 0; i--)
    m_LbDevices->Delete(i);
  m_ListedConfs.clear();
}

void GOSettingsMidiDeviceList::Init()
{
  ClearDevices();
  m_ConfListTmp.Clear();
}

void GOSettingsMidiDeviceList::RefreshDevices(
  const GOPortsConfig& portsConfig,
  const bool isToAutoEnable,
  const GOSettingsMidiDeviceList* pOutDevList
)
{
  ClearDevices();
  for (GOMidiPort* port : m_Ports)
    if (
      port->IsToUse()
      && portsConfig.IsEnabled(port->GetPortName(), port->GetApiName())
    )
    {
      const wxString physicalName = port->GetName();
      GOMidiDeviceConfig* const pConf
	= m_ConfList.FindByPhysicalName(physicalName);
      GOMidiDeviceConfig* pConfTmp
	= m_ConfListTmp.FindByPhysicalName(physicalName);

      if (! pConfTmp)
	pConfTmp = pConf
	  ? m_ConfListTmp.Append(
	    *pConf, pOutDevList ? &pOutDevList->m_ConfListTmp : NULL
	  )
	  : m_ConfListTmp.Append(
	    port->GetDefaultLogicalName(),
	    port->GetDefaultRegEx(),
	    isToAutoEnable,
	    physicalName
	  );

      m_ListedConfs.push_back(pConfTmp);
      
      const int i = m_LbDevices->Append(physicalName);

      if (pConfTmp->m_IsEnabled)
	m_LbDevices->Check(i);
    }
}

unsigned GOSettingsMidiDeviceList::GetDeviceCount() const
{
  return m_LbDevices->GetCount();
}

GOMidiDeviceConfig&
  GOSettingsMidiDeviceList::GetSelectedDeviceConf()
  const
{
  return GetDeviceConf(m_LbDevices->GetSelection());
}

void GOSettingsMidiDeviceList::OnSelected(wxCommandEvent& event)
{
}

void GOSettingsMidiDeviceList::OnChecked(wxCommandEvent& event)
{
  unsigned i = (unsigned) event.GetInt();

  GetDeviceConf(i).m_IsEnabled = m_LbDevices->IsChecked(i);
}

void GOSettingsMidiDeviceList::Save(
  const GOSettingsMidiDeviceList* pOutDevList
)
{
  for (GOMidiDeviceConfig* pDevConfTmp : m_ListedConfs)
  {
    GOMidiDeviceConfig* pDevConf = m_ConfList.FindByPhysicalName(
      pDevConfTmp->m_PhysicalName
    );
    const GOMidiDeviceConfigList* pOutConfList
      = pOutDevList ? & pOutDevList->m_ConfList : NULL;

    if (pDevConf)
    {
      pDevConf->Assign(*pDevConfTmp);
      if (pOutConfList)
	pOutConfList->MapOutputDevice(*pDevConfTmp, *pDevConf);
    } else
      m_ConfList.Append(*pDevConfTmp, pOutConfList);
  }
}
