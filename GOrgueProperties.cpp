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

#include "GrandOrgue.h"
#include "GOrgueProperties.h"
#include <wx/statline.h>
#include <wx/html/helpctrl.h>
#include "GrandOrgueFile.h"

extern GrandOrgueFile* organfile;

BEGIN_EVENT_TABLE(wxStaticLink, wxStaticText)
    EVT_LEFT_UP(wxStaticLink::OnClick)
END_EVENT_TABLE()

wxStaticLink::wxStaticLink(wxWindow* parent, const wxString& label, const wxString& url) : wxStaticText(parent, wxID_ANY, label)
{
    SetForegroundColour(*wxBLUE);
    wxFont font = GetFont();
    font.SetWeight(wxFONTWEIGHT_BOLD);
    font.SetUnderlined(true);
    SetFont(font);
    SetCursor(wxCursor(wxCURSOR_HAND));
    SetToolTip(url);
    m_url = url;
}

void wxStaticLink::OnClick(wxMouseEvent& event)
{
    ::wxLaunchDefaultBrowser(m_url);
}

wxStaticText* GOrguePropertiesText(wxWindow* parent, int title, wxString text)
{
	wxStaticText* item = new wxStaticText(parent, wxID_ANY, text);

	if (title <= 0)
	{
		wxFont font = item->GetFont();
		font.SetWeight(wxFONTWEIGHT_BOLD);
		item->SetFont(font);
	}
	if (title < 0)
        item->SetForegroundColour(*wxRED);
	if (title)
		item->Wrap(abs(title));
	return item;
}

bool GOrguePropertiesTest(const wxString& what)
{
	return !what.IsEmpty() && what.CmpNoCase("fictional") && what.CmpNoCase("unknown") && what.CmpNoCase("none") && what.CmpNoCase("N/A");
}

GOrgueProperties::GOrgueProperties(wxWindow* win) : wxDialog(win, wxID_ANY, (wxString)_("Organ Properties"))
{
	wxASSERT(organfile);
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	sizer->Add(GOrguePropertiesText(this, 0, _("Title")), 0);
	sizer->Add(GOrguePropertiesText(this, 300, organfile->GetChurchName()), 0, wxLEFT, 10);
	if (GOrguePropertiesTest(organfile->GetChurchAddress()))
	{
		sizer->Add(GOrguePropertiesText(this, 0, _("Address")), 0, wxTOP, 5);
		sizer->Add(GOrguePropertiesText(this, 300, organfile->GetChurchAddress()), 0, wxLEFT, 10);
	}
	if (GOrguePropertiesTest(organfile->GetOrganBuilder()))
	{
		sizer->Add(GOrguePropertiesText(this, 0, _("Builder")), 0, wxTOP, 5);
		sizer->Add(GOrguePropertiesText(this, 300, organfile->GetOrganBuilder()), 0, wxLEFT, 10);
	}
	if (GOrguePropertiesTest(organfile->GetOrganBuildDate()))
	{
		sizer->Add(GOrguePropertiesText(this, 0,  _("Build Date")), 0, wxTOP, 5);
		sizer->Add(GOrguePropertiesText(this, 300, organfile->GetOrganBuildDate()), 0, wxLEFT, 10);
	}
	if (GOrguePropertiesTest(organfile->GetRecordingDetails()))
	{
		sizer->Add(GOrguePropertiesText(this, 0,  _("Recording Details")), 0, wxTOP, 5);
		sizer->Add(GOrguePropertiesText(this, 300, organfile->GetRecordingDetails()), 0, wxLEFT, 10);
	}
	if (GOrguePropertiesTest(organfile->GetOrganComments()))
	{
		sizer->Add(GOrguePropertiesText(this, 0,  _("Other Comments")), 0, wxTOP, 5);
		sizer->Add(GOrguePropertiesText(this, 300, organfile->GetOrganComments()), 0, wxLEFT, 10);
	}

	if (!organfile->GetInfoFilename().IsEmpty())
	    sizer->Add(new wxStaticLink(this, _("More Information"), organfile->GetInfoFilename()), 0, wxTOP | wxALIGN_CENTER_HORIZONTAL, 5);
	topSizer->Add(sizer, 0, wxALL, 10);
	topSizer->Add(new wxStaticLine(this), 0, wxEXPAND | wxLEFT | wxRIGHT, 5);
	topSizer->Add(CreateButtonSizer(wxOK), 0, wxALL | wxEXPAND, 10);

	SetSizer(topSizer);
	topSizer->Fit(this);
}

GOrgueProperties::~GOrgueProperties(void)
{
}

BEGIN_EVENT_TABLE(GOrgueRegister, wxDialog)
    EVT_TEXT(ID_LICENSEKEY, GOrgueRegister::OnKey)
    EVT_BUTTON(wxID_HELP, GOrgueRegister::OnHelp)
END_EVENT_TABLE()

void GOrgueRegister::OnKey(wxCommandEvent& event)
{
    int i, j;
    wxString str(event.GetString());
    for (i = 0, j = 0; i < (int)str.Length() && j < 200; i++)
        if ((str[i] >= '2' && str[i] <= '9') || (str[i] >= 'A' && str[i] <= 'Z'))
            j++;
	FindWindowById(wxID_OK,this)->Enable(!event.GetString().IsEmpty());
    if (j >= 200)
        FindWindowById(wxID_OK, this)->SetFocus();
    event.Skip();
}

void GOrgueRegister::OnHelp(wxCommandEvent& event)
{
    ::wxGetApp().m_help->Display(_("Register"));
}

GOrgueRegister::GOrgueRegister(wxWindow* parent) : wxDialog(parent, wxID_ANY, (wxString)_("Register"))
{
    wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

    sizer->Add(GOrguePropertiesText(this, 460, _("This program is free. You may use this version as you need. ")), 0, wxBOTTOM, 5);
    sizer->Add(new wxStaticLink(this, "http://www.grand-orgue.com", "http://www.grand-orgue.com"), 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 15);
    sizer->Add(GOrguePropertiesText(this, 460, _("Enjoy.")), 0, wxBOTTOM, 5);
    /*sizer->Add(m_key = new wxTextCtrl(this, ID_LICENSEKEY, wxEmptyString, wxDefaultPosition, wxSize(wxDefaultCoord, 130), wxTE_MULTILINE), 0, wxEXPAND | wxBOTTOM, 5);
    m_key->SetFont(wxSystemSettings::GetFont(wxSYS_ANSI_FIXED_FONT));
    sizer->Add(GOrguePropertiesText(this, -460, _("If you would like to continue evaluating, you would simply need to reload the current sample set or load a different one.")), 0);
    */
	topSizer->Add(sizer, 0, wxALL, 10);
	topSizer->Add(new wxStaticLine(this), 0, wxEXPAND | wxLEFT | wxRIGHT, 5);
	topSizer->Add(CreateButtonSizer(wxOK | wxCANCEL | wxHELP), 0, wxALL | wxEXPAND, 10);

	SetSizer(topSizer);
	topSizer->Fit(this);

	FindWindowById(wxID_OK,this)->Enable(false);

	m_key->SetFocus();
}

GOrgueRegister::~GOrgueRegister(void)
{
}
