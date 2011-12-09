/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2011 GrandOrgue contributors (see AUTHORS)
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
#include "GOrgueManual.h"
#include "GOrgueStop.h"
#include "GOrguePipe.h"

class OrganTreeItemData : public wxTreeItemData
{
public:
	OrganTreeItemData(GOrgueStop* s)
	{
		config = &s->GetPipeConfig();
		stop = s;
		pipe = NULL;
		organfile = NULL;
	}

	OrganTreeItemData(GOrguePipe* p)
	{
		config = &p->GetPipeConfig();
		stop = NULL;
		pipe = p;
		organfile = NULL;
	}
		
	OrganTreeItemData(GrandOrgueFile* o)
	{
		config = &o->GetPipeConfig();
		stop = NULL;
		pipe = NULL;
		organfile = o;
	}

	GOrguePipeConfig* config;
	GOrgueStop* stop;
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
END_EVENT_TABLE()

OrganDialog::OrganDialog (wxWindow* parent, GrandOrgueFile* organfile) :
	wxDialog(parent, wxID_ANY, _("Organ settings"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE),
	m_organfile(organfile),
	m_Last(NULL)
{
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);
	topSizer->Add(mainSizer, 0, wxALL, 6);

	m_Tree = new wxTreeCtrl(this, ID_EVENT_TREE, wxDefaultPosition, wxSize(300, 400), wxTR_HAS_BUTTONS | wxTR_MULTIPLE | wxTR_EXTENDED);
	wxBoxSizer* Sizer1 = new wxBoxSizer(wxVERTICAL);
	Sizer1->Add(m_Tree, wxALIGN_TOP | wxEXPAND);
	mainSizer->Add(Sizer1, wxALIGN_LEFT | wxEXPAND);
	mainSizer->AddSpacer(5);

	wxBoxSizer* box1 = new wxStaticBoxSizer(wxVERTICAL, this, _("Settings"));

	wxFlexGridSizer* grid = new wxFlexGridSizer(4, 3, 5, 5);
	box1->Add(grid, 0, wxEXPAND | wxALL, 5);

	grid->Add(new wxStaticText(this, wxID_ANY, _("Amplitude:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM, 5);
	m_Amplitude = new wxTextCtrl(this, ID_EVENT_AMPLITUDE, wxEmptyString);
	m_AmplitudeSpin = new wxSpinButton(this, ID_EVENT_AMPLITUDE_SPIN); 
	grid->Add(m_Amplitude);
	grid->Add(m_AmplitudeSpin);
	m_AmplitudeSpin->SetRange(0, 1000);

	grid->Add(new wxStaticText(this, wxID_ANY, _("Tuning (Cent):")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM, 5);
	m_Tuning = new wxTextCtrl(this, ID_EVENT_TUNING, wxEmptyString);
	m_TuningSpin = new wxSpinButton(this, ID_EVENT_TUNING_SPIN); 
	grid->Add(m_Tuning);
	grid->Add(m_TuningSpin);
	m_TuningSpin->SetRange(-1200, 1200);

	wxBoxSizer* buttons = new wxBoxSizer(wxHORIZONTAL);
	m_Apply = new wxButton(this, ID_EVENT_APPLY, _("Apply"));
	m_Reset = new wxButton(this, ID_EVENT_RESET, _("Reset"));
	m_Default = new wxButton(this, ID_EVENT_DEFAULT, _("Default"));
	buttons->Add(m_Default);
	buttons->Add(m_Reset);
	buttons->Add(m_Apply);
	box1->Add(buttons);

	mainSizer->Add(box1, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxEXPAND);
	
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

void OrganDialog::Load()
{
	wxArrayTreeItemIds entries;
	m_Tree->GetSelections(entries);
	if (entries.size() == 0)
	{
		m_Last = NULL;
		m_Amplitude->ChangeValue(wxEmptyString);
		m_Amplitude->Disable();
		m_AmplitudeSpin->Disable();
		m_Tuning->ChangeValue(wxEmptyString);
		m_Tuning->Disable();
		m_TuningSpin->Disable();
		m_Apply->Disable();
		m_Reset->Disable();
		m_Default->Disable();
		return;
	}

	if (entries.size() > 1)
		m_Apply->Enable();
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
	m_Default->Enable();
	m_Reset->Disable();
	
	float amplitude;
	float tuning;

	amplitude = m_Last->config->GetAmplitude();
	tuning = m_Last->config->GetTuning();

	m_Amplitude->ChangeValue(wxString::Format(wxT("%f"), amplitude));
	m_AmplitudeSpin->SetValue(amplitude);
	m_Tuning->ChangeValue(wxString::Format(wxT("%f"), tuning));
	m_TuningSpin->SetValue(tuning);
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

bool OrganDialog::Changed()
{
	bool changed = false;
	if (m_Amplitude->IsModified())
		changed = true;
	if (m_Tuning->IsModified())
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
	for (unsigned j = m_organfile->GetFirstManualIndex(); j <= m_organfile->GetManualAndPedalCount(); j++)
	{
		GOrgueManual* manual = m_organfile->GetManual(j);
		wxTreeItemId id_manual = m_Tree->AppendItem(id_root, manual->GetName());
		for(unsigned i = 0; i < manual->GetStopCount(); i++)
		{
			GOrgueStop* stop = manual->GetStop(i);
			wxTreeItemId id_stop = m_Tree->AppendItem(id_manual, stop->GetName(), -1, -1, new OrganTreeItemData(stop));
			for(unsigned k = 0; k < stop->GetPipeCount(); k++)
			{
				GOrguePipe* pipe = stop->GetPipe(k);
				if (pipe->IsReference())
					continue;
				wxTreeItemId id_pipe = m_Tree->AppendItem(id_stop, pipe->GetFilename(), -1, -1, new OrganTreeItemData(pipe));
			}
		}
		m_Tree->Expand(id_manual);
	}
	m_Tree->Expand(id_root);
}

void OrganDialog::OnEventApply(wxCommandEvent &e)
{
	float amp = wxAtof(m_Amplitude->GetValue());
	float tuning = wxAtof(m_Tuning->GetValue());

	wxArrayTreeItemIds entries;
	m_Tree->GetSelections(entries);

	if (amp < 0 || amp > 1000)
	{
		wxMessageBox(_("Amplitude is invalid"), _("Error"), wxOK | wxICON_ERROR, NULL);
		return;
	}

	if (amp < - 1200 || amp > 1200)
	{
		wxMessageBox(_("Tuning is invalid"), _("Error"), wxOK | wxICON_ERROR, NULL);
		return;
	}

	for(unsigned i = 0; i < entries.size(); i++)
	{
		OrganTreeItemData* e = (OrganTreeItemData*)m_Tree->GetItemData(entries[i]);
		e->config->SetAmplitude(amp);
		e->config->SetTuning(tuning);
	}

	m_Last = NULL;
	Load();
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
		e->config->SetAmplitude(e->config->GetDefaultAmplitude());
		e->config->SetTuning(e->config->GetDefaultTuning());
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

