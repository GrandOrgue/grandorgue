/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include <wx/spinctrl.h>
#include <wx/tglbtn.h>
#include <wx/bookctrl.h>
#include <wx/numdlg.h>
#include <wx/html/helpctrl.h>
#include <wx/statline.h>
#include <wx/listctrl.h>

#include "MIDIListenDialog.h"
#include "SettingsDialog.h"
#include "GrandOrgue.h"
#include "GrandOrgueFrame.h"
#include "GrandOrgueID.h"
#include "GOrgueMidi.h"
#include "GOrgueSound.h"
#include "GOrgueRtHelpers.h"

IMPLEMENT_CLASS(SettingsDialog, wxPropertySheetDialog)

extern GOrgueSound* g_sound;

BEGIN_EVENT_TABLE(SettingsDialog, wxPropertySheetDialog)
	EVT_CHECKLISTBOX(ID_MIDI_DEVICES, SettingsDialog::OnChanged)
	EVT_LISTBOX(ID_MIDI_DEVICES, SettingsDialog::OnDevicesMIDIClick)
	EVT_LISTBOX_DCLICK(ID_MIDI_DEVICES, SettingsDialog::OnDevicesMIDIDoubleClick)
	EVT_BUTTON(ID_MIDI_PROPERTIES, SettingsDialog::OnDevicesMIDIDoubleClick)
	EVT_CHOICE(ID_SOUND_DEVICE, SettingsDialog::OnDevicesSoundChoice)
	EVT_CHOICE(ID_MONO_STEREO, SettingsDialog::OnChanged)
	EVT_CHECKBOX(ID_ENHANCE_SQUASH, SettingsDialog::OnChanged)
	EVT_CHECKBOX(ID_ENHANCE_MANAGE_POLYPHONY, SettingsDialog::OnChanged)
	EVT_CHECKBOX(ID_ENHANCE_ALIGN_RELEASE, SettingsDialog::OnChanged)
	EVT_CHECKBOX(ID_ENHANCE_SCALE_RELEASE, SettingsDialog::OnChanged)
	EVT_CHECKBOX(ID_ENHANCE_RANDOMIZE, SettingsDialog::OnChanged)

	EVT_LIST_ITEM_SELECTED(ID_MIDI_EVENTS, SettingsDialog::OnEventListClick)
	EVT_LIST_ITEM_ACTIVATED(ID_MIDI_EVENTS, SettingsDialog::OnEventListDoubleClick)
	EVT_BUTTON(ID_EVENT_PROPERTIES, SettingsDialog::OnEventListDoubleClickC)

    EVT_LIST_ITEM_SELECTED(ID_ORGAN_EVENTS, SettingsDialog::OnOrganEventListClick)
    EVT_LIST_ITEM_ACTIVATED(ID_MIDI_EVENTS, SettingsDialog::OnOrganEventListDoubleClick)
    EVT_BUTTON(ID_EVENT_ADD, SettingsDialog::OnAddOrgan)
    EVT_BUTTON(ID_EVENT_DEL, SettingsDialog::OnDelOrgan)
    EVT_BUTTON(ID_EVENT_ORGANPROPERTIES, SettingsDialog::OnOrganProperties)

	EVT_BUTTON(wxID_APPLY, SettingsDialog::OnApply)
	EVT_BUTTON(wxID_OK, SettingsDialog::OnOK)
	EVT_BUTTON(wxID_HELP, SettingsDialog::OnHelp)

	EVT_SPINCTRL(ID_LATENCY, SettingsDialog::OnLatencySpinnerChange)

END_EVENT_TABLE()

void SettingsDialog::SetLatencySpinner(int latency)
{

	int corresponding_estimated_latency = pConfig->Read(wxT("Devices/Sound/") + c_sound->GetStringSelection(), -1);

	if (c_latency->GetValue() != latency)
		c_latency->SetValue(latency);

	wxString lat_s = wxT("---");
	if (latency == corresponding_estimated_latency)
	{

		long int lat_l = pConfig->Read(wxT("Devices/Sound/ActualLatency/") + c_sound->GetStringSelection(), -1);
		if (lat_l > 0)
			lat_s.Printf(wxT("%ld ms"), lat_l);

	}

	c_actual_latency->SetLabel(lat_s);

}

#ifdef __WXMSW__
#define SETTINGS_DLG_SIZE wxSize(453,450)
#else
#define SETTINGS_DLG_SIZE wxSize(603,500)
#endif

SettingsDialog::SettingsDialog(wxWindow* win) :
	wxPropertySheetDialog(win, wxID_ANY, _("Audio Settings"), wxDefaultPosition, SETTINGS_DLG_SIZE)
{

	wxASSERT(g_sound);

	b_stereo = g_sound->IsStereo();
	b_squash = wxConfigBase::Get()->Read(wxT("LosslessCompression"), 1);

	pConfig = wxConfigBase::Get();

	SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);

    CreateButtons(wxOK | wxCANCEL | wxHELP);
    //JB: wxAPPLY not available in recent versions of wxWidgets

    wxBookCtrlBase* notebook = GetBookCtrl();

    wxPanel* devices  = CreateDevicesPage(notebook);
    wxPanel* messages = CreateMessagesPage(notebook);
    wxPanel* organs = CreateOrganPage(notebook);

    notebook->AddPage(devices,  _("Devices"));
    notebook->AddPage(messages, _("MIDI Messages"));
    notebook->AddPage(organs, _("Organs"));

	//this->FindWindowById(wxID_APPLY, this)->Disable();

    //LayoutDialog();

	page2list->SetColumnWidth(0, wxLIST_AUTOSIZE);
	page2list->SetColumnWidth(0, page2list->GetColumnWidth(0) + 16);
	page2list->SetColumnWidth(2, 62);
	page2list->SetColumnWidth(3, 100);

	page2list->SetColumnWidth(1, page2list->GetClientSize().GetWidth() - page2list->GetColumnWidth(0) - 108);

	g_sound->SetLogSoundErrorMessages(true);
}

SettingsDialog::~SettingsDialog()
{
	g_sound->SetLogSoundErrorMessages(false);
    if ((b_stereo != g_sound->IsStereo() || b_squash != wxConfigBase::Get()->Read(wxT("LosslessCompression"), 1)))
    {
        if (::wxMessageBox(_("Stereo mode and lossless compression won't take\neffect unless the sample set is reloaded.\n\nWould you like to reload the sample set now?"), wxT(APP_NAME), wxYES_NO | wxICON_QUESTION) == wxYES)
        {
            wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_FILE_RELOAD);
            ::wxGetApp().frame->AddPendingEvent(event);
        }
    }
}

void SettingsDialog::UpdateSoundStatus()
{

	SetLatencySpinner(pConfig->Read(wxT("Devices/Sound/") + c_sound->GetStringSelection(), 0L));
	c_format->SetLabel(wxString(GOrgueRtHelpers::GetAudioFormatName(g_sound->GetAudioFormat())));

}

wxPanel* SettingsDialog::CreateDevicesPage(wxWindow* parent)
{
	int i;
	std::map<wxString, GOrgueSound::GO_SOUND_DEV_CONFIG>::iterator it1;
	std::map<wxString, int>::iterator it2;

	wxPanel* panel = new wxPanel(parent, wxID_ANY);
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* item0 = new wxBoxSizer(wxHORIZONTAL);

	wxArrayString choices;

	if (!g_sound)
		throw "g_sound has not been allocated";

	for (it2 = g_sound->GetMidi().GetDevices().begin(); it2 != g_sound->GetMidi().GetDevices().end(); it2++)
	{
		choices.push_back(it2->first);
		page1checklistdata.push_back(pConfig->Read(wxT("Devices/MIDI/") + it2->first, 0L));
	}
	wxBoxSizer* item3 = new wxStaticBoxSizer(wxVERTICAL, panel, _("MIDI &input devices"));
	item0->Add(item3, 0, wxEXPAND | wxALL, 5);
	page1checklist = new wxCheckListBox(panel, ID_MIDI_DEVICES, wxDefaultPosition, wxDefaultSize, choices);
	for (i = 0; i < (int)page1checklistdata.size(); i++)
	{
        if (page1checklistdata[i] < 0)
			page1checklistdata[i] = -page1checklistdata[i] - 1;
		else
			page1checklist->Check(i);
	}
	item3->Add(page1checklist, 1, wxEXPAND | wxALL, 5);
	page1button = new wxButton(panel, ID_MIDI_PROPERTIES, _("A&dvanced..."));
	page1button->Disable();
	item3->Add(page1button, 0, wxALIGN_RIGHT | wxALL, 5);
	choices.clear();

	wxBoxSizer* item9 = new wxBoxSizer(wxVERTICAL);
	item0->Add(item9, 0, wxEXPAND | wxALL, 0);

	for (it1 = g_sound->GetAudioDevices().begin(); it1 != g_sound->GetAudioDevices().end(); it1++)
		choices.push_back(it1->first);
	wxBoxSizer* item1 = new wxStaticBoxSizer(wxVERTICAL, panel, _("Sound &output device"));
	item9->Add(item1, 0, wxEXPAND | wxALL, 5);
	c_sound = new wxChoice(panel, ID_SOUND_DEVICE, wxDefaultPosition, wxDefaultSize, choices);
	c_sound->SetStringSelection(g_sound->GetDefaultAudioDevice());
	c_sound->SetStringSelection(pConfig->Read(wxT("Devices/DefaultSound"), wxEmptyString));
	item1->Add(c_sound, 0, wxEXPAND | wxALL, 5);

	choices.clear();
	choices.push_back(_("Mono"));
	choices.push_back(_("Stereo"));

	wxFlexGridSizer* grid = new wxFlexGridSizer(4, 2, 5, 5);
	grid->Add(new wxStaticText(panel, wxID_ANY, _("Output Resolution:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM, 5);
	grid->Add(c_format = new wxStaticText(panel, wxID_ANY, wxEmptyString));

	/* estimated latency */
	grid->Add(new wxStaticText(panel, wxID_ANY, _("Desired &Latency:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
	wxBoxSizer* latency = new wxBoxSizer(wxHORIZONTAL);
	latency->Add(c_latency = new wxSpinCtrl(panel, ID_LATENCY, wxEmptyString, wxDefaultPosition, wxSize(48, wxDefaultCoord), wxSP_ARROW_KEYS, 1, 999), 0);
	latency->Add(new wxStaticText(panel, wxID_ANY, _("ms")), 0, wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	grid->Add(latency, 0);

	/* achieved latency for the above estimate */
	grid->Add(new wxStaticText(panel, wxID_ANY, _("Achieved Latency:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM, 5);
	grid->Add(c_actual_latency = new wxStaticText(panel, wxID_ANY, wxEmptyString));

	grid->Add(new wxStaticText(panel, wxID_ANY, _("Load &stereo samples in:")), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	grid->Add(c_stereo = new wxChoice(panel, ID_MONO_STEREO, wxDefaultPosition, wxDefaultSize, choices), 0, wxALL);
	item1->Add(grid, 0, wxEXPAND | wxALL, 5);

	UpdateSoundStatus();
	c_stereo->Select(pConfig->Read(wxT("StereoEnabled"), 0L));

	wxBoxSizer* item6 = new wxStaticBoxSizer(wxVERTICAL, panel, _("&Enhancements"));
	item9->Add(item6, 0, wxEXPAND | wxALL, 5);
    item6->Add(c_squash = new wxCheckBox(panel, ID_ENHANCE_SQUASH,           _("Lossless compression")          ), 0, wxEXPAND | wxALL, 5);
    item6->Add(c_limit  = new wxCheckBox(panel, ID_ENHANCE_MANAGE_POLYPHONY, _("Active polyphony management")   ), 0, wxEXPAND | wxALL, 5);
    item6->Add(c_align  = new wxCheckBox(panel, ID_ENHANCE_ALIGN_RELEASE,    _("Release sample phase alignment")), 0, wxEXPAND | wxALL, 5);
    item6->Add(c_scale  = new wxCheckBox(panel, ID_ENHANCE_SCALE_RELEASE,    _("Release sample scaling"        )), 0, wxEXPAND | wxALL, 5);
    item6->Add(c_random = new wxCheckBox(panel, ID_ENHANCE_RANDOMIZE,        _("Randomize pipe speaking"       )), 0, wxEXPAND | wxALL, 5);
	if (pConfig->Read(wxT("LosslessCompression"), 1))
		c_squash->SetValue(true);
	if (pConfig->Read(wxT("ManagePolyphony"), 1))
		c_limit ->SetValue(true);
	if (pConfig->Read(wxT("AlignRelease"), 1))
		c_align ->SetValue(true);
	if (pConfig->Read(wxT("ScaleRelease"), 1))
		c_scale ->SetValue(true);
	if (pConfig->Read(wxT("RandomizeSpeaking"), 1))
		c_random->SetValue(true);

    topSizer->Add(item0, 1, wxEXPAND | wxALIGN_CENTER | wxALL, 5);
	topSizer->AddSpacer(5);
	panel->SetSizer(topSizer);
	topSizer->Fit(panel);
	return panel;
}

wxPanel* SettingsDialog::CreateMessagesPage(wxWindow* parent)
{
	wxPanel* panel = new wxPanel(parent, wxID_ANY);
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	topSizer->AddSpacer(5);

	page2list = new wxListView(panel, ID_MIDI_EVENTS, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES);
	page2list->InsertColumn(0, _("Action"));
	page2list->InsertColumn(1, _("Event"));
	page2list->InsertColumn(2, _("Channel"));
	page2list->InsertColumn(3, _("Data/Offset"));
    topSizer->Add(page2list, 1, wxEXPAND | wxALL, 5);
	page2button = new wxButton(panel, ID_EVENT_PROPERTIES, _("P&roperties..."));
	page2button->Disable();
	topSizer->Add(page2button, 0, wxALIGN_RIGHT | wxALL, 5);

	for (unsigned i = 0; i < GOrgueMidi::NbMidiEvents(); i++)
	{
		page2list->InsertItem(i, GOrgueMidi::GetMidiEventTitle(i));
		UpdateMessages(i);
	}


	topSizer->AddSpacer(5);
	panel->SetSizer(topSizer);
	topSizer->Fit(panel);

	return panel;
}

wxPanel* SettingsDialog::CreateOrganPage(wxWindow* parent)
{
	wxPanel* panel = new wxPanel(parent, wxID_ANY);
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	topSizer->AddSpacer(5);

	organlist = new wxListView(panel, ID_ORGAN_EVENTS, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES);
	organlist->InsertColumn(0, _("Organ"));
	organlist->InsertColumn(1, _("Event"));
	organlist->InsertColumn(2, _("Channel"));
	organlist->InsertColumn(3, _("Data"));
    topSizer->Add(organlist, 1, wxEXPAND | wxALL, 5);

    wxBoxSizer* buttonSizer =  new wxBoxSizer(wxHORIZONTAL);
    buttonSizer->AddSpacer(5);
	propButton = new wxButton(panel, ID_EVENT_ORGANPROPERTIES, _("P&roperties..."));
	propButton->Disable();
	addButton = new wxButton(panel, ID_EVENT_ADD, _("&Add"));
	delButton = new wxButton(panel, ID_EVENT_DEL, _("&Delete"));
	delButton->Disable();

	buttonSizer->Add(propButton, 0, wxALIGN_RIGHT | wxALL, 5);
    buttonSizer->Add(addButton, 0, wxALIGN_LEFT | wxALL, 5);
	buttonSizer->Add(delButton, 0, wxALIGN_LEFT | wxALL, 5);
    topSizer->Add(buttonSizer, 0, wxALL, 5);

    long count=pConfig->Read(wxT("OrganMIDI/Count"), 0L);
    for (long i=0; i<count; i++) {
        wxString itemstr=wxT("OrganMIDI/Organ")+wxString::Format(wxT("%ld"), i);
        wxString file=pConfig->Read(itemstr+wxT(".file"), wxT(""));
        int j=pConfig->Read(itemstr+wxT(".midi"), 0L);
        organlist->InsertItem(i, file);
        organlist->SetItemData(i, j);
        UpdateOrganMessages(i);
    }
    if (count>0) {
        organlist->SetColumnWidth(0, wxLIST_AUTOSIZE);
        organlist->SetColumnWidth(1, wxLIST_AUTOSIZE);
        organlist->SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER);
        organlist->SetColumnWidth(3, wxLIST_AUTOSIZE_USEHEADER);
    }

	topSizer->AddSpacer(5);
	panel->SetSizer(topSizer);
	topSizer->Fit(panel);
	return panel;
}


void SettingsDialog::OnOrganEventListClick(wxListEvent& event) {
  delButton->Enable();
  propButton->Enable();
}

void SettingsDialog::OnOrganEventListDoubleClick(wxListEvent& event) {
}

void SettingsDialog::OnAddOrgan(wxCommandEvent& event) {
    //ask for .organ file
    wxFileDialog dlg(this, _("Choose a file"), wxT(""), wxT(""), _("*.organ"), wxFILE_MUST_EXIST | wxOPEN);
    if (dlg.ShowModal()==wxID_OK) {
        organlist->InsertItem(organlist->GetItemCount(), dlg.GetPath());
        organlist->SetColumnWidth(0, wxLIST_AUTOSIZE);
    }
}

void SettingsDialog::OnDelOrgan(wxCommandEvent& event) {
    int index=organlist->GetFirstSelected();
    wxString itemstr=wxT("OrganMIDI/Organ")+wxString::Format(wxT("%d"), index);
    organlist->DeleteItem(index);
    pConfig->DeleteEntry(itemstr+wxT(".file"));
    pConfig->DeleteEntry(itemstr+wxT(".midi"));
}

void SettingsDialog::OnOrganProperties(wxCommandEvent& event)
{

	int index = organlist->GetFirstSelected();

	MIDIListenDialog dlg
		(this
		,organlist->GetItemText(index)
		,MIDIListenDialog::LSTN_SETTINGSDLG_MEMORY_OR_ORGAN
		,organlist->GetItemData(index)
		);

	if (dlg.ShowModal() == wxID_OK)
	{
		organlist->SetItemData(index, dlg.GetEvent());
		UpdateOrganMessages(index);
		organlist->SetColumnWidth(1, wxLIST_AUTOSIZE);
		organlist->SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER);
		organlist->SetColumnWidth(3, wxLIST_AUTOSIZE_USEHEADER);
	}

}

void SettingsDialog::UpdateMessages(int i)
{

	MIDIListenDialog::LISTEN_DIALOG_TYPE type = GOrgueMidi::GetMidiEventListenDialogType(i);
	int j = pConfig->Read(wxString(wxT("MIDI/")) + GOrgueMidi::GetMidiEventTitle(i), 0L);
	page2list->SetItem(i, 1, MIDIListenDialog::GetEventTitle(j, type));
	page2list->SetItem(i, 2, MIDIListenDialog::GetEventChannelString(j));

	if (type != 3 && j)
	{
		int offset = (j & 0xF000) == 0xC000 ? 1 : 0;
		wxString temp;
		if (type == 1 ) // when type == 1 (keyboards) data contained in lower byte correspond to note offset
		{
			if ((j&0xFF)>0x7F) offset = -140; // negative numbers are coded as 128=-12 140=0
		}
		temp << ((j & 0xFF) + offset );
		page2list->SetItem(i, 3, temp);
	}

	page2list->SetItemData(i, j);

}

void SettingsDialog::UpdateOrganMessages(int i)
{

	int j = organlist->GetItemData(i);
	organlist->SetItem(i, 1, MIDIListenDialog::GetEventTitle(j, MIDIListenDialog::LSTN_SETTINGSDLG_MEMORY_OR_ORGAN));
	organlist->SetItem(i, 2, MIDIListenDialog::GetEventChannelString(j));
	// data
	wxString temp;
	int offset = (j & 0xF000) == 0xC000 ? 1 : 0;
	temp << ((j & 0x7F) + offset);
	organlist->SetItem(i, 3, temp);
	organlist->SetItemData(i, j);

}

void SettingsDialog::OnChanged(wxCommandEvent& event)
{

}

void SettingsDialog::OnDevicesSoundChoice(wxCommandEvent& event)
{

	int lat = pConfig->Read(wxT("Devices/Sound/") + c_sound->GetStringSelection(), 15);
	SetLatencySpinner(lat);

}

void SettingsDialog::OnLatencySpinnerChange(wxSpinEvent& event)
{

	SetLatencySpinner(event.GetPosition());

}

void SettingsDialog::OnDevicesMIDIClick(wxCommandEvent& event)
{
	page1button->Enable();
}

void SettingsDialog::OnDevicesMIDIDoubleClick(wxCommandEvent& event)
{
	page1button->Enable();
	int index = page1checklist->GetSelection();
	int result = ::wxGetNumberFromUser(_("A channel offset allows the use of two MIDI\ninterfaces with conflicting MIDI channels. For\nexample, applying a channel offset of 8 to\none of the MIDI interfaces would cause that\ninterface's channel 1 to appear as channel 9,\nchannel 2 to appear as channel 10, and so on."), _("Channel offset:"), page1checklist->GetString(index), page1checklistdata[index], 0, 15, this);
	if (result >= 0 && result != page1checklistdata[index])
	{
		page1checklistdata[index] = result;
		OnChanged(event);
	}
}

void SettingsDialog::OnEventListClick(wxListEvent& event)
{
	page2button->Enable();
}

void SettingsDialog::OnEventListDoubleClick(wxListEvent& event)
{

	page2button->Enable();

	int index = page2list->GetFirstSelected();

	MIDIListenDialog dlg
		(this
		,page2list->GetItemText(index)
		,GOrgueMidi::GetMidiEventListenDialogType(index)
		,page2list->GetItemData(index)
		);

	if (dlg.ShowModal() == wxID_OK)
	{
		pConfig->Write(wxString(wxT("MIDI/")) + GOrgueMidi::GetMidiEventTitle(index), dlg.GetEvent());
		UpdateMessages(index);
		OnChanged(event);
	}

}

void SettingsDialog::OnEventListDoubleClickC(wxCommandEvent& event)
{
    wxListEvent listevent;
	OnEventListDoubleClick(listevent);
}

void SettingsDialog::OnApply(wxCommandEvent& event)
{
	DoApply();
}

void SettingsDialog::OnHelp(wxCommandEvent& event)
{
    ::wxGetApp().m_help->Display(((wxNotebook*)GetBookCtrl())->GetSelection() ? _("MIDI Settings") : _("Device Settings"));
}

void SettingsDialog::OnOK(wxCommandEvent& event)
{

	pConfig->Write(wxT("OrganMIDI/Count"), organlist->GetItemCount());
	for (int i=0; i<organlist->GetItemCount(); i++)
	{
		wxString itemstr=wxString::Format(wxT("OrganMIDI/Organ%d"), i);
		pConfig->Write(itemstr+wxT(".file"), organlist->GetItemText(i));
		pConfig->Write(itemstr+wxT(".midi"), (long)organlist->GetItemData(i));
	}

	if (DoApply())
		event.Skip();

}

bool SettingsDialog::DoApply()
{

	if (!(this->Validate()))
		return false;

	for (size_t i = 0;	i < page1checklist->GetCount(); i++)
	{
		int j;

		j = page1checklistdata[i];
		if (!page1checklist->IsChecked(i))
            j = -j - 1;
        pConfig->Write(wxT("Devices/MIDI/") + page1checklist->GetString(i), j);
	}
	pConfig->Write(wxT("Devices/DefaultSound"), c_sound->GetStringSelection());
	pConfig->Write(wxT("Devices/Sound/") + c_sound->GetStringSelection(), c_latency->GetValue());
	pConfig->Write(wxT("StereoEnabled"), c_stereo->GetSelection());
	pConfig->Write(wxT("LosslessCompression"), (long)c_squash->IsChecked());
	pConfig->Write(wxT("ManagePolyphony"), (long)c_limit->IsChecked());
	pConfig->Write(wxT("AlignRelease"), (long)c_align->IsChecked());
	pConfig->Write(wxT("ScaleRelease"), (long)c_scale->IsChecked());
	pConfig->Write(wxT("RandomizeSpeaking"), (long)c_random->IsChecked());

    g_sound->ResetSound(NULL);
    UpdateSoundStatus();

	return true;
}

