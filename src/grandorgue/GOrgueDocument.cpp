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

#include "GOGUIPanel.h"
#include "GOrgueDocument.h"
#include "GOrgueEvent.h"
#include "GOrgueMidiEvent.h"
#include "GOrgueSettings.h"
#include "GOrgueSound.h"
#include "GOrgueDialogView.h"
#include "GOrguePanelView.h"
#include "GrandOrgueID.h"
#include "GrandOrgueFile.h"
#include "MIDIEventDialog.h"
#include "OrganDialog.h"

GOrgueDocument::GOrgueDocument(GOrgueSound* sound) :
	m_OrganFileReady(false),
	m_organfile(NULL),
	m_sound(*sound),
	m_listener(),
	m_Windows()
{
	m_listener.Register(&m_sound.GetMidi());
}

GOrgueDocument::~GOrgueDocument()
{
	m_listener.SetCallback(NULL);
	CloseOrgan();
}

bool GOrgueDocument::OnCloseDocument()
{
    return wxDocument::OnCloseDocument();
}

bool GOrgueDocument::DoOpenDocument(const wxString& file)
{
	return Load(file);
}

bool GOrgueDocument::DoSaveDocument(const wxString& file)
{
	Save();
	return true;
}

bool GOrgueDocument::Load(const wxString& odf)
{
	return Import(odf, wxEmptyString);
}

bool GOrgueDocument::Import(const wxString& odf, const wxString& cmb)
{
	wxBusyCursor busy;

	CloseOrgan();
	bool open_sound = m_sound.OpenSound();

	/* abort if we failed to open the sound device */
	if (!open_sound)
		return false;

	m_organfile = new GrandOrgueFile(this, m_sound.GetSettings());
	wxString error = m_organfile->Load(odf, cmb);
	if (!error.IsEmpty())
	{
		if (error != wxT("!"))
		{
			wxLogError(wxT("%s\n"),error.c_str());
			wxMessageBox(error, _("Load error"), wxOK | wxICON_ERROR, NULL);
		}
		CloseOrgan();
		if (!cmb.IsEmpty())
			DeleteAllViews();
		return false;
	}
	m_sound.GetSettings().AddOrgan(new GOrgueOrgan(m_organfile->GetODFFilename(), m_organfile->GetChurchName(), m_organfile->GetOrganBuilder(), m_organfile->GetRecordingDetails()));
	m_sound.PreparePlayback(m_organfile);
	if (m_organfile->GetVolume() != -121)
	{
		wxCommandEvent event(wxEVT_SETVALUE, ID_METER_AUDIO_SPIN);
		event.SetInt(m_organfile->GetVolume());
		wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);

		m_sound.GetEngine().SetVolume(m_organfile->GetVolume());
	}
	m_OrganFileReady = true;

	SetTitle(m_organfile->GetChurchName());
	m_sound.GetSettings().SetLastFile(m_organfile->GetODFFilename());

	ShowPanel(0);
	for (unsigned i = 1; i < m_organfile->GetPanelCount(); i++)
		if (m_organfile->GetPanel(i)->InitialOpenWindow())
			ShowPanel(i);

	m_listener.SetCallback(this);
	m_sound.GetSettings().Flush();
	return true;
}

bool GOrgueDocument::ImportCombination(const wxString& cmb)
{
	m_organfile->LoadCombination(cmb);
	m_organfile->Modified();
	return true;
}

void GOrgueDocument::ShowPanel(unsigned id)
{
	GOGUIPanel* panel = m_organfile->GetPanel(id);

	if (!showWindow(GOrgueDocument::PANEL, panel))
	{
		registerWindow(GOrgueDocument::PANEL, panel,
			       GOrguePanelView::createWindow(this, panel, id == 0 ? wxTheApp->GetTopWindow() : NULL));
	}
}

void GOrgueDocument::SyncState()
{
	m_organfile->SetVolume(m_sound.GetEngine().GetVolume());
	for (unsigned i = 0; i < m_organfile->GetPanelCount(); i++)
		m_organfile->GetPanel(i)->SetInitialOpenWindow(false);
	for(unsigned i = 0; i < m_Windows.size(); i++)
		if (m_Windows[i].type == PANEL && m_Windows[i].data)
			((GOrguePanelView*)m_Windows[i].window)->SyncState();
}

bool GOrgueDocument::Save()
{
	SyncState();
	m_organfile->Save();
	return true;
}

bool GOrgueDocument::Export(const wxString& cmb)
{
	SyncState();
	m_organfile->Export(cmb);
	return true;
}

void GOrgueDocument::CloseOrgan()
{
	m_listener.SetCallback(NULL);
	m_sound.CloseSound();
	while(m_Windows.size() > 0)
	{
		GOrgueDialogView* wnd = m_Windows[0].window;
		m_Windows.erase(m_Windows.begin());
		wnd->RemoveView();
	}

	m_OrganFileReady = false;
	GOMutexLocker locker(m_lock);
	if (m_organfile)
	{
		delete m_organfile;
		m_organfile = 0;
	}
}

GrandOrgueFile* GOrgueDocument::GetOrganFile()
{
	return m_organfile;
}

void GOrgueDocument::OnMidiEvent(const GOrgueMidiEvent& event)
{
	GOMutexLocker locker(m_lock);

	if (!m_OrganFileReady)
		return;

	if (m_organfile)
		m_organfile->ProcessMidi(event);
}

bool GOrgueDocument::WindowExists(WindowType type, void* data)
{
	for(unsigned i = 0; i < m_Windows.size(); i++)
		if (m_Windows[i].type == type && m_Windows[i].data == data)
			return true;
	return false;
}

bool GOrgueDocument::showWindow(WindowType type, void* data)
{
	for(unsigned i = 0; i < m_Windows.size(); i++)
		if (m_Windows[i].type == type && m_Windows[i].data == data)
		{
			m_Windows[i].window->ShowView();
			return true;
		}
	return false;
}

void GOrgueDocument::registerWindow(WindowType type, void* data, GOrgueDialogView *window)
{
	WindowInfo info;
	info.type = type;
	info.data = data;
	info.window = window;
	m_Windows.push_back(info);
	window->ShowView();
}

void GOrgueDocument::unregisterWindow(GOrgueDialogView* window)
{
	for(unsigned i = 0; i < m_Windows.size(); i++)
		if (m_Windows[i].window == window)
		{
			m_Windows.erase(m_Windows.begin() + i);
			return;
		}
}

void GOrgueDocument::ShowOrganDialog()
{
	if (!showWindow(GOrgueDocument::ORGAN_DIALOG, NULL) && m_organfile)
	{
		registerWindow(GOrgueDocument::ORGAN_DIALOG, NULL,
			       new OrganDialog(this, NULL, m_organfile));
	}
}

void GOrgueDocument::ShowMIDIEventDialog(void* element, wxString title, GOrgueMidiReceiver* event, GOrgueMidiSender* sender, GOrgueKeyReceiver* key)
{
	if (!showWindow(GOrgueDocument::MIDI_EVENT, element) && m_organfile)
	{
		registerWindow(GOrgueDocument::MIDI_EVENT, element,
			       new MIDIEventDialog (this, NULL, title, m_organfile->GetSettings(), event, sender, key));
	}
}
