/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOOrganSettingsDialog.h"

#include <wx/bookctrl.h>
#include <wx/button.h>
#include <wx/sizer.h>

#include "model/GOOrganModel.h"

#include "GOOrganSettingsPipesTab.h"

enum {
  ID_EVENT_TREE = 300,
  ID_BUTTON_DISTRIBUTE_AUDIO,
  ID_BUTTON_DEFAULT,
  ID_BUTTON_DISCARD,
  ID_BUTTON_APPLY,
};

BEGIN_EVENT_TABLE(GOOrganSettingsDialog, GOTabbedDialog)
EVT_BUTTON(
  ID_BUTTON_DISTRIBUTE_AUDIO, GOOrganSettingsDialog::OnButtonDistributeAudio)
EVT_BUTTON(ID_BUTTON_DEFAULT, GOOrganSettingsDialog::OnButtonDefault)
EVT_BUTTON(ID_BUTTON_DISCARD, GOOrganSettingsDialog::OnButtonDiscard)
EVT_BUTTON(ID_BUTTON_APPLY, GOOrganSettingsDialog::OnButtonApply)
END_EVENT_TABLE()

GOOrganSettingsDialog::GOOrganSettingsDialog(
  GOOrganModel &organModel, GODocumentBase *doc, wxWindow *parent)
  : GOTabbedDialog(
    parent,
    wxT("Organ settings"),
    _("Organ settings"),
    organModel.GetConfig().m_DialogSizes,
    wxEmptyString,
    0,
    wxHELP | wxCLOSE),
    GOView(doc, this) {
  wxBookCtrlBase *pNoteBook = GetBookCtrl();

  m_PipesTab = new GOOrganSettingsPipesTab(organModel, pNoteBook, *this);
  AddTab(m_PipesTab, "Pipes", _("Pipes"));

  // add a custom button 'Reason into the space of the standard dialog button
  wxSizer *const pButtonSizer = GetButtonSizer();

  if (pButtonSizer) {
    pButtonSizer->InsertSpacer(2, 10);
    m_AudioGroupAssistant = new wxButton(
      this, ID_BUTTON_DISTRIBUTE_AUDIO, _("Distribute audio groups"));
    pButtonSizer->Insert(
      3,
      m_AudioGroupAssistant,
      0,
      wxALIGN_CENTRE_VERTICAL | wxLEFT | wxRIGHT,
      2);
    m_Default = new wxButton(this, ID_BUTTON_DEFAULT, _("Default"));
    pButtonSizer->Insert(
      4, m_Default, 0, wxALIGN_CENTRE_VERTICAL | wxLEFT | wxRIGHT, 2);
    pButtonSizer->InsertSpacer(6, 10);
    m_Discard = new wxButton(this, ID_BUTTON_DISCARD, _("Discard"));
    pButtonSizer->Insert(
      7, m_Discard, 0, wxALIGN_CENTRE_VERTICAL | wxLEFT | wxRIGHT, 2);
    m_Apply = new wxButton(this, ID_BUTTON_APPLY, _("Apply"));
    pButtonSizer->Insert(
      8, m_Apply, 0, wxALIGN_CENTRE_VERTICAL | wxLEFT | wxRIGHT, 2);
    pButtonSizer->InsertSpacer(9, 10);
  }

  LayoutDialog();
}

void GOOrganSettingsDialog::ButtonStatesChanged() {
  m_AudioGroupAssistant->Enable(m_PipesTab->IsDistributeAudioEnabled());
  m_Default->Enable(m_PipesTab->IsDefaultEnabled());
  m_Discard->Enable(m_PipesTab->IsRevertEnabled());
  m_Apply->Enable(m_PipesTab->IsApplyEnabled());
}

void GOOrganSettingsDialog::OnButtonDistributeAudio(wxCommandEvent &e) {
  m_PipesTab->DistributeAudio();
}

void GOOrganSettingsDialog::OnButtonDefault(wxCommandEvent &e) {
  m_PipesTab->ResetToDefault();
}

void GOOrganSettingsDialog::OnButtonDiscard(wxCommandEvent &e) {
  m_PipesTab->DiscardChanges();
}

void GOOrganSettingsDialog::OnButtonApply(wxCommandEvent &e) {
  m_PipesTab->ApplyChanges();
}
