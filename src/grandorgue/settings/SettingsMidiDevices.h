/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef SETTINGSMIDIDEVICES_H
#define SETTINGSMIDIDEVICES_H

#include <wx/panel.h>
#include <vector>

class GOSound;
class wxButton;
class wxCheckListBox;
class wxChoice;

class SettingsMidiDevices : public wxPanel
{
	enum {
		ID_INDEVICES = 200,
		ID_INCHANNELSHIFT,
		ID_INOUTDEVICE,
		ID_OUTDEVICES,
		ID_RECORDERDEVICE,
	};
private:
	GOSound& m_Sound;
	wxCheckListBox* m_InDevices;
	wxCheckListBox* m_OutDevices;
	std::vector<int> m_InDeviceData;
	std::vector<wxString> m_InOutDeviceData;
	wxButton* m_InProperties;
	wxButton* m_InOutDevice;
	wxChoice* m_RecorderDevice;

	void OnInDevicesClick(wxCommandEvent& event);
	void OnInOutDeviceClick(wxCommandEvent& event);
	void OnInChannelShiftClick(wxCommandEvent& event);

public:
	SettingsMidiDevices(GOSound& sound, wxWindow* parent);

	void Save();

	DECLARE_EVENT_TABLE()
};

#endif
