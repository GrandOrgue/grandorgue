/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTINGSMIDIDEVICES_H
#define GOSETTINGSMIDIDEVICES_H

#include <wx/checkbox.h>
#include <wx/panel.h>

#include "GOSettingsMidiDeviceList.h"
#include "GOSettingsPorts.h"
#include "config/GOConfig.h"
#include "config/GOPortsConfig.h"

class wxButton;
class wxCheckListBox;
class wxChoice;
class GOMidi;
class GOMidiPort;
class GOConfig;

class SettingsMidiDevices : public wxPanel, GOSettingsPorts {
  enum {
    ID_INDEVICES = 200,
    ID_AUTO_ADD_MIDI,
    ID_CHECK_ON_STARTUP,
    ID_INCHANNELSHIFT,
    ID_INOUTDEVICE,
    ID_OUTDEVICES,
    ID_RECORDERDEVICE,
  };

private:
  GOConfig &m_config;
  GOMidi &m_Midi;

  GOSettingsMidiDeviceList m_InDevices;
  GOSettingsMidiDeviceList m_OutDevices;

  wxCheckBox *m_AutoAddInput;
  wxCheckBox *m_CheckOnStartup;
  wxButton *m_InProperties;
  wxButton *m_InOutDevice;
  wxChoice *m_RecorderDevice;

  void RenewDevices(
    const GOPortsConfig &portsConfig, const bool isToAutoAddInput);
  void OnPortChanged(
    const wxString &portName,
    const wxString apiName,
    bool oldEnabled,
    bool newEnabled);
  void OnInDevicesClick(wxCommandEvent &event);
  void OnInOutDeviceClick(wxCommandEvent &event);
  void OnInChannelShiftClick(wxCommandEvent &event);
  void OnOutDevicesClick(wxCommandEvent &event);

public:
  SettingsMidiDevices(GOConfig &settings, GOMidi &midi, wxWindow *parent);

  virtual bool TransferDataFromWindow() override;

  DECLARE_EVENT_TABLE()
};

#endif
