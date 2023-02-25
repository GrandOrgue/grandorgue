/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTINGSMIDIDEVICELIST_H
#define GOSETTINGSMIDIDEVICELIST_H

#include <wx/button.h>
#include <wx/checklst.h>
#include <wx/window.h>

#include "config/GOMidiDeviceConfigList.h"
#include "config/GOPortsConfig.h"
#include "midi/ports/GOMidiPort.h"
#include "ptrvector.h"

class GOSettingsMidiDeviceList {
private:
  wxWindow *m_parent;
  const ptr_vector<GOMidiPort> &m_Ports;
  GOMidiDeviceConfigList &m_ConfList;
  GOMidiDeviceConfig::RefVector m_ListedConfs;
  wxCheckListBox *m_LbDevices;
  wxButton *m_BMatching;

  // temporary storage for configs when edited
  GOMidiDeviceConfigList m_ConfListTmp;

  void ClearDevices();

  void OnChecked(wxCommandEvent &event);
  void OnMatchingClick(wxCommandEvent &event);

public:
  GOSettingsMidiDeviceList(
    const ptr_vector<GOMidiPort> &ports,
    GOMidiDeviceConfigList &configListPersist,
    wxWindow *parent,
    wxWindowID id);

  wxCheckListBox *GetListbox() const { return m_LbDevices; }
  wxButton *GetMatchingButton() const { return m_BMatching; }

  void Init();

  void RefreshDevices(
    const GOPortsConfig &portsConfig,
    const bool isToAutoEnable,
    const GOSettingsMidiDeviceList *pOutDevList = NULL);

  unsigned GetDeviceCount() const;
  GOMidiDeviceConfig &GetDeviceConf(unsigned i) const {
    return *m_ListedConfs[i];
  }
  GOMidiDeviceConfig &GetSelectedDeviceConf() const;

  void OnSelected(wxCommandEvent &event);

  void Save(const GOSettingsMidiDeviceList *pOutDevList = NULL);
};

#endif /* GOSETTINGSMIDIDEVICELIST_H */
