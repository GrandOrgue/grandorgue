/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GODocument.h"

#include <wx/app.h>

#include "config/GOConfig.h"
#include "dialogs/GOMidiListDialog.h"
#include "dialogs/GOOrganDialog.h"
#include "dialogs/midi-event/GOMidiEventDialog.h"
#include "document-base/GOView.h"
#include "gui/GOGUIPanel.h"
#include "midi/GOMidiEvent.h"
#include "sound/GOSound.h"
#include "threading/GOMutexLocker.h"

#include "GODefinitionFile.h"
#include "GOEvent.h"
#include "GOFrame.h"
#include "GOOrgan.h"
#include "GOPanelView.h"
#include "GOResizable.h"
#include "go_ids.h"

GODocument::GODocument(GOResizable *pMainWindow, GOSound *sound)
  : p_MainWindow(pMainWindow),
    m_sound(*sound),
    m_OrganFileReady(false),
    m_organfile(NULL),
    m_listener(),
    m_modified(false) {
  m_listener.Register(&m_sound.GetMidi());
}

GODocument::~GODocument() {
  m_listener.SetCallback(NULL);
  CloseOrgan();
}

bool GODocument::IsModified() { return m_modified; }

void GODocument::Modify(bool modified) { m_modified = modified; }

bool GODocument::Load(GOProgressDialog *dlg, const GOOrgan &organ) {
  return Import(dlg, organ, wxEmptyString);
}

bool GODocument::Import(
  GOProgressDialog *dlg, const GOOrgan &organ, const wxString &cmb) {
  wxBusyCursor busy;
  GOConfig &cfg = m_sound.GetSettings();

  CloseOrgan();
  Modify(false);
  m_organfile = new GODefinitionFile(this, cfg);
  wxString error = m_organfile->Load(dlg, organ, cmb);
  if (!error.IsEmpty()) {
    if (error != wxT("!")) {
      wxLogError(wxT("%s\n"), error.c_str());
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

  const unsigned releaseTail = m_organfile->GetReleaseTail();

  cfg.ReleaseLength(releaseTail);
  m_sound.GetEngine().SetReleaseLength(releaseTail);
  m_sound.GetSettings().Flush();

  wxCommandEvent event(wxEVT_WINTITLE, 0);
  event.SetString(m_organfile->GetChurchName());
  wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);

  for (unsigned i = 0; i < m_organfile->GetPanelCount(); i++)
    if (m_organfile->GetPanel(i)->InitialOpenWindow())
      ShowPanel(i);

  const GOLogicalRect &mRect(m_organfile->GetMainWindowData()->GetWindowRect());

  if (!mRect.IsEmpty() && p_MainWindow)
    p_MainWindow->SetPosSize(mRect);

  m_sound.AssignOrganFile(m_organfile);
  m_OrganFileReady = true;
  m_listener.SetCallback(this);
  Modify(!cmb.IsEmpty());

  /* The sound was open on GOFrame::Init.
   * m_sound.AssignOrganFile made all necessary for the new organfile.
   * So the new opening is not necessary
  if (m_sound.OpenSound())
          return false;
   */
  return true;
}

bool GODocument::ImportCombination(const wxString &cmb) {
  if (!m_organfile)
    return false;
  m_organfile->LoadCombination(cmb);
  m_organfile->Modified();
  return true;
}

bool GODocument::UpdateCache(GOProgressDialog *dlg, bool compress) {
  if (!m_organfile)
    return false;
  return m_organfile->UpdateCache(dlg, compress);
}

void GODocument::ShowPanel(unsigned id) {
  GOGUIPanel *panel = m_organfile->GetPanel(id);

  if (!showWindow(GODocument::PANEL, panel)) {
    registerWindow(
      GODocument::PANEL, panel, GOPanelView::createWithFrame(this, panel));
  }
}

void GODocument::SyncState() {
  m_organfile->SetVolume(m_sound.GetEngine().GetVolume());
  if (p_MainWindow)
    m_organfile->GetMainWindowData()->SetWindowRect(p_MainWindow->GetPosSize());
  for (unsigned i = 0; i < m_organfile->GetPanelCount(); i++)
    m_organfile->GetPanel(i)->SetInitialOpenWindow(false);
  GODocumentBase::SyncState();
}

bool GODocument::Revert(GOProgressDialog *dlg) {
  if (m_organfile)
    m_organfile->DeleteSettings();
  Modify(false);
  return Load(dlg, m_organfile->GetOrganInfo());
}

bool GODocument::Save() {
  SyncState();
  return m_organfile->Save();
}

bool GODocument::Export(const wxString &cmb) {
  SyncState();
  return m_organfile->Export(cmb);
}

void GODocument::CloseOrgan() {
  m_listener.SetCallback(NULL);
  m_sound.AssignOrganFile(NULL);
  // m_sound.CloseSound();
  CloseWindows();
  wxTheApp->ProcessPendingEvents();

  m_OrganFileReady = false;
  GOMutexLocker locker(m_lock);
  if (m_organfile) {
    delete m_organfile;
    m_organfile = 0;
  }

  wxCommandEvent event(wxEVT_WINTITLE, 0);
  event.SetString(wxEmptyString);
  wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);
}

GODefinitionFile *GODocument::GetOrganFile() { return m_organfile; }

void GODocument::OnMidiEvent(const GOMidiEvent &event) {
  GOMutexLocker locker(m_lock);

  if (!m_OrganFileReady)
    return;

  if (m_organfile)
    m_organfile->ProcessMidi(event);
}

void GODocument::ShowOrganDialog() {
  if (!showWindow(GODocument::ORGAN_DIALOG, NULL) && m_organfile) {
    registerWindow(
      GODocument::ORGAN_DIALOG,
      NULL,
      new GOOrganDialog(this, NULL, m_organfile));
  }
}

void GODocument::ShowMidiList() {
  if (!showWindow(GODocument::MIDI_LIST, NULL) && m_organfile) {
    registerWindow(
      GODocument::MIDI_LIST,
      NULL,
      new GOMidiListDialog(this, NULL, m_organfile));
  }
}

void GODocument::ShowMIDIEventDialog(
  void *element,
  wxString title,
  GOMidiReceiverBase *event,
  GOMidiSender *sender,
  GOKeyReceiver *key,
  GOMidiSender *division) {
  if (!showWindow(GODocument::MIDI_EVENT, element) && m_organfile) {
    GOMidiEventDialog *dlg = new GOMidiEventDialog(
      this,
      NULL,
      title,
      m_organfile->GetSettings(),
      event,
      sender,
      key,
      division);
    dlg->RegisterMIDIListener(m_organfile->GetMidi());
    registerWindow(GODocument::MIDI_EVENT, element, dlg);
  }
}
