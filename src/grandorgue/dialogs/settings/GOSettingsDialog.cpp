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
#include "GOSettingsAudio.h"
#include "GOSettingsMidiDevices.h"
#include "GOSettingsMidiMessage.h"
#include "GOSettingsOptions.h"
#include "GOSettingsOrgans.h"
#include "GOSettingsPaths.h"
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

  m_OptionsPage = new GOSettingsOptions(m_Sound.GetSettings(), notebook);
  notebook->AddPage(m_OptionsPage, _("Options"));
  m_PathsPage = new GOSettingsPaths(m_Sound.GetSettings(), notebook);
  notebook->AddPage(m_PathsPage, _("Paths"));
  m_AudioPage = new GOSettingsAudio(m_Sound.GetSettings(), m_Sound, notebook);
  notebook->AddPage(m_AudioPage, _("Audio"));
  m_MidiDevicePage = new SettingsMidiDevices(
    m_Sound.GetSettings(), m_Sound.GetMidi(), notebook);
  notebook->AddPage(m_MidiDevicePage, _("MIDI Devices"));
  m_MidiMessagePage = new GOSettingsMidiMessage(
    m_Sound.GetSettings(), m_Sound.GetMidi(), notebook);
  notebook->AddPage(m_MidiMessagePage, _("Initial MIDI"));
  m_OrgansPage
    = new GOSettingsOrgans(m_Sound.GetSettings(), m_Sound.GetMidi(), notebook);
  notebook->AddPage(m_OrgansPage, _("Organs"));
  m_ReverbPage = new GOSettingsReverb(m_Sound.GetSettings(), notebook);
  notebook->AddPage(m_ReverbPage, _("Reverb"));
  m_TemperamentsPage
    = new GOSettingsTemperaments(m_Sound.GetSettings(), notebook);
  notebook->AddPage(m_TemperamentsPage, _("Temperaments"));

  bool hasReasons = reasons && reasons->size();

  notebook->SetSelection(
    hasReasons ? reasons->operator[](0).m_SettingsPageNum : 0);

  // add a custom button 'Reason into the space of the standard dialog button
  wxSizer *const pButtonSizer = CreateButtonSizer(wxOK | wxCANCEL | wxHELP);

  if (pButtonSizer) {
    wxButton *const pReasonBtn = new wxButton(this, ID_REASONS, _("Reason"));

    if (!hasReasons)
      pReasonBtn->Disable();

    pButtonSizer->Insert(
      3, pReasonBtn, 0, wxALIGN_CENTRE_VERTICAL | wxLEFT | wxRIGHT, 10);
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
  if (DoApply())
    event.Skip();
}

bool GOSettingsDialog::DoApply() {
  if (!(this->Validate()))
    return false;

  m_OptionsPage->Save();
  m_PathsPage->Save();
  m_AudioPage->Save();
  m_MidiDevicePage->Save();
  m_OrgansPage->Save();
  m_ReverbPage->Save();
  m_TemperamentsPage->Save();

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
      if ((int)reason.m_SettingsPageNum == currPageNum)
        currReasonIndex = i;
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
