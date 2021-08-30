/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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

#include "GOrgueSoundDevInfo.h"
#include "GOrgueSoundPortsConfig.h"
#include "SettingsAudioGroup.h"

class AudioItemData;
class GOrgueSound;

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
	GOrgueSound& m_Sound;
	GOrgueSettings& m_Settings;
	GOAudioGroupCallback& m_GroupCallback;
	GOrgueSoundPortsConfig m_SoundPortsConfig;
	
	wxChoice* m_SampleRate;
	wxSpinCtrl* m_SamplesPerBuffer;
	
	wxTreeListCtrl* m_SoundPorts;
	wxTreeCtrl* m_AudioOutput;
	wxButton* m_Add;
	wxButton* m_Del;
	wxButton* m_Change;
	wxButton* m_Properties;
	wxButton* m_Default;
	
	GOrgueSoundPortsConfig m_PortsConfigPopulatedWith;
	std::vector<GOrgueSoundDevInfo> m_DeviceList;
	
	bool GetPortItemChecked(
	  const wxString &portName, const wxString& apiName = wxEmptyString
	) const;
	void SetPortItemChecked(wxTreeListItem item, bool isChecked);
	GOrgueSoundPortsConfig & RenewSoundPortsConfig();

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
	SettingsAudioOutput(GOrgueSound& sound, GOAudioGroupCallback& callback, wxWindow* parent);

	void Save();

	DECLARE_EVENT_TABLE()
};

#endif
