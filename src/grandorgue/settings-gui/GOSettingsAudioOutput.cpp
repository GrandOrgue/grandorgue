/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "SettingsAudioOutput.h"

#include <wx/arrstr.h>
#include <wx/button.h>
#include <wx/choicdlg.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/numdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textdlg.h>

#include "settings/GOSettings.h"
#include "sound/GOSound.h"
#include "sound/ports/GOSoundPortFactory.h"


class AudioItemData : public wxTreeItemData
{
public:
	typedef enum { ROOT_NODE, AUDIO_NODE, CHANNEL_NODE, GROUP_NODE } node_type;
	
	AudioItemData()
	{
		type = ROOT_NODE;
		name = wxEmptyString;
		channel = 0;
		latency = 0;
		left = false;
		volume = -121;
	}

	AudioItemData(const wxString& device_name, unsigned desired_latency)
	{
		type = AUDIO_NODE;
		name = device_name;
		channel = 0;
		latency = desired_latency;
		left = false;
		volume = -121;
	}

	AudioItemData(int ch)
	{
		type = CHANNEL_NODE;
		name = wxEmptyString;
		channel = ch;
		latency = 0;
		left = false;
		volume = -121;
	}

	AudioItemData(const wxString& group_name, bool left_channel, float vol)
	{
		type = GROUP_NODE;
		name = group_name;
		channel = 0;
		latency = 0;
		left = left_channel;
		volume = vol;
	}

	node_type type;
	wxString name;
	unsigned channel;
	unsigned latency;
	bool left;
	float volume;
};

BEGIN_EVENT_TABLE(SettingsAudioOutput, wxPanel)
	EVT_TREE_SEL_CHANGED(ID_OUTPUT_LIST, SettingsAudioOutput::OnOutputChanged)
	EVT_BUTTON(ID_OUTPUT_ADD, SettingsAudioOutput::OnOutputAdd)
	EVT_BUTTON(ID_OUTPUT_DEL, SettingsAudioOutput::OnOutputDel)
	EVT_BUTTON(ID_OUTPUT_CHANGE, SettingsAudioOutput::OnOutputChange)
	EVT_BUTTON(ID_OUTPUT_PROPERTIES, SettingsAudioOutput::OnOutputProperties)
	EVT_BUTTON(ID_OUTPUT_DEFAULT, SettingsAudioOutput::OnOutputDefault)
END_EVENT_TABLE()

SettingsAudioOutput::SettingsAudioOutput(GOSound& sound, GOAudioGroupCallback& callback, wxWindow* parent) :
	wxPanel(parent, wxID_ANY),
	GOSettingsPorts(this, GOSoundPortFactory::getInstance(), _("Sound &ports")),
	m_Sound(sound),
	m_Settings(sound.GetSettings()),
	m_GroupCallback(callback)
{
	wxBoxSizer* const item0 = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* item1 = new wxBoxSizer(wxHORIZONTAL);
	wxArrayString choices;
	wxBoxSizer* item2 = new wxStaticBoxSizer(wxVERTICAL, this, _("&Sound output"));
	wxFlexGridSizer* grid = new wxFlexGridSizer(2, 5, 5);

	choices.clear();
	choices.push_back(wxT("44100"));
	choices.push_back(wxT("48000"));
	choices.push_back(wxT("96000"));
	grid->Add(new wxStaticText(this, wxID_ANY, _("Sample Rate:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_SampleRate = new wxChoice(this, ID_SAMPLE_RATE, wxDefaultPosition, wxDefaultSize, choices), 0, wxALL);
	grid->Add(new wxStaticText(this, wxID_ANY, _("Samples per buffer:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(m_SamplesPerBuffer = new wxSpinCtrl(this, ID_SAMPLES_PER_BUFFER, wxEmptyString, wxDefaultPosition, wxDefaultSize), 0, wxALL);
	m_SamplesPerBuffer->SetRange(1, MAX_FRAME_SIZE);
	m_SamplesPerBuffer->SetValue(m_Settings.SamplesPerBuffer());

	m_SampleRate->Select(0);
	for(unsigned i = 0; i < m_SampleRate->GetCount(); i++)
		if (wxString::Format(wxT("%d"), m_Settings.SampleRate()) == m_SampleRate->GetString(i))
			m_SampleRate->Select(i);
	
	item2->Add(grid, 0, wxEXPAND | wxALL, 5);
	item1->Add(item2, 0, wxALL | wxALIGN_TOP, 5);
	
	item1->Add(GetPortsBox(), 1, wxEXPAND | wxALL, 5);
	item0->Add(item1, 1, wxEXPAND | wxALL, 5);

	item2 = new wxStaticBoxSizer(wxVERTICAL, this, _("&Mapping output"));
	m_AudioOutput = new wxTreeCtrl(this, ID_OUTPUT_LIST, wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS | wxTR_SINGLE);
	item2->Add(m_AudioOutput, 1, wxALIGN_LEFT | wxEXPAND);
	item2->AddSpacer(5);

	wxBoxSizer* buttonSizer =  new wxBoxSizer(wxHORIZONTAL);
	buttonSizer->AddSpacer(5);
	m_Change = new wxButton(this, ID_OUTPUT_CHANGE, _("Change"));
	m_Add = new wxButton(this, ID_OUTPUT_ADD, _("&Add"));
	m_Del = new wxButton(this, ID_OUTPUT_DEL, _("&Delete"));
	m_Properties = new wxButton(this, ID_OUTPUT_PROPERTIES, _("Properties"));
	m_Default = new wxButton(this, ID_OUTPUT_DEFAULT, _("Revert to Default"));

	buttonSizer->Add(m_Add, 0, wxALL, 5);
	buttonSizer->Add(m_Del, 0, wxALL, 5);
	buttonSizer->Add(m_Change, wxALL, 5);
	buttonSizer->Add(m_Properties, 0, wxALL, 5);
	buttonSizer->Add(m_Default, 0, wxALL, 5);
	item2->Add(buttonSizer, 0, wxALL, 5);
	item0->Add(item2, 2, wxEXPAND | wxALL, 5);

	m_AudioOutput->AddRoot(_("Audio Output"), -1, -1, new AudioItemData());

	std::vector<GOAudioDeviceConfig> audio_config = m_Sound.GetSettings().GetAudioDeviceConfig();
	for(unsigned i = 0; i < audio_config.size(); i++)
	{
		wxTreeItemId audio = AddDeviceNode(audio_config[i].name, audio_config[i].desired_latency);
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

	// topSizer->AddSpacer(5);
	this->SetSizer(item0);
	item0->Fit(this);

	FillPortsWith(sound.GetSettings().GetSoundPortsConfig());
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
	return AddDeviceNode(name, m_Sound.GetSettings().GetDefaultLatency());
}

wxTreeItemId SettingsAudioOutput::AddDeviceNode(wxString name, unsigned desired_latency)
{
	wxTreeItemId current;
	if (name == wxEmptyString)
	{
	  name = m_Sound.GetDefaultAudioDevice(RenewPortsConfig());
	}
	current = GetDeviceNode(name);
	if (current.IsOk())
		return current;
	current = m_AudioOutput->AppendItem(m_AudioOutput->GetRootItem(), wxEmptyString, -1, -1, new AudioItemData(name, desired_latency));
	UpdateDevice(current);
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

void SettingsAudioOutput::UpdateDevice(const wxTreeItemId& dev)
{
	AudioItemData* data = GetObject(dev);
	wxString text = wxString::Format(_("Device: %s [%d ms requested]"), data->name.c_str(), data->latency);
	m_AudioOutput->SetItemText(dev, text);
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

void SettingsAudioOutput::AssureDeviceList() {
  const GOPortsConfig portsConfig(RenewPortsConfig());
  
  if (m_PortsConfigPopulatedWith != portsConfig) {
    m_DeviceList = m_Sound.GetAudioDevices(portsConfig);
    m_PortsConfigPopulatedWith = portsConfig;
  }
}

std::vector<wxString> SettingsAudioOutput::GetRemainingAudioDevices(
  const wxTreeItemId *ignoreItem
)
{
  AssureDeviceList();
  
  std::vector<wxString> result;
  
  for(unsigned i = 0; i < m_DeviceList.size(); i++)
  {
    const wxTreeItemId item = GetDeviceNode(m_DeviceList[i].name);
    

    if (
      ! item.IsOk() || (ignoreItem != NULL && item.GetID() == ignoreItem->GetID())
    ) result.push_back(m_DeviceList[i].name);
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
		/*
		bool enable = false;
		for(unsigned i = 0; i < m_DeviceList.size(); i++)
			if (m_DeviceList[i].name == data->name)
				if (m_AudioOutput->GetChildrenCount(selection, false) < m_DeviceList[i].channels)
					enable = true;
		if (enable)
			m_Add->Enable();
		else
			m_Add->Disable();
	
		 */
		m_Add->Enable();
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
		{
			if (data->channel > 0)
				m_Del->Enable();
			else
				m_Del->Disable();
		}
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
		m_Add->Enable();
		m_Del->Disable();
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
		unsigned channels = m_AudioOutput->GetChildrenCount(selection, false);
		/*
		for(unsigned i = 0; i < m_DeviceList.size(); i++)
			if (m_DeviceList[i].name == data->name)
				if (channels < m_DeviceList[i].channels) */
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
		std::vector<wxString> devices = GetRemainingAudioDevices(NULL);
		for(unsigned i = 0; i < devices.size(); i++)
			devs.Add(devices[i]);
		index = wxGetSingleChoiceIndex(_("Add new audio device"), _("New audio device"), devs, this);
		if (index == -1)
			return;
		wxMessageBox(_("Using more than one audio interface is currently not supported.") , _("Warning"), wxOK | wxICON_WARNING, this);
		wxTreeItemId audio = AddDeviceNode(devices[index]);
		AddChannelNode(audio, 0);
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
		std::vector<wxString> devices = GetRemainingAudioDevices(& selection);
		
		int initialSelection = 0;
		
		for(unsigned i = 0; i < devices.size(); i++)
		{
		  const wxString devName = devices[i];
		  
		  devs.Add(devName);
		  if (devName == data->name)
		    initialSelection = i;
		}
		index = wxGetSingleChoiceIndex(_("Change audio device"), _("Change audio device"), devs, initialSelection, this);
		if (index <= -1 || devs[index] == data->name)
			return;
		unsigned channels = m_AudioOutput->GetChildrenCount(selection, false);
		bool error = false;
		for(unsigned i = 0; i < m_DeviceList.size(); i++)
			if (m_DeviceList[i].name == devs[index])
				if (channels > m_DeviceList[i].channels)
					error = true;
		if (error)
		{
			wxMessageBox(_("Too many audio channels configured for the new audio interface") , _("Error"), wxOK | wxICON_ERROR, this);
			return;
		}
		data->name = devs[index];

		UpdateDevice(selection);
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
		int latency = data->latency;
		latency = wxGetNumberFromUser(_("Desired output latency"), _("Desired latency:"), _("Audio device settings"), latency, 0, 999, this);
		if (latency == -1)
			return;
		data->latency = latency;
		UpdateDevice(selection);
	}
	else if (data && data->type == AudioItemData::GROUP_NODE)
	{
		wxString current = wxString::Format(wxT("%f"), data->volume >= -120 ? data->volume : -120);
		current = wxGetTextFromUser(_("Please enter new volume in dB:"), _("Change audio group"), current);
		if (current == wxEmptyString)
			return;
		double volume;
		if (!current.ToDouble(&volume) || volume < -120.0 || volume > 40.0)
		{
			wxMessageBox(_("Please enter a volume between -120 and 40 dB") , _("Error"), wxOK | wxICON_ERROR, this);
			return;
		}
		data->volume = volume;
		UpdateVolume(selection, data->volume);
	}
	UpdateButtons();
}

void SettingsAudioOutput::OnOutputDefault(wxCommandEvent& event)
{
	if (wxMessageBox(_("Should the audio config be reverted to the default stereo configuration?"), _("Revert"), wxYES_NO, this) == wxNO)
		return;
	wxTreeItemId root = m_AudioOutput->GetRootItem();
	wxTreeItemId audio, channel, group;
	wxTreeItemIdValue i;
	wxString dev_name = wxEmptyString;
	audio = m_AudioOutput->GetFirstChild(root, i);
	while(audio.IsOk())
	{
		if (dev_name == wxEmptyString)
			dev_name = ((AudioItemData*)m_AudioOutput->GetItemData(audio))->name;
		audio = m_AudioOutput->GetNextChild(root, i);
	}
	m_AudioOutput->DeleteChildren(root);

	std::vector<wxString> groups = m_GroupCallback.GetGroups();

	audio = AddDeviceNode(dev_name);
	channel = AddChannelNode(audio, 0);

	for(unsigned i = 0; i < groups.size(); i++)
	{
		group = AddGroupNode(channel, groups[i], true);
		UpdateVolume(group, 0);
	}

	channel = AddChannelNode(audio, 1);

	for(unsigned i = 0; i < groups.size(); i++)
	{
		group = AddGroupNode(channel, groups[i], false);
		UpdateVolume(group, 0);
	}

	m_AudioOutput->ExpandAll();
	UpdateButtons();
}

void SettingsAudioOutput::Save()
{
  unsigned long sample_rate;

  if (m_SampleRate->GetStringSelection().ToULong(&sample_rate))
	  m_Settings.SampleRate(sample_rate);
  else
	  wxLogError(_("Invalid sample rate"));
  m_Settings.SamplesPerBuffer(m_SamplesPerBuffer->GetValue());
  
  m_Sound.GetSettings().SetSoundPortsConfig(RenewPortsConfig());
  
  std::vector<GOAudioDeviceConfig> audio_config;
  wxTreeItemId root = m_AudioOutput->GetRootItem();
  wxTreeItemId audio, channel, group;
  wxTreeItemIdValue i, j, k;
  
  audio = m_AudioOutput->GetFirstChild(root, i);
  while(audio.IsOk())
  {
	  GOAudioDeviceConfig conf;
	  conf.name = ((AudioItemData*)m_AudioOutput->GetItemData(audio))->name;
	  conf.desired_latency = ((AudioItemData*)m_AudioOutput->GetItemData(audio))->latency;
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
