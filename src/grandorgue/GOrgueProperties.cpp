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

#include "GOrgueProperties.h"

#include "GrandOrgueFile.h"
#include <wx/sizer.h>
#include <wx/statline.h>

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
	return !what.IsEmpty() && what.CmpNoCase(wxT("fictional")) && what.CmpNoCase(wxT("unknown")) && what.CmpNoCase(wxT("none")) && what.CmpNoCase(wxT("N/A"));
}

GOrgueProperties::GOrgueProperties(GrandOrgueFile* organfile, wxWindow* win) :
	wxDialog(win, wxID_ANY, (wxString)_("Organ Properties")),
	m_organfile(organfile)
{
	wxASSERT(organfile);
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	sizer->Add(GOrguePropertiesText(this, 0, _("Title")), 0);
	sizer->Add(GOrguePropertiesText(this, 300, m_organfile->GetChurchName()), 0, wxLEFT, 10);
	if (GOrguePropertiesTest(m_organfile->GetChurchAddress()))
	{
		sizer->Add(GOrguePropertiesText(this, 0, _("Address")), 0, wxTOP, 5);
		sizer->Add(GOrguePropertiesText(this, 300, m_organfile->GetChurchAddress()), 0, wxLEFT, 10);
	}
	if (GOrguePropertiesTest(m_organfile->GetOrganBuilder()))
	{
		sizer->Add(GOrguePropertiesText(this, 0, _("Builder")), 0, wxTOP, 5);
		sizer->Add(GOrguePropertiesText(this, 300, m_organfile->GetOrganBuilder()), 0, wxLEFT, 10);
	}
	if (GOrguePropertiesTest(m_organfile->GetOrganBuildDate()))
	{
		sizer->Add(GOrguePropertiesText(this, 0,  _("Build Date")), 0, wxTOP, 5);
		sizer->Add(GOrguePropertiesText(this, 300, m_organfile->GetOrganBuildDate()), 0, wxLEFT, 10);
	}
	if (GOrguePropertiesTest(m_organfile->GetRecordingDetails()))
	{
		sizer->Add(GOrguePropertiesText(this, 0,  _("Recording Details")), 0, wxTOP, 5);
		sizer->Add(GOrguePropertiesText(this, 300, m_organfile->GetRecordingDetails()), 0, wxLEFT, 10);
	}
	if (GOrguePropertiesTest(m_organfile->GetOrganComments()))
	{
		sizer->Add(GOrguePropertiesText(this, 0,  _("Other Comments")), 0, wxTOP, 5);
		sizer->Add(GOrguePropertiesText(this, 300, m_organfile->GetOrganComments()), 0, wxLEFT, 10);
	}

	if (!m_organfile->GetInfoFilename().IsEmpty())
	    sizer->Add(new wxStaticLink(this, _("More Information"), m_organfile->GetInfoFilename()), 0, wxTOP | wxALIGN_CENTER_HORIZONTAL, 5);
	topSizer->Add(sizer, 0, wxALL, 10);
	topSizer->Add(new wxStaticLine(this), 0, wxEXPAND | wxLEFT | wxRIGHT, 5);
	topSizer->Add(CreateButtonSizer(wxOK), 0, wxALL | wxEXPAND, 10);

	sizer->Add(GOrguePropertiesText(this, 0,  _("Allocated sample memory")), 0, wxTOP, 5);
	float size, size1;
	size1 = m_organfile->GetMemoryPool().GetAllocSize() / (1024.0 * 1024.0);
	size = m_organfile->GetMemoryPool().GetMemoryLimit() / (1024.0 * 1024.0);
	if (m_organfile->GetMemoryPool().GetMemoryLimit() > 0)
		sizer->Add(GOrguePropertiesText(this, 0,  wxString::Format(_("%.3f MB of %.3f MB"), size1, size)), 0, wxTOP, 5);
	else
		sizer->Add(GOrguePropertiesText(this, 0,  wxString::Format(_("%.3f MB (not limited)"), size1)), 0, wxTOP, 5);

	sizer->Add(GOrguePropertiesText(this, 0,  _("Mapped memory of the cache")), 0, wxTOP, 5);
	size = m_organfile->GetMemoryPool().GetMappedSize() / (1024.0 * 1024.0);
	sizer->Add(GOrguePropertiesText(this, 0,  wxString::Format(_("%.3f MB"), size)), 0, wxTOP, 5);

	sizer->Add(GOrguePropertiesText(this, 0,  _("Memory pool size")), 0, wxTOP, 5);
	size1 = m_organfile->GetMemoryPool().GetPoolUsage() / (1024.0 * 1024.0);
	size = m_organfile->GetMemoryPool().GetPoolSize() / (1024.0 * 1024.0);
	sizer->Add(GOrguePropertiesText(this, 0,  wxString::Format(_("%.3f MB of %.3f MB"), size1, size)), 0, wxTOP, 5);

	sizer->Add(GOrguePropertiesText(this, 0,  _("ODF Path")), 0, wxTOP, 5);
	sizer->Add(GOrguePropertiesText(this, 300, m_organfile->GetODFFilename()), 0, wxLEFT, 10);

	SetSizer(topSizer);
	topSizer->Fit(this);
}

GOrgueProperties::~GOrgueProperties(void)
{
}
