/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2013 GrandOrgue contributors (see AUTHORS)
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

#include "SettingsDialog.h"

#include "GOrgueEvent.h"
#include "GOrgueSettings.h"
#include "GOrgueSound.h"
#include "GrandOrgueID.h"
#include "SettingsAudioGroup.h"
#include "SettingsAudioOutput.h"
#include "SettingsMidiDevices.h"
#include "SettingsMidiMessage.h"
#include "SettingsOption.h"
#include "SettingsOrgan.h"
#include "SettingsReverb.h"
#include <wx/app.h>
#include <wx/bookctrl.h>
#include <wx/msgdlg.h>

BEGIN_EVENT_TABLE(SettingsDialog, wxPropertySheetDialog)
	EVT_BUTTON(wxID_APPLY, SettingsDialog::OnApply)
	EVT_BUTTON(wxID_OK, SettingsDialog::OnOK)
	EVT_BUTTON(wxID_HELP, SettingsDialog::OnHelp)
END_EVENT_TABLE()

SettingsDialog::SettingsDialog(wxWindow* win, GOrgueSound& sound) :
	wxPropertySheetDialog(win, wxID_ANY, _("Midi & Audio Settings"), wxDefaultPosition, wxSize(740,600)),
	m_Sound(sound)
{
	CreateButtons(wxOK | wxCANCEL | wxHELP);

	wxBookCtrlBase* notebook = GetBookCtrl();

	m_MidiDevicePage = new SettingsMidiDevices(m_Sound, notebook);
	m_OptionsPage = new SettingsOption(m_Sound.GetSettings(), notebook);
	m_OrganPage = new SettingsOrgan(m_Sound.GetSettings(), m_Sound.GetMidi(), notebook);
	m_MidiMessagePage = new SettingsMidiMessage(m_Sound.GetSettings(), m_Sound.GetMidi(), notebook);
	m_GroupPage = new SettingsAudioGroup(m_Sound.GetSettings(), notebook);
	m_OutputPage = new SettingsAudioOutput(m_Sound, *m_GroupPage, notebook);
	m_ReverbPage = new SettingsReverb(m_Sound.GetSettings(), notebook);

	notebook->AddPage(m_OptionsPage,  _("Options"));
	notebook->AddPage(m_OutputPage, _("Audio Output"));
	notebook->AddPage(m_ReverbPage, _("Reverb"));
	notebook->AddPage(m_GroupPage, _("Audio Groups"));
	notebook->AddPage(m_OrganPage, _("Organs"));
	notebook->AddPage(m_MidiDevicePage,  _("MIDI Devices"));
	notebook->AddPage(m_MidiMessagePage, _("Initial MIDI Configuration"));

}

SettingsDialog::~SettingsDialog()
{
	if (m_OptionsPage->NeedReload() &&  m_Sound.GetOrganFile() != NULL)
	{
		if (wxMessageBox(_("Some changed settings effect unless the sample set is reloaded.\n\nWould you like to reload the sample set now?"), _("GrandOrgue"), wxYES_NO | wxICON_QUESTION, this) == wxYES)
		{
			wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_FILE_RELOAD);
			wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);
		}
	}
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
	m_GroupPage->Save();
	m_OutputPage->Save();
	m_ReverbPage->Save();

	m_Sound.ResetSound(true);
	m_Sound.GetSettings().Flush();

	return true;
}

