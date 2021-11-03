/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueDocument.h"

#include "GOGUIPanel.h"
#include "GOrgueEvent.h"
#include "GOrgueMidiEvent.h"
#include "GOrgueOrgan.h"
#include "GOrguePanelView.h"
#include "GOrgueSettings.h"
#include "GOrgueSound.h"
#include "GOrgueView.h"
#include "GrandOrgueFile.h"
#include "GrandOrgueFrame.h"
#include "GrandOrgueID.h"
#include "MIDIEventDialog.h"
#include "MIDIList.h"
#include "OrganDialog.h"
#include "threading/GOMutexLocker.h"
#include <wx/app.h>

GOrgueDocument::GOrgueDocument(GOrgueSound* sound) :
	m_OrganFileReady(false),
	m_organfile(NULL),
	m_sound(*sound),
	m_listener(),
	m_modified(false)
{
	m_listener.Register(&m_sound.GetMidi());
}

GOrgueDocument::~GOrgueDocument()
{
	m_listener.SetCallback(NULL);
	CloseOrgan();
}

bool GOrgueDocument::IsModified()
{
	return m_modified;
}

void GOrgueDocument::Modify(bool modified)
{
	m_modified = modified;
}

bool GOrgueDocument::Load(GOrgueProgressDialog* dlg, const GOrgueOrgan& organ)
{
	return Import(dlg, organ, wxEmptyString);
}

bool GOrgueDocument::Import(GOrgueProgressDialog* dlg, const GOrgueOrgan& organ, const wxString& cmb)
{
	wxBusyCursor busy;

	CloseOrgan();
	Modify(false);
	m_organfile = new GrandOrgueFile(this, m_sound.GetSettings());
	wxString error = m_organfile->Load(dlg, organ, cmb);
	if (!error.IsEmpty())
	{
		if (error != wxT("!"))
		{
			wxLogError(wxT("%s\n"),error.c_str());
			GOMessageBox(error, _("Load error"), wxOK | wxICON_ERROR, NULL);
		}
		CloseOrgan();
		return false;
	}
	m_sound.GetSettings().AddOrgan(m_organfile->GetOrganInfo());
	{
		wxCommandEvent event(wxEVT_SETVALUE, ID_METER_AUDIO_SPIN);
		event.SetInt(m_organfile->GetVolume());
		wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);

		m_sound.GetEngine().SetVolume(m_organfile->GetVolume());
	}
	m_sound.GetSettings().Flush();

	wxCommandEvent event(wxEVT_WINTITLE, 0);
	event.SetString(m_organfile->GetChurchName());
	wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);

	for (unsigned i = 0; i < m_organfile->GetPanelCount(); i++)
		if (m_organfile->GetPanel(i)->InitialOpenWindow())
			ShowPanel(i);
	if (!m_organfile->GetMainWindowData()->GetWindowSize().IsEmpty())
	{
	  GOrgueFrame * const frame = dynamic_cast<GOrgueFrame *>(wxTheApp->GetTopWindow());
	  
	  if (frame)
	    frame->ApplyRectFromSettings(m_organfile->GetMainWindowData()->GetWindowSize());
	}

	m_sound.AssignOrganFile(m_organfile);
	m_OrganFileReady = true;
	m_listener.SetCallback(this);
	Modify(!cmb.IsEmpty());

	/* The sound was open on GOrgueFrame::Init. 
	 * m_sound.AssignOrganFile made all necessary for the new organfile.
	 * So the new opening is not necessary
	if (m_sound.OpenSound())
		return false;
	 */
	return true;
}

bool GOrgueDocument::ImportCombination(const wxString& cmb)
{
	if (!m_organfile)
		return false;
	m_organfile->LoadCombination(cmb);
	m_organfile->Modified();
	return true;
}

bool GOrgueDocument::UpdateCache(GOrgueProgressDialog* dlg, bool compress)
{
	if (!m_organfile)
		return false;
	return m_organfile->UpdateCache(dlg, compress);
}

void GOrgueDocument::ShowPanel(unsigned id)
{
	GOGUIPanel* panel = m_organfile->GetPanel(id);

	if (!showWindow(GOrgueDocument::PANEL, panel))
	{
		registerWindow(GOrgueDocument::PANEL, panel,
			       GOrguePanelView::createWithFrame(this, panel));
	}
}

void GOrgueDocument::SyncState()
{
	m_organfile->SetVolume(m_sound.GetEngine().GetVolume());
	m_organfile->GetMainWindowData()->SetWindowSize(wxTheApp->GetTopWindow()->GetRect());
	for (unsigned i = 0; i < m_organfile->GetPanelCount(); i++)
		m_organfile->GetPanel(i)->SetInitialOpenWindow(false);
	GOrgueDocumentBase::SyncState();
}

bool GOrgueDocument::Revert(GOrgueProgressDialog* dlg)
{
	if (m_organfile)
		m_organfile->DeleteSettings();
	Modify(false);
	return Load(dlg, m_organfile->GetOrganInfo());
}

bool GOrgueDocument::Save()
{
	SyncState();
	return m_organfile->Save();
}

bool GOrgueDocument::Export(const wxString& cmb)
{
	SyncState();
	return m_organfile->Export(cmb);
}

void GOrgueDocument::CloseOrgan()
{
	m_listener.SetCallback(NULL);
	m_sound.AssignOrganFile(NULL);
	// m_sound.CloseSound();
	CloseWindows();
	wxTheApp->ProcessPendingEvents();

	m_OrganFileReady = false;
	GOMutexLocker locker(m_lock);
	if (m_organfile)
	{
		delete m_organfile;
		m_organfile = 0;
	}

	wxCommandEvent event(wxEVT_WINTITLE, 0);
	event.SetString(wxEmptyString);
	wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);
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

void GOrgueDocument::ShowOrganDialog()
{
	if (!showWindow(GOrgueDocument::ORGAN_DIALOG, NULL) && m_organfile)
	{
		registerWindow(GOrgueDocument::ORGAN_DIALOG, NULL,
			       new OrganDialog(this, NULL, m_organfile));
	}
}

void GOrgueDocument::ShowMidiList()
{
	if (!showWindow(GOrgueDocument::MIDI_LIST, NULL) && m_organfile)
	{
		registerWindow(GOrgueDocument::MIDI_LIST, NULL,
			       new MIDIList(this, NULL, m_organfile));
	}
}

void GOrgueDocument::ShowMIDIEventDialog(void* element, wxString title, GOrgueMidiReceiverBase* event, GOrgueMidiSender* sender, GOrgueKeyReceiver* key, GOrgueMidiSender* division)
{
	if (!showWindow(GOrgueDocument::MIDI_EVENT, element) && m_organfile)
	{
		MIDIEventDialog* dlg = new MIDIEventDialog (this, NULL, title, m_organfile->GetSettings(), event, sender, key, division);
		dlg->RegisterMIDIListener(m_organfile->GetMidi());
		registerWindow(GOrgueDocument::MIDI_EVENT, element, dlg);
	}
}
