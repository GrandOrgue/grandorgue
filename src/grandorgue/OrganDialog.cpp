/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2014 GrandOrgue contributors (see AUTHORS)
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

#include "OrganDialog.h"

#include "GOrgueEvent.h"
#include "GOrgueRank.h"
#include "GOrguePipe.h"
#include "GOrgueSettings.h"
#include "GOrgueWindchest.h"
#include "GrandOrgueFile.h"
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choicdlg.h>
#include <wx/choice.h>
#include <wx/combobox.h>
#include <wx/sizer.h>
#include <wx/spinbutt.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/treectrl.h>

class OrganTreeItemData : public wxTreeItemData
{
public:
	OrganTreeItemData(GOrguePipeConfig& c)
	{
		config = &c;
	}

	GOrguePipeConfig* config;
};

DEFINE_LOCAL_EVENT_TYPE(wxEVT_TREE_UPDATED)

BEGIN_EVENT_TABLE(OrganDialog, wxDialog)
	EVT_BUTTON(ID_EVENT_APPLY, OrganDialog::OnEventApply)
	EVT_BUTTON(ID_EVENT_RESET, OrganDialog::OnEventReset)
	EVT_BUTTON(ID_EVENT_DEFAULT, OrganDialog::OnEventDefault)
	EVT_BUTTON(wxID_OK, OrganDialog::OnOK)
	EVT_BUTTON(wxID_CANCEL, OrganDialog::OnCancel)
	EVT_TREE_SEL_CHANGING(ID_EVENT_TREE, OrganDialog::OnTreeChanging)
	EVT_TREE_SEL_CHANGED(ID_EVENT_TREE, OrganDialog::OnTreeChanged)
	EVT_COMMAND(ID_EVENT_TREE, wxEVT_TREE_UPDATED, OrganDialog::OnTreeUpdated)
	EVT_TEXT(ID_EVENT_AMPLITUDE, OrganDialog::OnAmplitudeChanged)
	EVT_SPIN(ID_EVENT_AMPLITUDE_SPIN, OrganDialog::OnAmplitudeSpinChanged)
	EVT_TEXT(ID_EVENT_GAIN, OrganDialog::OnGainChanged)
	EVT_SPIN(ID_EVENT_GAIN_SPIN, OrganDialog::OnGainSpinChanged)
	EVT_TEXT(ID_EVENT_TUNING, OrganDialog::OnTuningChanged)
	EVT_SPIN(ID_EVENT_TUNING_SPIN, OrganDialog::OnTuningSpinChanged)
	EVT_TEXT(ID_EVENT_DELAY, OrganDialog::OnDelayChanged)
	EVT_SPIN(ID_EVENT_DELAY_SPIN, OrganDialog::OnDelaySpinChanged)
	EVT_TEXT(ID_EVENT_AUDIO_GROUP, OrganDialog::OnAudioGroupChanged)
	EVT_BUTTON(ID_EVENT_AUDIO_GROUP_ASSISTANT, OrganDialog::OnAudioGroupAssitant)
	EVT_CHOICE(ID_EVENT_BITS_PER_SAMPLE, OrganDialog::OnBitsPerSampleChanged)
	EVT_CHOICE(ID_EVENT_COMPRESS, OrganDialog::OnCompressChanged)
	EVT_CHOICE(ID_EVENT_CHANNELS, OrganDialog::OnChannelsChanged)
	EVT_CHOICE(ID_EVENT_LOOP_LOAD, OrganDialog::OnLoopLoadChanged)
	EVT_CHOICE(ID_EVENT_ATTACK_LOAD, OrganDialog::OnAttackLoadChanged)
	EVT_CHOICE(ID_EVENT_RELEASE_LOAD, OrganDialog::OnReleaseLoadChanged)
END_EVENT_TABLE()

OrganDialog::OrganDialog (GOrgueDocument* doc, wxWindow* parent, GrandOrgueFile* organfile) :
	wxDialog(parent, wxID_ANY, _("Organ settings"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE),
	GOrgueView(doc, this),
	m_organfile(organfile),
	m_Apply(NULL),
	m_Reset(NULL),
	m_Last(NULL),
	m_LoadChangeCnt(0),
	m_ModalDialog(NULL),
	m_Destroying(false),
	m_DestroyPending(false)
{
	wxArrayString choices;

	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);

	topSizer->Add(mainSizer, 0, wxALL, 6);

	m_Tree = new wxTreeCtrl(this, ID_EVENT_TREE, wxDefaultPosition, wxSize(300, 400), wxTR_HAS_BUTTONS | wxTR_MULTIPLE);
	wxBoxSizer* Sizer1 = new wxBoxSizer(wxVERTICAL);
	Sizer1->Add(m_Tree, wxALIGN_TOP | wxEXPAND);

	m_AudioGroupAssistant = new wxButton(this, ID_EVENT_AUDIO_GROUP_ASSISTANT, _("Distribute audio groups"));
	Sizer1->Add(m_AudioGroupAssistant, wxALIGN_CENTER);

	mainSizer->Add(Sizer1, wxALIGN_LEFT | wxEXPAND);
	mainSizer->AddSpacer(5);

	wxBoxSizer* settingSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* box1 = new wxStaticBoxSizer(wxVERTICAL, this, _("Settings"));
	wxFlexGridSizer* grid = new wxFlexGridSizer(4, 2, 5, 5);
	box1->Add(grid, 0, wxEXPAND | wxALL, 5);
	settingSizer->Add(box1, 0, wxEXPAND | wxALL, 5);

	grid->Add(new wxStaticText(this, wxID_ANY, _("Amplitude:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM, 5);
	wxBoxSizer* box2 = new wxBoxSizer(wxHORIZONTAL);
	m_Amplitude = new wxTextCtrl(this, ID_EVENT_AMPLITUDE, wxEmptyString);
	m_AmplitudeSpin = new wxSpinButton(this, ID_EVENT_AMPLITUDE_SPIN); 
	box2->Add(m_Amplitude);
	box2->Add(m_AmplitudeSpin);
	grid->Add(box2);
	m_AmplitudeSpin->SetRange(0, 1000);

	grid->Add(new wxStaticText(this, wxID_ANY, _("Gain (dB):")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM, 5);
	box2 = new wxBoxSizer(wxHORIZONTAL);
	m_Gain = new wxTextCtrl(this, ID_EVENT_GAIN, wxEmptyString);
	m_GainSpin = new wxSpinButton(this, ID_EVENT_GAIN_SPIN); 
	box2->Add(m_Gain);
	box2->Add(m_GainSpin);
	grid->Add(box2);
	m_GainSpin->SetRange(-120, 40);

	grid->Add(new wxStaticText(this, wxID_ANY, _("Tuning (Cent):")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM, 5);
	box2 = new wxBoxSizer(wxHORIZONTAL);
	m_Tuning = new wxTextCtrl(this, ID_EVENT_TUNING, wxEmptyString);
	m_TuningSpin = new wxSpinButton(this, ID_EVENT_TUNING_SPIN); 
	box2->Add(m_Tuning);
	box2->Add(m_TuningSpin);
	grid->Add(box2);
	m_TuningSpin->SetRange(-1200, 1200);

	grid->Add(new wxStaticText(this, wxID_ANY, _("Tracker (ms):")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM, 5);
	box2 = new wxBoxSizer(wxHORIZONTAL);
	m_Delay = new wxTextCtrl(this, ID_EVENT_DELAY, wxEmptyString);
	m_DelaySpin = new wxSpinButton(this, ID_EVENT_DELAY_SPIN); 
	box2->Add(m_Delay);
	box2->Add(m_DelaySpin);
	grid->Add(box2);
	m_DelaySpin->SetRange(0, 10000);

	grid->Add(new wxStaticText(this, wxID_ANY, _("Audio group:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM, 5);
	m_AudioGroup = new wxComboBox(this, ID_EVENT_AUDIO_GROUP, wxEmptyString);
	grid->Add(m_AudioGroup);
	m_AudioGroup->Append(wxEmptyString);
	std::vector<wxString> audio_groups = m_organfile->GetSettings().GetAudioGroups();
	for(unsigned i = 0; i < audio_groups.size(); i++)
		m_AudioGroup->Append(audio_groups[i]);

	m_AudioGroup->SetValue(wxT(" "));
	m_LastAudioGroup = m_AudioGroup->GetValue();

	box1 = new wxStaticBoxSizer(wxVERTICAL, this, _("Sample Loading"));
	grid = new wxFlexGridSizer(6, 2, 5, 5);
	box1->Add(grid, 0, wxEXPAND | wxALL, 5);
	settingSizer->Add(box1, 0, wxEXPAND | wxALL, 5);

	choices.clear();
	choices.push_back(_("Parent default"));
	choices.push_back(_("8 bits"));
	choices.push_back(_("12 bits"));
	choices.push_back(_("16 bits"));
	choices.push_back(_("20 bits"));
	choices.push_back(_("24 bits"));
	grid->Add(new wxStaticText(this, wxID_ANY, _("Sample Size:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM, 5);
	m_BitsPerSample = new wxChoice(this, ID_EVENT_BITS_PER_SAMPLE, wxDefaultPosition, wxDefaultSize, choices);
	grid->Add(m_BitsPerSample);

	choices.clear();
	choices.push_back(_("Parent default"));
	choices.push_back(_("Disabled"));
	choices.push_back(_("Enabled"));
	grid->Add(new wxStaticText(this, wxID_ANY, _("Lossless compression:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM, 5);
	m_Compress = new wxChoice(this, ID_EVENT_COMPRESS, wxDefaultPosition, wxDefaultSize, choices);
	grid->Add(m_Compress);

	choices.clear();
	choices.push_back(_("Parent default"));
	choices.push_back(_("Don't load"));
	choices.push_back(_("Mono"));
	choices.push_back(_("Stereo"));
	grid->Add(new wxStaticText(this, wxID_ANY, _("Sample channels:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM, 5);
	m_Channels = new wxChoice(this, ID_EVENT_CHANNELS, wxDefaultPosition, wxDefaultSize, choices);
	grid->Add(m_Channels);

	choices.clear();
	choices.push_back(_("Parent default"));
	choices.push_back(_("First loop"));
	choices.push_back(_("Longest loop"));
	choices.push_back(_("All loops"));
	grid->Add(new wxStaticText(this, wxID_ANY, _("Loop loading:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM, 5);
	m_LoopLoad = new wxChoice(this, ID_EVENT_LOOP_LOAD, wxDefaultPosition, wxDefaultSize, choices);
	grid->Add(m_LoopLoad);

	choices.clear();
	choices.push_back(_("Parent default"));
	choices.push_back(_("Single attack"));
	choices.push_back(_("All"));
	grid->Add(new wxStaticText(this, wxID_ANY, _("Attack loading:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM, 5);
	m_AttackLoad = new wxChoice(this, ID_EVENT_ATTACK_LOAD, wxDefaultPosition, wxDefaultSize, choices);
	grid->Add(m_AttackLoad);

	choices.clear();
	choices.push_back(_("Parent default"));
	choices.push_back(_("Single release"));
	choices.push_back(_("All"));
	grid->Add(new wxStaticText(this, wxID_ANY, _("Release loading:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM, 5);
	m_ReleaseLoad = new wxChoice(this, ID_EVENT_RELEASE_LOAD, wxDefaultPosition, wxDefaultSize, choices);
	grid->Add(m_ReleaseLoad);

	m_BitsPerSample->SetSelection(wxNOT_FOUND);
	m_LastBitsPerSample = m_BitsPerSample->GetSelection();
	m_Compress->SetSelection(wxNOT_FOUND);
	m_LastCompress = m_Compress->GetSelection();
	m_Channels->SetSelection(wxNOT_FOUND);
	m_LastChannels = m_Channels->GetSelection();
	m_LoopLoad->SetSelection(wxNOT_FOUND);
	m_LastLoopLoad = m_LoopLoad->GetSelection();
	m_AttackLoad->SetSelection(wxNOT_FOUND);
	m_LastAttackLoad = m_AttackLoad->GetSelection();
	m_ReleaseLoad->SetSelection(wxNOT_FOUND);
	m_LastReleaseLoad = m_ReleaseLoad->GetSelection();

	wxBoxSizer* buttons = new wxBoxSizer(wxHORIZONTAL);
	m_Apply = new wxButton(this, ID_EVENT_APPLY, _("Apply"));
	m_Reset = new wxButton(this, ID_EVENT_RESET, _("Reset"));
	m_Default = new wxButton(this, ID_EVENT_DEFAULT, _("Default"));
	buttons->Add(m_Default);
	buttons->Add(m_Reset);
	buttons->Add(m_Apply);
	settingSizer->Add(buttons);

	wxBoxSizer* box3 = new wxStaticBoxSizer(wxVERTICAL, this, _("Tuning and Voicing"));
	box3->Add(m_IgnorePitch = new wxCheckBox (this, ID_EVENT_IGNORE_PITCH, _("Ignore pitch info in organ samples wav files"       )), 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxBOTTOM, 5);
	if (m_organfile->GetIgnorePitch())
		m_IgnorePitch->SetValue(true);
		
	settingSizer->Add(box3, 0, wxEXPAND | wxALL, 4);
	mainSizer->Add(settingSizer, wxALIGN_RIGHT | wxEXPAND);	
	
	topSizer->Add(new wxStaticLine(this), 0, wxEXPAND | wxALL, 5);
	topSizer->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALIGN_RIGHT | wxALL, 5);
	topSizer->AddSpacer(5);
	SetSizer(topSizer);

	FillTree();
	Load();
	
	topSizer->Fit(this);
}

OrganDialog::~OrganDialog()
{
}

bool OrganDialog::CloseModal()
{
	if (m_DestroyPending)
	{
		wxDialog::Destroy();
		return true;
	}
	if (m_ModalDialog)
	{
		wxDialog* dlg = m_ModalDialog;
		m_ModalDialog = NULL;
		dlg->EndModal(wxID_CANCEL);
		if (m_Destroying)
			m_DestroyPending = true;
		return true;
	}
	return false;
}

bool OrganDialog::Destroy()
{
	Hide();
	m_Destroying = true;
	if (CloseModal())
		return true;
	return wxDialog::Destroy();
}


void OrganDialog::SetEmpty(wxChoice* choice)
{
	int index = choice->FindString(wxEmptyString);
	if (index == wxNOT_FOUND)
		index = choice->Append(wxEmptyString);
	choice->SetSelection(index);
}

void OrganDialog::RemoveEmpty(wxChoice* choice)
{
	int sel = choice->GetSelection();
	int index = choice->FindString(wxEmptyString);
	if (index != wxNOT_FOUND)
		choice->Delete(index);
	choice->SetSelection(sel);
}

void OrganDialog::Load()
{
	wxArrayTreeItemIds entries;

	m_Tree->GetSelections(entries);
	for(unsigned i = 0; i < entries.size(); i++)
	{
		if (!m_Tree->GetItemData(entries[i]))
		{
			wxLogError(_("Invalid item selected: %s"), m_Tree->GetItemText(entries[i]).c_str());
			entries.RemoveAt(i, 1);
			i--;
		}
	}

	if (entries.size() == 0)
	{
		m_Last = NULL;
		m_Amplitude->ChangeValue(wxEmptyString);
		m_Amplitude->Disable();
		m_AmplitudeSpin->Disable();
		m_Gain->ChangeValue(wxEmptyString);
		m_Gain->Disable();
		m_GainSpin->Disable();
		m_Tuning->ChangeValue(wxEmptyString);
		m_Tuning->Disable();
		m_TuningSpin->Disable();
		m_Delay->ChangeValue(wxEmptyString);
		m_Delay->Disable();
		m_DelaySpin->Disable();
		m_AudioGroup->Disable();
		m_BitsPerSample->Disable();
		m_Compress->Disable();
		m_Channels->Disable();
		m_LoopLoad->Disable();
		m_AttackLoad->Disable();
		m_ReleaseLoad->Disable();
		m_Apply->Disable();
		m_Reset->Disable();
		m_Default->Disable();
		m_AudioGroupAssistant->Disable();
		return;
	}

	m_AudioGroupAssistant->Enable();

	if (entries.size() > 1)
	{
		if (!m_Amplitude->IsModified())
			m_Amplitude->ChangeValue(wxEmptyString);
		if (!m_Gain->IsModified())
			m_Gain->ChangeValue(wxEmptyString);
		if (!m_Tuning->IsModified())
			m_Tuning->ChangeValue(wxEmptyString);
		if (!m_Delay->IsModified())
			m_Delay->ChangeValue(wxEmptyString);
		if (m_AudioGroup->GetValue() == m_LastAudioGroup)
		{
			m_AudioGroup->SetValue(wxT(" "));
			m_LastAudioGroup = m_AudioGroup->GetValue();
		}
		if (m_BitsPerSample->GetSelection() == m_LastBitsPerSample)
		{
			SetEmpty(m_BitsPerSample);
			m_LastBitsPerSample = m_BitsPerSample->GetSelection();
		}
		if (m_Compress->GetSelection() == m_LastCompress)
		{
			SetEmpty(m_Compress);
			m_LastCompress = m_Compress->GetSelection();
		}
		if (m_Channels->GetSelection() == m_LastChannels)
		{
			SetEmpty(m_Channels);
			m_LastChannels = m_Channels->GetSelection();
		}
		if (m_LoopLoad->GetSelection() == m_LastLoopLoad)
		{
			SetEmpty(m_LoopLoad);
			m_LastLoopLoad = m_LoopLoad->GetSelection();
		}
		if (m_AttackLoad->GetSelection() == m_LastAttackLoad)
		{
			SetEmpty(m_AttackLoad);
			m_LastAttackLoad = m_AttackLoad->GetSelection();
		}
		if (m_ReleaseLoad->GetSelection() == m_LastReleaseLoad)
		{
			SetEmpty(m_ReleaseLoad);
			m_LastReleaseLoad = m_ReleaseLoad->GetSelection();
		}
	}
	else
		m_Apply->Disable();

	for(unsigned i = 0; i < entries.size(); i++)
		if (m_Last && m_Tree->GetItemData(entries[i]) == m_Last)
			return;

	m_Last = 0;
	for(unsigned i = 0; i < entries.size() && !m_Last; i++)
		m_Last = (OrganTreeItemData*)m_Tree->GetItemData(entries[i]);


	m_Amplitude->Enable();
	m_AmplitudeSpin->Enable();
	m_Gain->Enable();
	m_GainSpin->Enable();
	m_Tuning->Enable();
	m_TuningSpin->Enable();
	m_Delay->Enable();
	m_DelaySpin->Enable();
	m_AudioGroup->Enable();
	m_BitsPerSample->Enable();
	m_Compress->Enable();
	m_Channels->Enable();
	m_LoopLoad->Enable();
	m_AttackLoad->Enable();
	m_ReleaseLoad->Enable();
	m_Default->Enable();
	m_Reset->Disable();
	
	float amplitude = m_Last->config->GetAmplitude();
	float gain = m_Last->config->GetGain();
	float tuning = m_Last->config->GetTuning();
	unsigned delay = m_Last->config->GetDelay();

	if (entries.size() == 1)
		m_Amplitude->ChangeValue(wxString::Format(wxT("%f"), amplitude));
	m_AmplitudeSpin->SetValue(amplitude);
	if (entries.size() == 1)
		m_Gain->ChangeValue(wxString::Format(wxT("%f"), gain));
	m_GainSpin->SetValue(gain);
	if (entries.size() == 1)
		m_Tuning->ChangeValue(wxString::Format(wxT("%f"), tuning));
	m_TuningSpin->SetValue(tuning);
	if (entries.size() == 1)
		m_Delay->ChangeValue(wxString::Format(wxT("%u"), delay));
	m_DelaySpin->SetValue(delay);
	if (entries.size() == 1)
	{
		m_AudioGroup->SetValue(m_Last->config->GetAudioGroup());

		int bits_per_sample = m_Last->config->GetBitsPerSample();
		if (bits_per_sample == -1)
			bits_per_sample = 0;
		else
			bits_per_sample = (bits_per_sample - 4) / 4;

		RemoveEmpty(m_BitsPerSample);
		RemoveEmpty(m_Compress);
		RemoveEmpty(m_Channels);
		RemoveEmpty(m_LoopLoad);
		RemoveEmpty(m_AttackLoad);
		RemoveEmpty(m_ReleaseLoad);

		m_BitsPerSample->SetSelection(bits_per_sample);
		m_Compress->SetSelection(m_Last->config->GetCompress() + 1);
		m_Channels->SetSelection(m_Last->config->GetChannels() + 1);
		m_LoopLoad->SetSelection(m_Last->config->GetLoopLoad() + 1);
		m_AttackLoad->SetSelection(m_Last->config->GetAttackLoad() + 1);
		m_ReleaseLoad->SetSelection(m_Last->config->GetReleaseLoad() + 1);

		m_LastAudioGroup = m_AudioGroup->GetValue();
		m_LastBitsPerSample = m_BitsPerSample->GetSelection();
		m_LastCompress = m_Compress->GetSelection();
		m_LastChannels = m_Channels->GetSelection();
		m_LastLoopLoad = m_LoopLoad->GetSelection();
		m_LastAttackLoad = m_AttackLoad->GetSelection();
		m_LastReleaseLoad = m_ReleaseLoad->GetSelection();
	}
}

void OrganDialog::OnAmplitudeSpinChanged(wxSpinEvent& e)
{
	m_Amplitude->ChangeValue(wxString::Format(wxT("%f"), (float)m_AmplitudeSpin->GetValue()));
	m_Amplitude->MarkDirty();
	Modified();
}

void OrganDialog::OnAmplitudeChanged(wxCommandEvent &e)
{
	double amp;
	if (m_Amplitude->GetValue().ToDouble(&amp))
		m_AmplitudeSpin->SetValue(amp);
	Modified();
}

void OrganDialog::OnGainSpinChanged(wxSpinEvent& e)
{
	m_Gain->ChangeValue(wxString::Format(wxT("%f"), (float)m_GainSpin->GetValue()));
	m_Gain->MarkDirty();
	Modified();
}

void OrganDialog::OnGainChanged(wxCommandEvent &e)
{
	double gain;
	if (m_Gain->GetValue().ToDouble(&gain))
		m_GainSpin->SetValue(gain);
	Modified();
}

void OrganDialog::OnTuningSpinChanged(wxSpinEvent& e)
{
	m_Tuning->ChangeValue(wxString::Format(wxT("%f"), (float)m_TuningSpin->GetValue()));
	m_Tuning->MarkDirty();
	Modified();
}

void OrganDialog::OnTuningChanged(wxCommandEvent &e)
{
	double tuning;
	if (m_Tuning->GetValue().ToDouble(&tuning))
		m_TuningSpin->SetValue(tuning);
	Modified();
}

void OrganDialog::OnDelaySpinChanged(wxSpinEvent& e)
{
	m_Delay->ChangeValue(wxString::Format(wxT("%u"), (unsigned)m_DelaySpin->GetValue()));
	m_Delay->MarkDirty();
	Modified();
}

void OrganDialog::OnDelayChanged(wxCommandEvent &e)
{
	long delay;
	if (m_Delay->GetValue().ToLong(&delay))
		m_DelaySpin->SetValue(delay);
	Modified();
}

void OrganDialog::OnAudioGroupChanged(wxCommandEvent &e)
{
	Modified();
}

void OrganDialog::OnBitsPerSampleChanged(wxCommandEvent &e)
{
	RemoveEmpty(m_BitsPerSample);
	Modified();
}

void OrganDialog::OnCompressChanged(wxCommandEvent &e)
{
	RemoveEmpty(m_Compress);
	Modified();
}

void OrganDialog::OnChannelsChanged(wxCommandEvent &e)
{
	RemoveEmpty(m_Channels);
	Modified();
}

void OrganDialog::OnLoopLoadChanged(wxCommandEvent &e)
{
	RemoveEmpty(m_LoopLoad);
	Modified();
}

void OrganDialog::OnAttackLoadChanged(wxCommandEvent &e)
{
	RemoveEmpty(m_AttackLoad);
	Modified();
}

void OrganDialog::OnReleaseLoadChanged(wxCommandEvent &e)
{
	RemoveEmpty(m_ReleaseLoad);
	Modified();
}

bool OrganDialog::Changed()
{
	bool changed = false;
	if (m_Amplitude->IsModified())
		changed = true;
	if (m_Gain->IsModified())
		changed = true;
	if (m_Tuning->IsModified())
		changed = true;
	if (m_Delay->IsModified())
		changed = true;
	if (m_AudioGroup->GetValue() != m_LastAudioGroup)
		changed = true;
	if (m_BitsPerSample->GetSelection() != m_LastBitsPerSample)
		changed = true;
	if (m_Compress->GetSelection() != m_LastCompress)
		changed = true;
	if (m_Channels->GetSelection() != m_LastChannels)
		changed = true;
	if (m_LoopLoad->GetSelection() != m_LastLoopLoad)
		changed = true;
	if (m_AttackLoad->GetSelection() != m_LastAttackLoad)
		changed = true;
	if (m_ReleaseLoad->GetSelection() != m_LastReleaseLoad)
		changed = true;

	return changed;
}

void OrganDialog::Modified()
{
	if (m_Reset)
		m_Reset->Enable();
	if (m_Apply)
		m_Apply->Enable();
}

void OrganDialog::FillTree()
{
	wxTreeItemId id_root = m_Tree->AddRoot(m_organfile->GetChurchName(), -1, -1, new OrganTreeItemData(m_organfile->GetPipeConfig()));
	for (unsigned j = 0; j < m_organfile->GetWindchestGroupCount(); j++)
	{
		GOrgueWindchest* windchest = m_organfile->GetWindchest(j);
		wxTreeItemId id_windchest = m_Tree->AppendItem(id_root, windchest->GetName());
		for(unsigned i = 0; i < windchest->GetRankCount(); i++)
		{
			GOrgueRank* rank = windchest->GetRank(i);
			wxTreeItemId id_rank = m_Tree->AppendItem(id_windchest, rank->GetName(), -1, -1, new OrganTreeItemData(rank->GetPipeConfig()));
			for(unsigned k = 0; k < rank->GetPipeCount(); k++)
			{
				GOrguePipe* pipe = rank->GetPipe(k);
				if (pipe->IsReference())
					continue;
				m_Tree->AppendItem(id_rank, wxString::Format(_("%d: %s"), pipe->GetMidiKeyNumber(), pipe->GetFilename().c_str()), 
						   -1, -1, new OrganTreeItemData(pipe->GetPipeConfig()));
			}
		}
		m_Tree->Expand(id_windchest);
	}
	m_Tree->Expand(id_root);
}

void OrganDialog::OnEventApply(wxCommandEvent &e)
{
	double amp, gain, tuning;
	long delay;

	wxArrayTreeItemIds entries;
	m_Tree->GetSelections(entries);

	if (!m_Amplitude->GetValue().ToDouble(&amp) &&
	    (m_Amplitude->IsModified() &&
	     (amp < 0 || amp > 1000)))
	{
		GOMessageBox(_("Amplitude is invalid"), _("Error"), wxOK | wxICON_ERROR, this);
		return;
	}

	if (!m_Gain->GetValue().ToDouble(&gain) &&
	    (m_Gain->IsModified() &&
	     (gain < -120 || gain > 40)))
	{
		GOMessageBox(_("Gain is invalid"), _("Error"), wxOK | wxICON_ERROR, this);
		return;
	}

	if (!m_Tuning->GetValue().ToDouble(&tuning) &&
	    (m_Tuning->IsModified() &&
	     (tuning < - 1200 || tuning > 1200)))
	{
		GOMessageBox(_("Tuning is invalid"), _("Error"), wxOK | wxICON_ERROR, this);
		return;
	}

	if (!m_Delay->GetValue().ToLong(&delay) &&
	    (m_Delay->IsModified() &&
	     (delay < 0 || delay > 10000)))
	{
		GOMessageBox(_("Tracker delay is invalid"), _("Error"), wxOK | wxICON_ERROR, this);
		return;
	}

	for(unsigned i = 0; i < entries.size(); i++)
	{
		OrganTreeItemData* e = (OrganTreeItemData*)m_Tree->GetItemData(entries[i]);
		if (!e)
			continue;
		if (m_Amplitude->IsModified())
			e->config->SetAmplitude(amp);
		if (m_Gain->IsModified())
			e->config->SetGain(gain);
		if (m_Tuning->IsModified())
			e->config->SetTuning(tuning);
		if (m_Delay->IsModified())
			e->config->SetDelay(delay);
		if (m_AudioGroup->GetValue() != m_LastAudioGroup)
			e->config->SetAudioGroup(m_AudioGroup->GetValue().Trim());
		if (m_BitsPerSample->GetSelection() != m_LastBitsPerSample)
			e->config->SetBitsPerSample(m_BitsPerSample->GetSelection() == 0 ? -1 : m_BitsPerSample->GetSelection() * 4 + 4);
		if (m_Compress->GetSelection() != m_LastCompress)
			e->config->SetCompress(m_Compress->GetSelection() - 1);
		if (m_Channels->GetSelection() != m_LastChannels)
			e->config->SetChannels(m_Channels->GetSelection() - 1);
		if (m_LoopLoad->GetSelection() != m_LastLoopLoad)
			e->config->SetLoopLoad(m_LoopLoad->GetSelection() - 1);
		if (m_AttackLoad->GetSelection() != m_LastAttackLoad)
			e->config->SetAttackLoad(m_AttackLoad->GetSelection() - 1);
		if (m_ReleaseLoad->GetSelection() != m_LastReleaseLoad)
			e->config->SetReleaseLoad(m_ReleaseLoad->GetSelection() - 1);
	}

	m_Reset->Disable();
	m_Apply->Disable();
	if (m_Amplitude->IsModified())
		m_Amplitude->ChangeValue(wxString::Format(wxT("%f"), amp));
	if (m_Gain->IsModified())
		m_Gain->ChangeValue(wxString::Format(wxT("%f"), gain));
	if (m_Tuning->IsModified())
		m_Tuning->ChangeValue(wxString::Format(wxT("%f"), tuning));
	if (m_Delay->IsModified())
		m_Delay->ChangeValue(wxString::Format(wxT("%u"), delay));
	m_LastAudioGroup = m_AudioGroup->GetValue();
	m_LastBitsPerSample = m_BitsPerSample->GetSelection();
	m_LastCompress = m_Compress->GetSelection();
	m_LastChannels = m_Channels->GetSelection();
	m_LastLoopLoad = m_LoopLoad->GetSelection();
	m_LastAttackLoad = m_AttackLoad->GetSelection();
	m_LastReleaseLoad = m_ReleaseLoad->GetSelection();
}

void OrganDialog::OnEventReset(wxCommandEvent &e)
{
	m_Last = NULL;
	Load();
}

void OrganDialog::OnEventDefault(wxCommandEvent &e)
{
	wxArrayTreeItemIds entries;
	m_Tree->GetSelections(entries);

	for(unsigned i = 0; i < entries.size(); i++)
	{
		OrganTreeItemData* e = (OrganTreeItemData*)m_Tree->GetItemData(entries[i]);
		if (!e)
			continue;
		e->config->SetAmplitude(e->config->GetDefaultAmplitude());
		e->config->SetGain(e->config->GetDefaultGain());
		e->config->SetTuning(e->config->GetDefaultTuning());
		e->config->SetDelay(e->config->GetDefaultDelay());
		e->config->SetAudioGroup(wxEmptyString);
		e->config->SetBitsPerSample(-1);
		e->config->SetCompress(-1);
		e->config->SetChannels(-1);
		e->config->SetLoopLoad(-1);
		e->config->SetAttackLoad(-1);
		e->config->SetReleaseLoad(-1);
	}

	m_Last = NULL;
	Load();
}


void OrganDialog::OnTreeChanging(wxTreeEvent& e)
{
	if (Changed())
	{
		GOMessageBox(_("Please apply changes first"), _("Error"), wxOK | wxICON_ERROR, this);
		e.Veto();
	}
}

void OrganDialog::OnTreeChanged(wxTreeEvent& e)
{
	wxArrayTreeItemIds entries;
	if (m_LoadChangeCnt)
		return;
	m_LoadChangeCnt++;
	do
	{
		bool rescan = false;
		m_Tree->GetSelections(entries);
		for(unsigned i = 0; i < entries.size(); i++)
		{
			OrganTreeItemData* e = (OrganTreeItemData*)m_Tree->GetItemData(entries[i]);
			if (!e)
			{
				wxTreeItemIdValue it;
				wxTreeItemId child = m_Tree->GetFirstChild(entries[i], it);
				while(child.IsOk())
				{
					bool found = false;
					for(unsigned j = 0; j < entries.size(); j++)
						if (entries[j] == child)
							found = true;
					if (!found)
							m_Tree->SelectItem(child, true);
					child = m_Tree->GetNextChild(entries[i], it);
				}
				rescan = true;
				m_Tree->SelectItem(entries[i], false);
			}
		}
		if (rescan)
			continue;
	}
	while(false);
	m_LoadChangeCnt--;

	wxCommandEvent event(wxEVT_TREE_UPDATED, ID_EVENT_TREE);
	GetEventHandler()->AddPendingEvent(event);
}

void OrganDialog::OnTreeUpdated(wxCommandEvent& e)
{
	Load();
}

void OrganDialog::OnOK(wxCommandEvent& event)
{
	if (Changed())
	{
		GOMessageBox(_("Please apply changes first"), _("Error"), wxOK | wxICON_ERROR, this);
		return;
	}
	m_organfile->SetIgnorePitch(m_IgnorePitch->GetValue());
	m_organfile->SetTemperament(m_organfile->GetTemperament());
	m_organfile->Modified();
	Destroy();
}

void OrganDialog::OnCancel(wxCommandEvent& event)
{
	Destroy();
}

void OrganDialog::UpdateAudioGroup(std::vector<wxString> audio_group, unsigned& pos, wxTreeItemId item)
{
	OrganTreeItemData* e = (OrganTreeItemData*)m_Tree->GetItemData(item);
	if (e)
	{
		e->config->SetAudioGroup(audio_group[pos]);
		pos++;
		if (pos >= audio_group.size())
			pos = 0;
	}

	wxTreeItemIdValue it;
	wxTreeItemId child = m_Tree->GetFirstChild(item, it);
	while(child.IsOk())
	{
		UpdateAudioGroup(audio_group, pos, child);
		child = m_Tree->GetNextChild(item, it);
	}
}


void OrganDialog::OnAudioGroupAssitant(wxCommandEvent &e)
{
	if (Changed())
	{
		GOMessageBox(_("Please apply changes first"), _("Error"), wxOK | wxICON_ERROR, this);
		return;
	}
	wxArrayString strs;
	std::vector<wxString> group_list = m_organfile->GetSettings().GetAudioGroups();
	for(unsigned i = 0; i < group_list.size(); i++)
		strs.Add(group_list[i]);
	
	wxMultiChoiceDialog dlg(this, _("Select audio groups to distribute:"), _("Organ dialog"), strs);
	m_ModalDialog = &dlg;
	if (dlg.ShowModal() != wxID_OK)
	{
		CloseModal();
		return;
	}
	wxArrayInt sel = dlg.GetSelections();
	CloseModal();
	if (sel.Count() == 0)
	{
		GOMessageBox(_("No audio group selected"), _("Error"), wxOK | wxICON_ERROR, this);
		return;
	}
	group_list.clear();
	for(unsigned i = 0; i < sel.Count(); i++)
		group_list.push_back(strs[sel[i]]);

	wxArrayTreeItemIds entries;
	m_Tree->GetSelections(entries);
	unsigned pos = 0;
	for(unsigned i = 0; i < entries.size(); i++)
		UpdateAudioGroup(group_list, pos, entries[i]);
	m_Last = NULL;
	Load();
}
