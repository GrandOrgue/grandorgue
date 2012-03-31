/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2012 GrandOrgue contributors (see AUTHORS)
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
 * MA 02111-1307, USA.
 */

#include <wx/treectrl.h>
#include <wx/numdlg.h>

#include "SettingsAudioOutput.h"
#include "GOrgueSound.h"
#include "GOrgueSettings.h"

class AudioItemData : public wxTreeItemData
{
public:
	typedef enum { ROOT_NODE, AUDIO_NODE, CHANNEL_NODE, GROUP_NODE } node_type;
	
	AudioItemData()
	{
		type = ROOT_NODE;
		name = wxEmptyString;
		channel = 0;
		left = false;
		volume = -121;
	}

	AudioItemData(const wxString& device_name)
	{
		type = AUDIO_NODE;
		name = device_name;
		channel = 0;
		left = false;
		volume = -121;
	}

	AudioItemData(int ch)
	{
		type = CHANNEL_NODE;
		name = wxEmptyString;
		channel = ch;
		left = false;
		volume = -121;
	}

	AudioItemData(const wxString& group_name, bool left_channel, float vol)
	{
		type = GROUP_NODE;
		name = group_name;
		channel = 0;
		left = left_channel;
		volume = vol;
	}

	node_type type;
	wxString name;
	unsigned channel;
	bool left;
	float volume;
};

BEGIN_EVENT_TABLE(SettingsAudioOutput, wxPanel)
	EVT_TREE_SEL_CHANGED(ID_OUTPUT_LIST, SettingsAudioOutput::OnOutputChanged)
	EVT_BUTTON(ID_OUTPUT_ADD, SettingsAudioOutput::OnOutputAdd)
	EVT_BUTTON(ID_OUTPUT_DEL, SettingsAudioOutput::OnOutputDel)
	EVT_BUTTON(ID_OUTPUT_CHANGE, SettingsAudioOutput::OnOutputChange)
	EVT_BUTTON(ID_OUTPUT_PROPERTIES, SettingsAudioOutput::OnOutputProperties)
END_EVENT_TABLE()


SettingsAudioOutput::SettingsAudioOutput(GOrgueSound& sound, GOAudioGroupCallback& callback, wxWindow* parent) :
	wxPanel(parent, wxID_ANY),
	m_Sound(sound),
	m_GroupCallback(callback)
{
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	topSizer->AddSpacer(5);

	m_AudioOutput = new wxTreeCtrl(this, ID_OUTPUT_LIST, wxDefaultPosition, wxSize(640, 300), wxTR_HAS_BUTTONS | wxTR_SINGLE);
	topSizer->Add(m_AudioOutput, wxALIGN_LEFT | wxEXPAND);
	topSizer->AddSpacer(5);

	wxBoxSizer* buttonSizer =  new wxBoxSizer(wxHORIZONTAL);
	buttonSizer->AddSpacer(5);
	m_Change = new wxButton(this, ID_OUTPUT_CHANGE, _("Change"));
	m_Add = new wxButton(this, ID_OUTPUT_ADD, _("&Add"));
	m_Del = new wxButton(this, ID_OUTPUT_DEL, _("&Delete"));
	m_Properties = new wxButton(this, ID_OUTPUT_PROPERTIES, _("Properties"));

	buttonSizer->Add(m_Add, 0, wxALIGN_LEFT | wxALL, 5);
	buttonSizer->Add(m_Del, 0, wxALIGN_LEFT | wxALL, 5);
	buttonSizer->Add(m_Change, 0, wxALIGN_RIGHT | wxALL, 5);
	buttonSizer->Add(m_Properties, 0, wxALIGN_RIGHT | wxALL, 5);
	topSizer->Add(buttonSizer, 0, wxALL, 5);

	m_AudioOutput->AddRoot(_("Audio Output"), -1, -1, new AudioItemData());

	std::vector<GOAudioDeviceConfig> audio_config = m_Sound.GetSettings().GetAudioDeviceConfig();
	for(unsigned i = 0; i < audio_config.size(); i++)
	{
		wxTreeItemId audio = AddDeviceNode(audio_config[i].name);
		for(unsigned j = 0; j < audio_config[i].channels; j++)
		{
			wxTreeItemId channel = AddChannelNode(audio, j);
			if (j >= audio_config[i].scale_factors.size())
				continue;
			for(unsigned k = 0; k < audio_config[i].scale_factors[j].size(); k++)
			{
				if (audio_config[i].scale_factors[j][k].left >= -120)
				{
					wxTreeItemId group;
					group = AddGroupNode(channel, audio_config[i].scale_factors[j][k].name, true);
					UpdateVolume(group, audio_config[i].scale_factors[j][k].left);
				}
				if (audio_config[i].scale_factors[j][k].right >= -120)
				{
					wxTreeItemId group;
					group = AddGroupNode(channel, audio_config[i].scale_factors[j][k].name, false);
					UpdateVolume(group, audio_config[i].scale_factors[j][k].right);
				}
			}
		}
	}

	m_AudioOutput->ExpandAll();

	topSizer->AddSpacer(5);
	this->SetSizer(topSizer);
	topSizer->Fit(this);

	UpdateButtons();
}


AudioItemData* SettingsAudioOutput::GetObject(const wxTreeItemId& id)
{
	if (!id.IsOk())
		return NULL;
	return (AudioItemData*) m_AudioOutput->GetItemData(id);
}

wxTreeItemId SettingsAudioOutput::GetDeviceNode(const wxString& name)
{
	wxTreeItemIdValue i;
	wxTreeItemId current;
	wxTreeItemId root = m_AudioOutput->GetRootItem();
	current = m_AudioOutput->GetFirstChild(root, i);
	while(current.IsOk())
	{
		AudioItemData* data = (AudioItemData*) m_AudioOutput->GetItemData(current);
		if (data && data->type == AudioItemData::AUDIO_NODE && data->name == name)
			return current;
		current = m_AudioOutput->GetNextChild(root, i);
	}
	return current;
}

wxTreeItemId SettingsAudioOutput::GetChannelNode(const wxTreeItemId& audio, unsigned channel)
{
	wxTreeItemIdValue i;
	wxTreeItemId current;
	current = m_AudioOutput->GetFirstChild(audio, i);
	while(current.IsOk())
	{
		AudioItemData* data = (AudioItemData*) m_AudioOutput->GetItemData(current);
		if (data && data->type == AudioItemData::CHANNEL_NODE && data->channel == channel)
			return current;
		current = m_AudioOutput->GetNextChild(audio, i);
	}
	return current;
}

wxTreeItemId SettingsAudioOutput::GetGroupNode(const wxTreeItemId& channel, const wxString& name, bool left)
{
	wxTreeItemIdValue i;
	wxTreeItemId current;
	current = m_AudioOutput->GetFirstChild(channel, i);
	while(current.IsOk())
	{
		AudioItemData* data = (AudioItemData*) m_AudioOutput->GetItemData(current);
		if (data && data->type == AudioItemData::GROUP_NODE && data->name == name && data->left == left)
			return current;
		current = m_AudioOutput->GetNextChild(channel, i);
	}
	return current;
}

wxTreeItemId SettingsAudioOutput::AddDeviceNode(wxString name)
{
	wxTreeItemId current;
	if (name == wxEmptyString)
		name = m_Sound.GetDefaultAudioDevice();
	current = GetDeviceNode(name);
	if (current.IsOk())
		return current;
	wxString text;
	int latency = m_Sound.GetSettings().GetAudioDeviceActualLatency(name);
	if (latency > 0)
		text = wxString::Format(_("Device: %s (%d ms)"), name.c_str(), latency);
	else
		text = wxString::Format(_("Device: %s"), name.c_str());
	current = m_AudioOutput->AppendItem(m_AudioOutput->GetRootItem(), text, -1, -1, new AudioItemData(name));
	m_AudioOutput->Expand(current);
	return current;
}

wxTreeItemId SettingsAudioOutput::AddChannelNode(const wxTreeItemId& audio, unsigned channel)
{
	wxTreeItemId current;
	current = GetChannelNode(audio, channel);
	if (current.IsOk())
		return current;
	current = m_AudioOutput->AppendItem(audio, wxString::Format(_("Channel %d"), channel + 1), -1, -1, new AudioItemData(channel));
	m_AudioOutput->Expand(current);
	m_AudioOutput->Expand(audio);
	return current;
}

wxTreeItemId SettingsAudioOutput::AddGroupNode(const wxTreeItemId& channel, const wxString& name, bool left)
{
	wxTreeItemId current;
	current = GetGroupNode(channel, name, left);
	if (current.IsOk())
		return current;
	current = m_AudioOutput->AppendItem(channel, name, -1, -1, new AudioItemData(name, left, -121));
	m_AudioOutput->Expand(current);
	m_AudioOutput->Expand(channel);
	UpdateVolume(current, -121);
	return current;
}

void SettingsAudioOutput::UpdateVolume(const wxTreeItemId& group, float volume)
{
	AudioItemData* data = GetObject(group);
	wxString name = wxString::Format(data->left ? _("%s - left") :  _("%s - right"), data->name.c_str());
	if (volume < -121.0 || volume > 40.0)
		volume = 0;
	data->volume = volume;
	if (volume >= -120)
		m_AudioOutput->SetItemText(group, wxString::Format(_("%s: %f dB"), name.c_str(), volume));
	else
		m_AudioOutput->SetItemText(group, wxString::Format(_("%s: mute"), name.c_str()));
}

std::vector<wxString> SettingsAudioOutput::GetRemainingAudioDevices()
{
	std::vector<wxString> result;
	std::map<wxString, GOrgueSound::GO_SOUND_DEV_CONFIG> devs = m_Sound.GetAudioDevices();
	std::map<wxString, GOrgueSound::GO_SOUND_DEV_CONFIG>::iterator it = devs.begin();
	while(it != devs.end())
	{
		if (!GetDeviceNode(it->first).IsOk())
			result.push_back(it->first);
		it++;
	}
	return result;
}

std::vector<std::pair<wxString, bool> > SettingsAudioOutput::GetRemainingAudioGroups(const wxTreeItemId& channel)
{
	std::vector<std::pair<wxString, bool> > result;
	std::vector<wxString> groups = m_GroupCallback.GetGroups();
	for(unsigned i = 0; i < groups.size(); i++)
	{
		const wxString& name = groups[i];
		if (!GetGroupNode(channel, name, true).IsOk())
			result.push_back(std::pair<wxString, bool>(name, true));
		if (!GetGroupNode(channel, name, false).IsOk())
			result.push_back(std::pair<wxString, bool>(name, false));
	}
	return result;
}

void SettingsAudioOutput::UpdateButtons()
{
	wxTreeItemId selection = m_AudioOutput->GetSelection();
	AudioItemData* data = GetObject(selection);
	if (data && data->type == AudioItemData::AUDIO_NODE)
	{
		std::map<wxString, GOrgueSound::GO_SOUND_DEV_CONFIG> devs = m_Sound.GetAudioDevices();
		std::map<wxString, GOrgueSound::GO_SOUND_DEV_CONFIG>::iterator it = devs.find(data->name);
		if (it != devs.end() && m_AudioOutput->GetChildrenCount(selection, false) != it->second.channels)
			m_Add->Enable();
		else
			m_Add->Disable();
	
		m_Properties->Enable();
		m_Change->Enable();
		if (m_AudioOutput->GetChildrenCount(m_AudioOutput->GetRootItem(), false) > 1)
			m_Del->Enable();
		else
			m_Del->Disable();
	}
	else if (data && data->type == AudioItemData::CHANNEL_NODE)
	{
		m_Properties->Disable();
		m_Change->Disable();
		if (GetRemainingAudioGroups(selection).size())
			m_Add->Enable();
		else
			m_Add->Disable();
		if (m_AudioOutput->GetLastChild(m_AudioOutput->GetItemParent(selection)) == selection)
			m_Del->Enable();
		else
			m_Del->Disable();
	}
	else if (data && data->type == AudioItemData::GROUP_NODE)
	{
		m_Properties->Enable();
		m_Change->Enable();
		m_Add->Disable();
		m_Del->Enable();
	}
	else if (data && data->type == AudioItemData::ROOT_NODE)
	{
		m_Properties->Disable();
		m_Change->Disable();
		if (GetRemainingAudioDevices().size())
			m_Add->Enable();
		else
			m_Add->Disable();
		m_Del->Disable();
		m_Add->Disable();
	}
	else
	{
		m_Properties->Disable();
		m_Change->Disable();
		m_Add->Disable();
		m_Del->Disable();
	}
}

void SettingsAudioOutput::OnOutputChanged(wxTreeEvent& event)
{
	UpdateButtons();
}

void SettingsAudioOutput::OnOutputAdd(wxCommandEvent& event)
{
	wxTreeItemId selection = m_AudioOutput->GetSelection();
	AudioItemData* data = GetObject(selection);
	if (data && data->type == AudioItemData::AUDIO_NODE)
	{
		std::map<wxString, GOrgueSound::GO_SOUND_DEV_CONFIG> devs = m_Sound.GetAudioDevices();
		std::map<wxString, GOrgueSound::GO_SOUND_DEV_CONFIG>::iterator it = devs.find(data->name);
		unsigned channels = m_AudioOutput->GetChildrenCount(selection, false);
		if (it != devs.end() && channels != it->second.channels)
			AddChannelNode(selection, channels);
	}
	else if (data && data->type == AudioItemData::CHANNEL_NODE)
	{
		std::vector<std::pair<wxString, bool> > groups;
		wxArrayString names;
		int index;
		groups = GetRemainingAudioGroups(selection);
		for(unsigned i = 0; i < groups.size(); i++)
			names.Add(wxString::Format(groups[i].second ? _("%s - left") : _("%s - right"), groups[i].first.c_str()));
		index = wxGetSingleChoiceIndex(_("Add new audio group"), _("New audio group"), names, this);
		if (index == -1)
			return;
		wxTreeItemId id = AddGroupNode(selection, groups[index].first, groups[index].second);
		UpdateVolume(id, 0);
	}
	else if (data && data->type == AudioItemData::ROOT_NODE)
	{
		int index;
		wxArrayString devs;
		std::vector<wxString> devices = GetRemainingAudioDevices();
		for(unsigned i = 0; i < devices.size(); i++)
			devs.Add(devices[i]);
		index = wxGetSingleChoiceIndex(_("Add new audio device"), _("New audio device"), devs, this);
		if (index == -1)
			return;
		AddDeviceNode(devices[index]);
	}
	UpdateButtons();
}

void SettingsAudioOutput::OnOutputDel(wxCommandEvent& event)
{
	wxTreeItemId selection = m_AudioOutput->GetSelection();
	AudioItemData* data = GetObject(selection);
	if (data && data->type == AudioItemData::AUDIO_NODE)
	{
		if (m_AudioOutput->GetChildrenCount(m_AudioOutput->GetRootItem(), false) > 1)
			m_AudioOutput->Delete(selection);
	}
	else if (data && data->type == AudioItemData::CHANNEL_NODE)
	{
		m_AudioOutput->Delete(selection);
	}
	else if (data && data->type == AudioItemData::GROUP_NODE)
	{
		m_AudioOutput->Delete(selection);
	}
	UpdateButtons();
}

void SettingsAudioOutput::OnOutputChange(wxCommandEvent& event)
{
	wxTreeItemId selection = m_AudioOutput->GetSelection();
	AudioItemData* data = GetObject(selection);
	if (data && data->type == AudioItemData::AUDIO_NODE)
	{
		int index;
		wxArrayString devs;
		std::vector<wxString> devices = GetRemainingAudioDevices();
		for(unsigned i = 0; i < devices.size(); i++)
			devs.Add(devices[i]);
		devs.Insert(data->name, 0);
		index = wxGetSingleChoiceIndex(_("Change audio device"), _("Change audio device"), devs, this);
		if (index == -1 || index == 0)
			return;
		data->name = devs[index];

		wxString text;
		int latency = m_Sound.GetSettings().GetAudioDeviceActualLatency(data->name);
		if (latency > 0)
			text = wxString::Format(_("Device: %s (%d ms)"), data->name.c_str(), latency);
		else
			text = wxString::Format(_("Device: %s"), data->name.c_str());
		m_AudioOutput->SetItemText(selection, text);
	}
	else if (data && data->type == AudioItemData::GROUP_NODE)
	{
		int index;
		std::vector<std::pair<wxString, bool> > groups;
		wxArrayString names;
		groups = GetRemainingAudioGroups(m_AudioOutput->GetItemParent(selection));
		groups.insert(groups.begin(), std::pair<wxString, bool>(data->name, data->left));
		for(unsigned i = 0; i < groups.size(); i++)
			names.Add(wxString::Format(groups[i].second ? _("%s - left") : _("%s - right"), groups[i].first.c_str()));
		index = wxGetSingleChoiceIndex(_("Change audio group"), _("Change audio group"), names, this);
		if (index == -1 || index == 0)
			return;
		data->name = groups[index].first;
		data->left = groups[index].second;
		UpdateVolume(selection, data->volume);
	}
	UpdateButtons();
}

void SettingsAudioOutput::OnOutputProperties(wxCommandEvent& event)
{
	wxTreeItemId selection = m_AudioOutput->GetSelection();
	AudioItemData* data = GetObject(selection);
	if (data && data->type == AudioItemData::AUDIO_NODE)
	{
		int latency = m_Sound.GetSettings().GetAudioDeviceLatency(data->name);
		latency = wxGetNumberFromUser(_("Desired output latency"), _("Desired latency:"), _("Audio device settings"), latency, 1, 999, this);
		if (latency == -1)
			return;
		m_Sound.GetSettings().SetAudioDeviceLatency(data->name, latency);
	}
	else if (data && data->type == AudioItemData::GROUP_NODE)
	{
		wxString current = wxString::Format(wxT("%f"), data->volume >= -120 ? data->volume : -120);
		current = wxGetTextFromUser(_("Please enter new volume in dB:"), _("Change audio group"), current);
		if (current == wxEmptyString)
			return;
		data->volume = wxAtof(current);
		UpdateVolume(selection, data->volume);
	}
	UpdateButtons();
}

void SettingsAudioOutput::Save()
{
	std::vector<GOAudioDeviceConfig> audio_config;
	wxTreeItemId root = m_AudioOutput->GetRootItem();
	wxTreeItemId audio, channel, group;
	wxTreeItemIdValue i, j, k;
	audio = m_AudioOutput->GetFirstChild(root, i);
	while(audio.IsOk())
	{
		GOAudioDeviceConfig conf;
		conf.name = ((AudioItemData*)m_AudioOutput->GetItemData(audio))->name;
		conf.channels = m_AudioOutput->GetChildrenCount(audio, false);
		conf.scale_factors.resize(conf.channels);

		int channel_id = 0;
		channel = m_AudioOutput->GetFirstChild(audio, j);
		while(channel.IsOk())
		{
			group = m_AudioOutput->GetFirstChild(channel, k);
			while(group.IsOk())
			{
				AudioItemData* data = (AudioItemData*)m_AudioOutput->GetItemData(group);
				bool found = false;

				for(unsigned l = 0; l < conf.scale_factors[channel_id].size(); l++)
					if (conf.scale_factors[channel_id][l].name == data->name)
					{
						found = true;
						if (data->left)
							conf.scale_factors[channel_id][l].left = data->volume;
						else
							conf.scale_factors[channel_id][l].right = data->volume;
					}
				if (!found)
				{
					GOAudioGroupOutputConfig gconf;
					gconf.name = data->name;
					gconf.left = -121;
					gconf.right = -121;

					if (data->left)
						gconf.left = data->volume;
					else
						gconf.right = data->volume;

					conf.scale_factors[channel_id].push_back(gconf);
				}

				group = m_AudioOutput->GetNextChild(channel, k);
			}

			channel = m_AudioOutput->GetNextChild(audio, j);
			channel_id ++;
		}

		audio_config.push_back(conf);
		audio = m_AudioOutput->GetNextChild(root, i);
	}
	m_Sound.GetSettings().SetAudioDeviceConfig(audio_config);
}
