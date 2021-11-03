/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOButton.h"

#include "config/GOConfigReader.h"
#include "GODocument.h"
#include "settings/GOSettings.h"
#include "GODefinitionFile.h"
#include <wx/intl.h>

GOButton::GOButton(GODefinitionFile* organfile, MIDI_RECEIVER_TYPE midi_type, bool pushbutton) :
	m_organfile(organfile),
	m_midi(organfile, midi_type),
	m_sender(organfile, MIDI_SEND_BUTTON),
	m_shortcut(organfile, KEY_RECV_BUTTON),
	m_Pushbutton(pushbutton),
	m_Displayed(false),
	m_Name(),
	m_Engaged(false),
	m_DisplayInInvertedState(false),
	m_ReadOnly(false)
{
	m_organfile->RegisterEventHandler(this);
	m_organfile->RegisterMidiConfigurator(this);
	m_organfile->RegisterPlaybackStateHandler(this);
}

GOButton::~GOButton()
{
}

void GOButton::Init(GOConfigReader& cfg, wxString group, wxString name)
{
	m_organfile->RegisterSaveableObject(this);
	m_group = group;
	m_Name = name;
	m_Displayed = false;
	m_DisplayInInvertedState = false;
	if (!m_ReadOnly)
	{
		m_midi.Load(cfg, group, m_organfile->GetSettings().GetMidiMap());
		m_shortcut.Load(cfg, group);
	}
	m_sender.Load(cfg, group, m_organfile->GetSettings().GetMidiMap());
}

void GOButton::Load(GOConfigReader& cfg, wxString group)
{
	m_organfile->RegisterSaveableObject(this);
	m_group = group;
	m_Name = cfg.ReadStringNotEmpty(ODFSetting, group, wxT("Name"), true);
	m_Displayed = cfg.ReadBoolean(ODFSetting, group, wxT("Displayed"), false, false);
	m_DisplayInInvertedState = cfg.ReadBoolean(ODFSetting, group, wxT("DisplayInInvertedState"), false, false);
	if (!m_ReadOnly)
	{
		m_midi.Load(cfg, group, m_organfile->GetSettings().GetMidiMap());
		m_shortcut.Load(cfg, group);
	}
	m_sender.Load(cfg, group, m_organfile->GetSettings().GetMidiMap());
}

void GOButton::Save(GOConfigWriter& cfg)
{
	if (!m_ReadOnly)
	{
		m_midi.Save(cfg, m_group, m_organfile->GetSettings().GetMidiMap());
		m_shortcut.Save(cfg, m_group);
	}
	m_sender.Save(cfg, m_group, m_organfile->GetSettings().GetMidiMap());
}

bool GOButton::IsDisplayed()
{
	return m_Displayed;
}

bool GOButton::IsReadOnly()
{
	return m_ReadOnly;
}

const wxString& GOButton::GetName()
{
	return m_Name;
}

void GOButton::HandleKey(int key)
{
	if (m_ReadOnly)
		return;
	switch (m_shortcut.Match(key))
	{
	case KEY_MATCH:
		Push();
		break;

	default:
		break;
	}
}

void GOButton::Push()
{
	if (m_ReadOnly)
		return;
	Set(m_Engaged ^ true);
}

void GOButton::Set(bool on)
{
}

void GOButton::AbortPlayback()
{
	m_sender.SetDisplay(false);
	m_sender.SetName(wxEmptyString);
}

void GOButton::PreparePlayback()
{
	m_midi.PreparePlayback();
	m_sender.SetName(m_Name);
}

void GOButton::StartPlayback()
{
}

void GOButton::PrepareRecording()
{
	m_sender.SetDisplay(m_Engaged);
}

void GOButton::ProcessMidi(const GOMidiEvent& event)
{
	if (m_ReadOnly)
		return;
	switch(m_midi.Match(event))
	{
	case MIDI_MATCH_CHANGE:
		Push();
		break;

	case MIDI_MATCH_ON:
		if (m_Pushbutton)
			Push();
		else
			Set(true);
		break;

	case MIDI_MATCH_OFF:
		if (!m_Pushbutton)
			Set(false);
		break;

	default:
		break;
	}
}

void GOButton::Display(bool onoff)
{
	if (m_Engaged == onoff)
		return;
	m_sender.SetDisplay(onoff);
	m_Engaged = onoff;
	m_organfile->ControlChanged(this);
}

bool GOButton::IsEngaged() const
{
	return m_Engaged;
}

bool GOButton::DisplayInverted() const
{
	return m_DisplayInInvertedState;
}

void GOButton::SetElementID(int id)
{
	if (!m_ReadOnly)
	{
		m_midi.SetElementID(id);
		m_sender.SetElementID(id);
	}
}

void GOButton::SetShortcutKey(unsigned key)
{
	m_shortcut.SetShortcut(key);
}

void GOButton::SetPreconfigIndex(unsigned index)
{
	m_midi.SetIndex(index);
}

wxString GOButton::GetMidiName()
{
	return GetName();
}

void GOButton::ShowConfigDialog()
{
	wxString title = wxString::Format(_("Midi-Settings for %s - %s"), GetMidiType().c_str(), GetMidiName().c_str());

	GOMidiReceiver* midi = &m_midi;
	GOKeyReceiver* key = &m_shortcut;
	if (IsReadOnly())
	{
		midi = NULL;
		key = NULL;
	}

	m_organfile->GetDocument()->ShowMIDIEventDialog(this, title, midi, &m_sender, key);
}

wxString GOButton::GetElementStatus()
{
	return m_Engaged ? _("ON") : _("OFF");
}

std::vector<wxString> GOButton::GetElementActions()
{
	std::vector<wxString> actions;
	actions.push_back(_("trigger"));
	return actions;
}

void GOButton::TriggerElementActions(unsigned no)
{
	if (no == 0)
		Push();
}
