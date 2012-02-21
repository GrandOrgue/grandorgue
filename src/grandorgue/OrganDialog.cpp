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

#include <wx/statline.h>
#include <wx/spinbutt.h>
#include <wx/treectrl.h>
#include "OrganDialog.h"
#include "GrandOrgueFile.h"
#include "GOrgueRank.h"
#include "GOrguePipe.h"
#include "GOrgueWindchest.h"

class OrganTreeItemData : public wxTreeItemData
{
public:
	OrganTreeItemData(GOrgueRank* r)
	{
		config = &r->GetPipeConfig();
		rank = r;
		pipe = NULL;
		organfile = NULL;
	}

	OrganTreeItemData(GOrguePipe* p)
	{
		config = &p->GetPipeConfig();
		rank = NULL;
		pipe = p;
		organfile = NULL;
	}
		
	OrganTreeItemData(GrandOrgueFile* o)
	{
		config = &o->GetPipeConfig();
		rank = NULL;
		pipe = NULL;
		organfile = o;
	}

	GOrguePipeConfig* config;
	GOrgueRank* rank;
	GOrguePipe* pipe;
	GrandOrgueFile* organfile;
};

IMPLEMENT_CLASS(OrganDialog, wxDialog)

BEGIN_EVENT_TABLE(OrganDialog, wxDialog)
	EVT_BUTTON(ID_EVENT_APPLY, OrganDialog::OnEventApply)
	EVT_BUTTON(ID_EVENT_RESET, OrganDialog::OnEventReset)
	EVT_BUTTON(ID_EVENT_DEFAULT, OrganDialog::OnEventDefault)
	EVT_BUTTON(wxID_OK, OrganDialog::OnEventOK)
	EVT_TREE_SEL_CHANGING(ID_EVENT_TREE, OrganDialog::OnTreeChanging)
	EVT_TREE_SEL_CHANGED(ID_EVENT_TREE, OrganDialog::OnTreeChanged)
	EVT_TEXT(ID_EVENT_AMPLITUDE, OrganDialog::OnAmplitudeChanged)
	EVT_SPIN(ID_EVENT_AMPLITUDE_SPIN, OrganDialog::OnAmplitudeSpinChanged)
	EVT_TEXT(ID_EVENT_TUNING, OrganDialog::OnTuningChanged)
	EVT_SPIN(ID_EVENT_TUNING_SPIN, OrganDialog::OnTuningSpinChanged)
	EVT_BUTTON(wxID_OK, OrganDialog::OnOK)
	EVT_CHOICE(ID_EVENT_BITS_PER_SAMPLE, OrganDialog::OnBitsPerSampleChanged)
	EVT_CHOICE(ID_EVENT_COMPRESS, OrganDialog::OnCompressChanged)
	EVT_CHOICE(ID_EVENT_CHANNELS, OrganDialog::OnChannelsChanged)
	EVT_CHOICE(ID_EVENT_LOOP_LOAD, OrganDialog::OnLoopLoadChanged)
	EVT_CHOICE(ID_EVENT_ATTACK_LOAD, OrganDialog::OnAttackLoadChanged)
	EVT_CHOICE(ID_EVENT_RELEASE_LOAD, OrganDialog::OnReleaseLoadChanged)
END_EVENT_TABLE()

OrganDialog::OrganDialog (wxWindow* parent, GrandOrgueFile* organfile) :
	wxDialog(parent, wxID_ANY, _("Organ settings"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE),
	m_organfile(organfile),
	m_Last(NULL)
{
	wxArrayString choices;

	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);

	topSizer->Add(mainSizer, 0, wxALL, 6);

	m_Tree = new wxTreeCtrl(this, ID_EVENT_TREE, wxDefaultPosition, wxSize(300, 400), wxTR_HAS_BUTTONS | wxTR_MULTIPLE);
	wxBoxSizer* Sizer1 = new wxBoxSizer(wxVERTICAL);
	Sizer1->Add(m_Tree, wxALIGN_TOP | wxEXPAND);
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

	grid->Add(new wxStaticText(this, wxID_ANY, _("Tuning (Cent):")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM, 5);
	box2 = new wxBoxSizer(wxHORIZONTAL);
	m_Tuning = new wxTextCtrl(this, ID_EVENT_TUNING, wxEmptyString);
	m_TuningSpin = new wxSpinButton(this, ID_EVENT_TUNING_SPIN); 
	box2->Add(m_Tuning);
	box2->Add(m_TuningSpin);
	grid->Add(box2);
	m_TuningSpin->SetRange(-1200, 1200);

	box1 = new wxStaticBoxSizer(wxVERTICAL, this, _("Sample Loading"));
	grid = new wxFlexGridSizer(4, 2, 5, 5);
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
		OrganTreeItemData* e = (OrganTreeItemData*)m_Tree->GetItemData(entries[i]);
		if (!e)
		{
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
		m_Tuning->ChangeValue(wxEmptyString);
		m_Tuning->Disable();
		m_TuningSpin->Disable();
		m_BitsPerSample->Disable();
		m_Compress->Disable();
		m_Channels->Disable();
		m_LoopLoad->Disable();
		m_AttackLoad->Disable();
		m_ReleaseLoad->Disable();
		m_Apply->Disable();
		m_Reset->Disable();
		m_Default->Disable();
		return;
	}

	if (entries.size() > 1)
	{
		if (!m_Amplitude->IsModified())
			m_Amplitude->ChangeValue(wxEmptyString);
		if (!m_Tuning->IsModified())
			m_Tuning->ChangeValue(wxEmptyString);
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
	m_Tuning->Enable();
	m_TuningSpin->Enable();
	m_BitsPerSample->Enable();
	m_Compress->Enable();
	m_Channels->Enable();
	m_LoopLoad->Enable();
	m_AttackLoad->Enable();
	m_ReleaseLoad->Enable();
	m_Default->Enable();
	m_Reset->Disable();
	
	float amplitude;
	float tuning;

	amplitude = m_Last->config->GetAmplitude();
	tuning = m_Last->config->GetTuning();

	if (entries.size() == 1)
		m_Amplitude->ChangeValue(wxString::Format(wxT("%f"), amplitude));
	m_AmplitudeSpin->SetValue(amplitude);
	if (entries.size() == 1)
		m_Tuning->ChangeValue(wxString::Format(wxT("%f"), tuning));
	m_TuningSpin->SetValue(tuning);
	if (entries.size() == 1)
	{
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
	float amp = wxAtof(m_Amplitude->GetValue());
	m_AmplitudeSpin->SetValue(amp);
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
	float tuning = wxAtof(m_Tuning->GetValue());
	m_TuningSpin->SetValue(tuning);
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
	if (m_Tuning->IsModified())
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
	m_Reset->Enable();
	m_Apply->Enable();
}

void OrganDialog::FillTree()
{
	wxTreeItemId id_root = m_Tree->AddRoot(m_organfile->GetChurchName(), -1, -1, new OrganTreeItemData(m_organfile));
	for (unsigned j = 0; j < m_organfile->GetWinchestGroupCount(); j++)
	{
		GOrgueWindchest* windchest = m_organfile->GetWindchest(j);
		wxTreeItemId id_windchest = m_Tree->AppendItem(id_root, windchest->GetName());
		for(unsigned i = 0; i < windchest->GetRankCount(); i++)
		{
			GOrgueRank* rank = windchest->GetRank(i);
			wxTreeItemId id_rank = m_Tree->AppendItem(id_windchest, rank->GetName(), -1, -1, new OrganTreeItemData(rank));
			for(unsigned k = 0; k < rank->GetPipeCount(); k++)
			{
				GOrguePipe* pipe = rank->GetPipe(k);
				if (pipe->IsReference())
					continue;
				m_Tree->AppendItem(id_rank, pipe->GetFilename(), -1, -1, new OrganTreeItemData(pipe));
			}
		}
		m_Tree->Expand(id_windchest);
	}
	m_Tree->Expand(id_root);
}

void OrganDialog::OnEventApply(wxCommandEvent &e)
{
	float amp = wxAtof(m_Amplitude->GetValue());
	float tuning = wxAtof(m_Tuning->GetValue());

	wxArrayTreeItemIds entries;
	m_Tree->GetSelections(entries);

	if (m_Amplitude->IsModified() &&
	    (amp < 0 || amp > 1000))
	{
		wxMessageBox(_("Amplitude is invalid"), _("Error"), wxOK | wxICON_ERROR, NULL);
		return;
	}

	if (m_Tuning->IsModified() &&
	    (tuning < - 1200 || tuning > 1200))
	{
		wxMessageBox(_("Tuning is invalid"), _("Error"), wxOK | wxICON_ERROR, NULL);
		return;
	}

	for(unsigned i = 0; i < entries.size(); i++)
	{
		OrganTreeItemData* e = (OrganTreeItemData*)m_Tree->GetItemData(entries[i]);
		if (!e)
			continue;
		if (m_Amplitude->IsModified())
			e->config->SetAmplitude(amp);
		if (m_Tuning->IsModified())
			e->config->SetTuning(tuning);
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
	if (m_Tuning->IsModified())
		m_Tuning->ChangeValue(wxString::Format(wxT("%f"), tuning));
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
		e->config->SetTuning(e->config->GetDefaultTuning());
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
	if (!m_Tree->GetItemData(e.GetItem()))
	{
		e.Veto();
		return;
	}
	if (Changed())
	{
		wxMessageBox(_("Please apply changes first"), _("Error"), wxOK | wxICON_ERROR, NULL);
		e.Veto();
	}
}

void OrganDialog::OnTreeChanged(wxTreeEvent& e)
{
	Load();
}

void OrganDialog::OnEventOK(wxCommandEvent &e)
{
	if (Changed())
		wxMessageBox(_("Please apply changes first"), _("Error"), wxOK | wxICON_ERROR, NULL);
	else
		e.Skip();
}

void OrganDialog::OnOK(wxCommandEvent& event)
{
	m_organfile->SetIgnorePitch(m_IgnorePitch->GetValue());
	m_organfile->SetTemperament(m_organfile->GetTemperament());
	m_organfile->Modified();
	event.Skip();
}
