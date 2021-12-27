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

#include "GOMidiDeviceConfigList.h"
#include "GOPortsConfig.h"
#include "GOSettings.h"
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

	class MidiDeviceListSettings
	{
	private:
	  const ptr_vector<GOMidiPort>& m_Ports;
	  GOMidiDeviceConfigList& m_ConfList;
	  wxCheckListBox* m_LbDevices;

	  // temporary storage for configs when edited
	  GOMidiDeviceConfigList m_ConfListTmp;

	  void ClearDevices();

	public:
	  MidiDeviceListSettings(
	    const ptr_vector<GOMidiPort>& ports,
	    GOMidiDeviceConfigList& configListPersist,
	    wxWindow* parent,
	    wxWindowID id
	  );

	  wxCheckListBox* GetListbox() const { return m_LbDevices; }

	  void Init();

	  void RefreshDevices(
	    const GOPortsConfig& portsConfig,
	    const bool isToAutoEnable,
	    const MidiDeviceListSettings* pOutDevList = NULL
	  );

	  unsigned GetDeviceCount() const;
	  GOMidiDeviceConfig& GetDeviceConf(unsigned i) const;
	  GOMidiDeviceConfig& GetSelectedDeviceConf() const;

	  void OnChecked(wxCommandEvent& event);

	  void Save(const MidiDeviceListSettings* pOutDevList = NULL);
	};

	MidiDeviceListSettings m_InDevices;
	MidiDeviceListSettings m_OutDevices;

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

public:
	SettingsMidiDevices(GOSettings& settings, GOMidi& midi, wxWindow* parent);

	void Save();

	DECLARE_EVENT_TABLE()
};

#endif
