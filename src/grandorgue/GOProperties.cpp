/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOProperties.h"

#include <wx/sizer.h>
#include <wx/statline.h>

#include "GOOrganController.h"

BEGIN_EVENT_TABLE(wxStaticLink, wxStaticText)
EVT_LEFT_UP(wxStaticLink::OnClick)
END_EVENT_TABLE()

wxStaticLink::wxStaticLink(
  wxWindow *parent, const wxString &label, const wxString &url)
  : wxStaticText(parent, wxID_ANY, label) {
  SetForegroundColour(*wxBLUE);
  wxFont font = GetFont();
  font.SetWeight(wxFONTWEIGHT_BOLD);
  font.SetUnderlined(true);
  SetFont(font);
  SetCursor(wxCursor(wxCURSOR_HAND));
  SetToolTip(url);
  m_url = url;
}

void wxStaticLink::OnClick(wxMouseEvent &event) {
  ::wxLaunchDefaultBrowser(m_url);
}

wxStaticText *GOPropertiesText(wxWindow *parent, int title, wxString text) {
  wxStaticText *item = new wxStaticText(parent, wxID_ANY, text);

  if (title <= 0) {
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

bool GOPropertiesTest(const wxString &what) {
  return !what.IsEmpty() && what.CmpNoCase(wxT("fictional"))
    && what.CmpNoCase(wxT("unknown")) && what.CmpNoCase(wxT("none"))
    && what.CmpNoCase(wxT("N/A"));
}

GOProperties::GOProperties(GOOrganController *organController, wxWindow *win)
  : wxDialog(win, wxID_ANY, (wxString)_("Organ Properties")),
    m_OrganController(organController) {
  wxASSERT(organController);
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

  sizer->Add(GOPropertiesText(this, 0, _("Title")), 0);
  sizer->Add(
    GOPropertiesText(this, 300, m_OrganController->GetChurchName()),
    0,
    wxLEFT,
    10);
  if (GOPropertiesTest(m_OrganController->GetChurchAddress())) {
    sizer->Add(GOPropertiesText(this, 0, _("Address")), 0, wxTOP, 5);
    sizer->Add(
      GOPropertiesText(this, 300, m_OrganController->GetChurchAddress()),
      0,
      wxLEFT,
      10);
  }
  if (GOPropertiesTest(m_OrganController->GetOrganBuilder())) {
    sizer->Add(GOPropertiesText(this, 0, _("Builder")), 0, wxTOP, 5);
    sizer->Add(
      GOPropertiesText(this, 300, m_OrganController->GetOrganBuilder()),
      0,
      wxLEFT,
      10);
  }
  if (GOPropertiesTest(m_OrganController->GetOrganBuildDate())) {
    sizer->Add(GOPropertiesText(this, 0, _("Build Date")), 0, wxTOP, 5);
    sizer->Add(
      GOPropertiesText(this, 300, m_OrganController->GetOrganBuildDate()),
      0,
      wxLEFT,
      10);
  }
  if (GOPropertiesTest(m_OrganController->GetRecordingDetails())) {
    sizer->Add(GOPropertiesText(this, 0, _("Recording Details")), 0, wxTOP, 5);
    sizer->Add(
      GOPropertiesText(this, 300, m_OrganController->GetRecordingDetails()),
      0,
      wxLEFT,
      10);
  }
  if (GOPropertiesTest(m_OrganController->GetOrganComments())) {
    sizer->Add(GOPropertiesText(this, 0, _("Other Comments")), 0, wxTOP, 5);
    sizer->Add(
      GOPropertiesText(this, 300, m_OrganController->GetOrganComments()),
      0,
      wxLEFT,
      10);
  }

  if (!m_OrganController->GetInfoFilename().IsEmpty())
    sizer->Add(
      new wxStaticLink(
        this, _("More Information"), m_OrganController->GetInfoFilename()),
      0,
      wxTOP | wxALIGN_CENTER_HORIZONTAL,
      5);
  topSizer->Add(sizer, 0, wxALL, 10);
  topSizer->Add(new wxStaticLine(this), 0, wxEXPAND | wxLEFT | wxRIGHT, 5);
  topSizer->Add(CreateButtonSizer(wxOK), 0, wxALL | wxEXPAND, 10);

  sizer->Add(
    GOPropertiesText(this, 0, _("Allocated sample memory")), 0, wxTOP, 5);
  float size, size1;
  size1 = m_OrganController->GetMemoryPool().GetAllocSize() / (1024.0 * 1024.0);
  size
    = m_OrganController->GetMemoryPool().GetMemoryLimit() / (1024.0 * 1024.0);
  if (m_OrganController->GetMemoryPool().GetMemoryLimit() > 0)
    sizer->Add(
      GOPropertiesText(
        this, 0, wxString::Format(_("%.3f MB of %.3f MB"), size1, size)),
      0,
      wxTOP,
      5);
  else
    sizer->Add(
      GOPropertiesText(
        this, 0, wxString::Format(_("%.3f MB (not limited)"), size1)),
      0,
      wxTOP,
      5);

  sizer->Add(
    GOPropertiesText(this, 0, _("Mapped memory of the cache")), 0, wxTOP, 5);
  size = m_OrganController->GetMemoryPool().GetMappedSize() / (1024.0 * 1024.0);
  sizer->Add(
    GOPropertiesText(this, 0, wxString::Format(_("%.3f MB"), size)),
    0,
    wxTOP,
    5);

  sizer->Add(GOPropertiesText(this, 0, _("Memory pool size")), 0, wxTOP, 5);
  size1 = m_OrganController->GetMemoryPool().GetPoolUsage() / (1024.0 * 1024.0);
  size = m_OrganController->GetMemoryPool().GetPoolSize() / (1024.0 * 1024.0);
  sizer->Add(
    GOPropertiesText(
      this, 0, wxString::Format(_("%.3f MB of %.3f MB"), size1, size)),
    0,
    wxTOP,
    5);

  sizer->Add(GOPropertiesText(this, 0, _("ODF Path")), 0, wxTOP, 5);
  sizer->Add(
    GOPropertiesText(this, 300, m_OrganController->GetOrganPathInfo()),
    0,
    wxLEFT,
    10);

  SetSizer(topSizer);
  topSizer->Fit(this);
}

GOProperties::~GOProperties(void) {}
