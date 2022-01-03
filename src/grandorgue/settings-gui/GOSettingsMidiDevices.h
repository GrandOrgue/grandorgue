/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef SETTINGSMIDIDEVICES_H
#define SETTINGSMIDIDEVICES_H

#include <wx/checkbox.h>
#include <wx/panel.h>

#include "GOPortsConfig.h"
#include "GOSettings.h"
#include "GOSettingsMidiDeviceList.h"
#include "GOSettingsPorts.h"

class wxButton;
class wxCheckListBox;
class wxChoice;
class GOMidi;
class GOMidiPort;
class GOSettings;

class SettingsMidiDevices : public wxPanel, GOSettingsPorts
{
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
	GOSettings& m_Settings;
	GOMidi& m_Midi;

	GOSettingsMidiDeviceList m_InDevices;
	GOSettingsMidiDeviceList m_OutDevices;

	wxCheckBox* m_AutoAddInput;
	wxCheckBox* m_CheckOnStartup;
	wxButton* m_InProperties;
	wxButton* m_InOutDevice;
	wxChoice* m_RecorderDevice;

	void RenewDevices(const GOPortsConfig& portsConfig, const bool isToAutoAddInput);
	void OnPortChanged(
	  const wxString &portName, const wxString apiName, bool oldEnabled, bool newEnabled
	);
	void OnInDevicesClick(wxCommandEvent& event);
	void OnInOutDeviceClick(wxCommandEvent& event);
	void OnInChannelShiftClick(wxCommandEvent& event);
	void OnOutDevicesClick(wxCommandEvent& event);

public:
	SettingsMidiDevices(GOSettings& settings, GOMidi& midi, wxWindow* parent);

	void Save();

	DECLARE_EVENT_TABLE()
};

#endif
