/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/


#include <wx/app.h>
#include <wx/bookctrl.h>
#include <wx/button.h>
#include <wx/choicdlg.h> 
#include <wx/msgdlg.h>
#include <wx/sizer.h>

#include "SettingsDialog.h"

#include "GOEvent.h"
#include "GOSound.h"
#include "go_ids.h"
#include "SettingsArchives.h"
#include "SettingsAudioGroup.h"
#include "SettingsAudioOutput.h"
#include "SettingsDefaults.h"
#include "SettingsMidiDevices.h"
#include "SettingsMidiMessage.h"
#include "SettingsOption.h"
#include "SettingsOrgan.h"
#include "SettingsReverb.h"
#include "SettingsTemperaments.h"

BEGIN_EVENT_TABLE(SettingsDialog, wxPropertySheetDialog)
  EVT_SHOW(SettingsDialog::OnShow)
  EVT_BUTTON(wxID_APPLY, SettingsDialog::OnApply)
  EVT_BUTTON(wxID_OK, SettingsDialog::OnOK)
  EVT_BUTTON(wxID_HELP, SettingsDialog::OnHelp)
  EVT_BUTTON(ID_REASONS, SettingsDialog::OnReasons)
END_EVENT_TABLE()

SettingsDialog::SettingsDialog(
  wxWindow* win, GOSound& sound, SettingsReasons *reasons
): wxPropertySheetDialog(
  win, wxID_ANY, _("Program Settings"), wxDefaultPosition, wxDefaultSize,
  wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxDIALOG_NO_PARENT
), m_Sound(sound), m_ReasonsAlreadyShown(false), m_Reasons(reasons)
{
  wxBookCtrlBase* notebook = GetBookCtrl();

  m_MidiDevicePage = new SettingsMidiDevices(m_Sound, notebook);
  m_OptionsPage = new SettingsOption(m_Sound.GetSettings(), notebook);
  m_OrganPage = new SettingsOrgan(m_Sound.GetSettings(), m_Sound.GetMidi(), notebook);
  m_ArchivePage = new SettingsArchives(m_Sound.GetSettings(), *m_OrganPage, notebook);
  m_MidiMessagePage = new SettingsMidiMessage(m_Sound.GetSettings(), m_Sound.GetMidi(), notebook);
  m_GroupPage = new SettingsAudioGroup(m_Sound.GetSettings(), notebook);
  m_OutputPage = new SettingsAudioOutput(m_Sound, *m_GroupPage, notebook);
  m_ReverbPage = new SettingsReverb(m_Sound.GetSettings(), notebook);
  m_TemperamentsPage = new SettingsTemperaments(m_Sound.GetSettings(), notebook);
  m_DefaultsPage = new SettingsDefaults(m_Sound.GetSettings(), notebook);

  notebook->AddPage(m_OptionsPage,  _("Options"));
  notebook->AddPage(m_DefaultsPage,  _("Defaults and Initial Settings"));
  notebook->AddPage(m_OutputPage, _("Audio Output"));
  notebook->AddPage(m_ReverbPage, _("Reverb"));
  notebook->AddPage(m_GroupPage, _("Audio Groups"));
  notebook->AddPage(m_OrganPage, _("Organs"));
  notebook->AddPage(m_MidiDevicePage,  _("MIDI Devices"));
  notebook->AddPage(m_TemperamentsPage, _("Temperaments"));
  notebook->AddPage(m_MidiMessagePage, _("Initial MIDI Configuration"));
  notebook->AddPage(m_ArchivePage, _("Organ Packages"));
  
  bool hasReasons = reasons && reasons->size();
  
  notebook->SetSelection(hasReasons ? reasons->operator[](0).m_SettingsPageNum : 0);

  // add a custom button 'Reason into the space of the standard dialog button
  wxSizer * const pButtonSizer = CreateButtonSizer(wxOK | wxCANCEL | wxHELP);
  
  if (pButtonSizer)
  {
    wxButton * const pReasonBtn = new wxButton(this, ID_REASONS, _("Reason"));
    
    if (! hasReasons)
      pReasonBtn->Disable();
      
    pButtonSizer->Insert(3, pReasonBtn, wxSizerFlags().Border(wxLEFT | wxRIGHT, 20));
    GetInnerSizer()->Add(pButtonSizer, wxSizerFlags().Expand().Border(wxALL, 2));
    GetInnerSizer()->AddSpacer(2);
  }

  LayoutDialog();
}

void SettingsDialog::OnShow(wxShowEvent &)
{
  if (! m_ReasonsAlreadyShown && m_Reasons && m_Reasons->size())
  {
    wxCommandEvent event(wxEVT_BUTTON, ID_REASONS);

    GetEventHandler()->AddPendingEvent(event);
  }
  m_ReasonsAlreadyShown = true;
}

void SettingsDialog::OnApply(wxCommandEvent& event)
{
	DoApply();
}

void SettingsDialog::OnHelp(wxCommandEvent& event)
{
	wxCommandEvent help(wxEVT_SHOWHELP, 0);
	help.SetString(GetBookCtrl()->GetPageText(GetBookCtrl()->GetSelection()));
	wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(help);
}

void SettingsDialog::OnOK(wxCommandEvent& event)
{
	if (DoApply())
		event.Skip();
}

bool SettingsDialog::DoApply()
{
	if (!(this->Validate()))
		return false;

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

void SettingsDialog::OnReasons(wxCommandEvent& event)
{
  unsigned nReasons = m_Reasons ? (unsigned) m_Reasons->size() : 0;
  
  if (nReasons)
  {
    wxBookCtrlBase* const notebook = GetBookCtrl();
    const int currPageNum = notebook->GetSelection();
    wxArrayString reasonStrs;
    unsigned currReasonIndex = 0;
    
    for (unsigned i = 0; i < nReasons; i++)
    {
      const SettingsReason &reason(m_Reasons->operator[](i));
      
      reasonStrs.Add(reason.m_ReasonMessage);
      if ((int) reason.m_SettingsPageNum == currPageNum)
	currReasonIndex = i;
    }
    
    const int index = wxGetSingleChoiceIndex(wxEmptyString, _("Settings Reason"), reasonStrs, currReasonIndex, this);
    
    if (index >= 0)
      GetBookCtrl()->SetSelection(m_Reasons->operator[](index).m_SettingsPageNum);
  }
}

bool SettingsDialog::NeedReload()
{
  return m_OptionsPage->NeedReload();
}

bool SettingsDialog::NeedRestart()
{
  return m_OptionsPage->NeedRestart();
}
