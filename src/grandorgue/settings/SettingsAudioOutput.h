/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef SETTINGSAUDIOOUTPUT_H
#define SETTINGSAUDIOOUTPUT_H

#include <vector>
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/panel.h>
#include <wx/spinctrl.h>
#include <wx/string.h>
#include <wx/treectrl.h>
#include <wx/treelist.h>

#include "sound/GOSoundDevInfo.h"
#include "sound/ports/GOSoundPortsConfig.h"
#include "SettingsAudioGroup.h"

class AudioItemData;
class GOSound;

class SettingsAudioOutput : public wxPanel
{
	enum {
		ID_OUTPUT_LIST = 200,
		ID_OUTPUT_ADD,
		ID_OUTPUT_DEL,
		ID_OUTPUT_CHANGE,
		ID_OUTPUT_PROPERTIES,
		ID_OUTPUT_DEFAULT,
		ID_SOND_PORTS,
		ID_SAMPLE_RATE,
		ID_SAMPLES_PER_BUFFER
	};

private:
	GOSound& m_Sound;
	GOSettings& m_Settings;
	GOAudioGroupCallback& m_GroupCallback;
	GOSoundPortsConfig m_SoundPortsConfig;
	
	wxChoice* m_SampleRate;
	wxSpinCtrl* m_SamplesPerBuffer;
	
	wxTreeListCtrl* m_SoundPorts;
	wxTreeCtrl* m_AudioOutput;
	wxButton* m_Add;
	wxButton* m_Del;
	wxButton* m_Change;
	wxButton* m_Properties;
	wxButton* m_Default;
	
	GOSoundPortsConfig m_PortsConfigPopulatedWith;
	std::vector<GOSoundDevInfo> m_DeviceList;
	
	bool GetPortItemChecked(
	  const wxString &portName, const wxString& apiName = wxEmptyString
	) const;
	void SetPortItemChecked(wxTreeListItem item, bool isChecked);
	GOSoundPortsConfig & RenewSoundPortsConfig();

	AudioItemData* GetObject(const wxTreeItemId& id);
	wxTreeItemId GetDeviceNode(const wxString& name);
	wxTreeItemId GetChannelNode(const wxTreeItemId& audio, unsigned channel);
	wxTreeItemId GetGroupNode(const wxTreeItemId& channel, const wxString& name, bool left);

	wxTreeItemId AddDeviceNode(wxString name);
	wxTreeItemId AddDeviceNode(wxString name, unsigned latency);
	wxTreeItemId AddChannelNode(const wxTreeItemId& audio, unsigned channel);
	wxTreeItemId AddGroupNode(const wxTreeItemId& channel, const wxString& name, bool left);
	void UpdateDevice(const wxTreeItemId& dev);
	void UpdateVolume(const wxTreeItemId& group, float volume);
	void UpdateButtons();

	void AssureDeviceList();
	std::vector<wxString> GetRemainingAudioDevices(const wxTreeItemId *ignoreItem);
	std::vector<std::pair<wxString, bool> > GetRemainingAudioGroups(const wxTreeItemId& channel);

	void OnOutputChanged(wxTreeEvent& event);
	void OnOutputAdd(wxCommandEvent& event);
	void OnOutputDel(wxCommandEvent& event);
	void OnOutputChange(wxCommandEvent& event);
	void OnOutputProperties(wxCommandEvent& event);
	void OnOutputDefault(wxCommandEvent& event);

public:
	SettingsAudioOutput(GOSound& sound, GOAudioGroupCallback& callback, wxWindow* parent);

	void Save();

	DECLARE_EVENT_TABLE()
};

#endif
