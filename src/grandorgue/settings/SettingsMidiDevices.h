/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef SETTINGSMIDIDEVICES_H
#define SETTINGSMIDIDEVICES_H

#include <vector>

#include <wx/checkbox.h>
#include <wx/panel.h>

#include "GOPortsConfig.h"
#include "GOSettings.h"
#include "GOSettingsPorts.h"

class GOSound;
class wxButton;
class wxCheckListBox;
class wxChoice;

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
	GOSound& m_Sound;
	GOSettings& m_Settings;

	wxCheckBox* m_AutoAddInput;
	wxCheckBox* m_CheckOnStartup;
	wxCheckListBox* m_InDevices;
	wxCheckListBox* m_OutDevices;
	wxButton* m_InProperties;
	wxButton* m_InOutDevice;
	wxChoice* m_RecorderDevice;

	std::vector<int> m_InDeviceData;
	std::vector<wxString> m_InOutDeviceData;

	void RenewDevices(const GOPortsConfig& portsConfig, const bool isToAutoAddInput);
	void OnPortChanged(
	  const wxString &portName, const wxString apiName, bool oldEnabled, bool newEnabled
	);
	void OnInDevicesClick(wxCommandEvent& event);
	void OnInOutDeviceClick(wxCommandEvent& event);
	void OnInChannelShiftClick(wxCommandEvent& event);

public:
	SettingsMidiDevices(GOSound& sound, wxWindow* parent);

	void Save();

	DECLARE_EVENT_TABLE()
};

#endif
