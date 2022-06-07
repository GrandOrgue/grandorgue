/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
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

BEGIN_EVENT_TABLE(GOSettingsDialog, GOTabbedDialog)
EVT_BUTTON(ID_REASONS, GOSettingsDialog::OnReasons)
END_EVENT_TABLE()

const wxString GOSettingsDialog::PAGE_OPTIONS = wxT("Options");
const wxString GOSettingsDialog::PAGE_PATHS = wxT("Paths");
const wxString GOSettingsDialog::PAGE_AUDIO = wxT("Audio");
const wxString GOSettingsDialog::PAGE_MIDI_DEVICES = wxT("MidiDevices");
const wxString GOSettingsDialog::PAGE_INITIAL_MIDI = wxT("InitialMidi");
const wxString GOSettingsDialog::PAGE_ORGANS = wxT("Organs");
const wxString GOSettingsDialog::PAGE_REVERB = wxT("Reverb");
const wxString GOSettingsDialog::PAGE_TEMPERAMENTS = wxT("Temperaments");

GOSettingsDialog::GOSettingsDialog(
  wxWindow *win, GOSound &sound, SettingsReasons *reasons)
  : GOTabbedDialog(win, "Settings", _("Program Settings"), wxDIALOG_NO_PARENT),
    m_Sound(sound),
    m_ReasonsAlreadyShown(false),
    m_Reasons(reasons) {
  wxBookCtrlBase *notebook = GetBook();

  m_OptionsPage = new GOSettingsOptions(m_Sound.GetSettings(), notebook);
  AddTab(m_OptionsPage, PAGE_OPTIONS, _("Options"));
  m_PathsPage = new GOSettingsPaths(m_Sound.GetSettings(), notebook);
  AddTab(m_PathsPage, PAGE_PATHS, _("Paths"));
  m_AudioPage = new GOSettingsAudio(m_Sound.GetSettings(), m_Sound, notebook);
  AddTab(m_AudioPage, PAGE_AUDIO, _("Audio"));
  m_MidiDevicePage = new SettingsMidiDevices(
    m_Sound.GetSettings(), m_Sound.GetMidi(), notebook);
  AddTab(m_MidiDevicePage, PAGE_MIDI_DEVICES, _("MIDI Devices"));
  m_MidiMessagePage = new GOSettingsMidiMessage(
    m_Sound.GetSettings(), m_Sound.GetMidi(), notebook);
  AddTab(m_MidiMessagePage, PAGE_INITIAL_MIDI, _("Initial MIDI"));
  m_OrgansPage
    = new GOSettingsOrgans(m_Sound.GetSettings(), m_Sound.GetMidi(), notebook);
  AddTab(m_OrgansPage, PAGE_ORGANS, _("Organs"));
  m_ReverbPage = new GOSettingsReverb(m_Sound.GetSettings(), notebook);
  AddTab(m_ReverbPage, PAGE_REVERB, _("Reverb"));
  m_TemperamentsPage
    = new GOSettingsTemperaments(m_Sound.GetSettings(), notebook);
  AddTab(m_TemperamentsPage, PAGE_TEMPERAMENTS, _("Temperaments"));

  bool hasReasons = reasons && reasons->size();

  // add a custom button 'Reason into the space of the standard dialog button
  wxSizer *const pButtonSizer = GetButtonSizer();

  if (pButtonSizer) {
    wxButton *const pReasonBtn = new wxButton(this, ID_REASONS, _("Reason"));

    if (!hasReasons)
      pReasonBtn->Disable();

    pButtonSizer->Insert(
      3, pReasonBtn, 0, wxALIGN_CENTRE_VERTICAL | wxLEFT | wxRIGHT, 10);
  }
  LayoutDialog();

  if (hasReasons)
    NavigateToTab((*reasons)[0].m_SettingsPageName);
}

void GOSettingsDialog::OnShow() {
  GOTabbedDialog::OnShow();
  if (!m_ReasonsAlreadyShown && m_Reasons && m_Reasons->size()) {
    wxCommandEvent event(wxEVT_BUTTON, ID_REASONS);

    GetEventHandler()->AddPendingEvent(event);
  }
  m_ReasonsAlreadyShown = true;
}

void GOSettingsDialog::OnReasons(wxCommandEvent &event) {
  unsigned nReasons = m_Reasons ? (unsigned)m_Reasons->size() : 0;

  if (nReasons) {
    const wxString &currPageName = GetCurrTabName();
    wxArrayString reasonStrs;
    unsigned currReasonIndex = 0;

    for (unsigned i = 0; i < nReasons; i++) {
      const GOSettingsReason &reason((*m_Reasons)[i]);

      reasonStrs.Add(reason.m_ReasonMessage);
      if (reason.m_SettingsPageName == currPageName)
        currReasonIndex = i;
    }

    const int index = wxGetSingleChoiceIndex(
      wxEmptyString, _("Settings Reason"), reasonStrs, currReasonIndex, this);

    if (index >= 0)
      NavigateToTab((*m_Reasons)[index].m_SettingsPageName);
  }
}

bool GOSettingsDialog::NeedReload() { return m_OptionsPage->NeedReload(); }

bool GOSettingsDialog::NeedRestart() { return m_OptionsPage->NeedRestart(); }
