/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSettingsDialog.h"

#include <wx/app.h>
#include <wx/bookctrl.h>
#include <wx/button.h>
#include <wx/choicdlg.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>

#include "GOEvent.h"
#include "GOSettingsArchives.h"
#include "GOSettingsAudioGroup.h"
#include "GOSettingsAudioOutput.h"
#include "GOSettingsDefaults.h"
#include "GOSettingsMidiDevices.h"
#include "GOSettingsMidiMessage.h"
#include "GOSettingsOption.h"
#include "GOSettingsOrgan.h"
#include "GOSettingsReverb.h"
#include "GOSettingsTemperaments.h"
#include "go_ids.h"
#include "sound/GOSound.h"

BEGIN_EVENT_TABLE(GOSettingsDialog, wxPropertySheetDialog)
EVT_SHOW(GOSettingsDialog::OnShow)
EVT_BUTTON(wxID_APPLY, GOSettingsDialog::OnApply)
EVT_BUTTON(wxID_OK, GOSettingsDialog::OnOK)
EVT_BUTTON(wxID_HELP, GOSettingsDialog::OnHelp)
EVT_BUTTON(ID_REASONS, GOSettingsDialog::OnReasons)
END_EVENT_TABLE()

GOSettingsDialog::GOSettingsDialog(
  wxWindow *win, GOSound &sound, SettingsReasons *reasons)
    : wxPropertySheetDialog(
      win,
      wxID_ANY,
      _("Program Settings"),
      wxDefaultPosition,
      wxDefaultSize,
      wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxDIALOG_NO_PARENT),
      m_Sound(sound),
      m_ReasonsAlreadyShown(false),
      m_Reasons(reasons) {
  wxBookCtrlBase *notebook = GetBookCtrl();

  m_MidiDevicePage = new SettingsMidiDevices(
    m_Sound.GetSettings(), m_Sound.GetMidi(), notebook);
  m_OptionsPage = new GOSettingsOption(m_Sound.GetSettings(), notebook);
  m_OrganPage
    = new GOSettingsOrgan(m_Sound.GetSettings(), m_Sound.GetMidi(), notebook);
  m_ArchivePage
    = new GOSettingsArchives(m_Sound.GetSettings(), *m_OrganPage, notebook);
  m_MidiMessagePage = new GOSettingsMidiMessage(
    m_Sound.GetSettings(), m_Sound.GetMidi(), notebook);
  m_GroupPage = new GOSettingsAudioGroup(m_Sound.GetSettings(), notebook);
  m_OutputPage = new GOSettingsAudioOutput(m_Sound, *m_GroupPage, notebook);
  m_ReverbPage = new GOSettingsReverb(m_Sound.GetSettings(), notebook);
  m_TemperamentsPage
    = new GOSettingsTemperaments(m_Sound.GetSettings(), notebook);
  m_DefaultsPage = new GOSettingsDefaults(m_Sound.GetSettings(), notebook);

  notebook->AddPage(m_OptionsPage, _("Options"));
  notebook->AddPage(m_DefaultsPage, _("Defaults and Initial Settings"));
  notebook->AddPage(m_OutputPage, _("Audio Output"));
  notebook->AddPage(m_ReverbPage, _("Reverb"));
  notebook->AddPage(m_GroupPage, _("Audio Groups"));
  notebook->AddPage(m_OrganPage, _("Organs"));
  notebook->AddPage(m_MidiDevicePage, _("MIDI Devices"));
  notebook->AddPage(m_TemperamentsPage, _("Temperaments"));
  notebook->AddPage(m_MidiMessagePage, _("Initial MIDI Configuration"));
  notebook->AddPage(m_ArchivePage, _("Organ Packages"));

  bool hasReasons = reasons && reasons->size();

  notebook->SetSelection(
    hasReasons ? reasons->operator[](0).m_SettingsPageNum : 0);

  // add a custom button 'Reason into the space of the standard dialog button
  wxSizer *const pButtonSizer = CreateButtonSizer(wxOK | wxCANCEL | wxHELP);

  if (pButtonSizer) {
    wxButton *const pReasonBtn = new wxButton(this, ID_REASONS, _("Reason"));

    if (!hasReasons) pReasonBtn->Disable();

    pButtonSizer->Insert(
      3, pReasonBtn, wxSizerFlags().Border(wxLEFT | wxRIGHT, 20));
    GetInnerSizer()->Add(
      pButtonSizer, wxSizerFlags().Expand().Border(wxALL, 2));
    GetInnerSizer()->AddSpacer(2);
  }

  LayoutDialog();
}

void GOSettingsDialog::OnShow(wxShowEvent &) {
  if (!m_ReasonsAlreadyShown && m_Reasons && m_Reasons->size()) {
    wxCommandEvent event(wxEVT_BUTTON, ID_REASONS);

    GetEventHandler()->AddPendingEvent(event);
  }
  m_ReasonsAlreadyShown = true;
}

void GOSettingsDialog::OnApply(wxCommandEvent &event) { DoApply(); }

void GOSettingsDialog::OnHelp(wxCommandEvent &event) {
  wxCommandEvent help(wxEVT_SHOWHELP, 0);
  help.SetString(GetBookCtrl()->GetPageText(GetBookCtrl()->GetSelection()));
  wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(help);
}

void GOSettingsDialog::OnOK(wxCommandEvent &event) {
  if (DoApply()) event.Skip();
}

bool GOSettingsDialog::DoApply() {
  if (!(this->Validate())) return false;

  m_MidiDevicePage->Save();
  m_OptionsPage->Save();
  m_OrganPage->Save();
  m_ArchivePage->Save();
  m_GroupPage->Save();
  m_OutputPage->Save();
  m_ReverbPage->Save();
  m_TemperamentsPage->Save();
  m_DefaultsPage->Save();

  return true;
}

void GOSettingsDialog::OnReasons(wxCommandEvent &event) {
  unsigned nReasons = m_Reasons ? (unsigned)m_Reasons->size() : 0;

  if (nReasons) {
    wxBookCtrlBase *const notebook = GetBookCtrl();
    const int currPageNum = notebook->GetSelection();
    wxArrayString reasonStrs;
    unsigned currReasonIndex = 0;

    for (unsigned i = 0; i < nReasons; i++) {
      const GOSettingsReason &reason(m_Reasons->operator[](i));

      reasonStrs.Add(reason.m_ReasonMessage);
      if ((int)reason.m_SettingsPageNum == currPageNum) currReasonIndex = i;
    }

    const int index = wxGetSingleChoiceIndex(
      wxEmptyString, _("Settings Reason"), reasonStrs, currReasonIndex, this);

    if (index >= 0)
      GetBookCtrl()->SetSelection(
        m_Reasons->operator[](index).m_SettingsPageNum);
  }
}

bool GOSettingsDialog::NeedReload() { return m_OptionsPage->NeedReload(); }

bool GOSettingsDialog::NeedRestart() { return m_OptionsPage->NeedRestart(); }
