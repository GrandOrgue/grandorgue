/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GODocument.h"

#include <wx/app.h>

#include "config/GOConfig.h"
#include "dialogs/GOMidiListDialog.h"
#include "dialogs/GOOrganSettingsDialog.h"
#include "dialogs/midi-event/GOMidiEventDialog.h"
#include "document-base/GOView.h"
#include "gui/GOGUIPanel.h"
#include "gui/GOGUIPanelView.h"
#include "midi/GOMidiEvent.h"
#include "size/GOResizable.h"
#include "sound/GOSound.h"
#include "threading/GOMutexLocker.h"

#include "GOEvent.h"
#include "GOFrame.h"
#include "GOOrgan.h"
#include "GOOrganController.h"
#include "go_ids.h"

GODocument::GODocument(GOResizable *pMainWindow, GOSound *sound)
  : p_MainWindow(pMainWindow),
    m_sound(*sound),
    m_OrganFileReady(false),
    m_OrganController(NULL),
    m_listener() {
  m_listener.Register(&m_sound.GetMidi());
}

GODocument::~GODocument() {
  m_listener.SetCallback(NULL);
  CloseOrgan();
}

bool GODocument::IsModified() const {
  return m_OrganController && m_OrganController->IsOrganModified();
}

bool GODocument::LoadOrgan(
  GOProgressDialog *dlg,
  const GOOrgan &organ,
  const wxString &cmb,
  bool isGuiOnly) {
  wxBusyCursor busy;
  GOConfig &cfg = m_sound.GetSettings();

  CloseOrgan();
  m_OrganController = new GOOrganController(cfg, this);
  wxString error = m_OrganController->Load(dlg, organ, cmb, isGuiOnly);
  if (!error.IsEmpty()) {
    if (error != wxT("!")) {
      wxLogError(wxT("%s\n"), error.c_str());
      GOMessageBox(error, _("Load error"), wxOK | wxICON_ERROR, NULL);
    }
    CloseOrgan();
    return false;
  }
  cfg.AddOrgan(m_OrganController->GetOrganInfo());
  cfg.Flush();
  {
    wxCommandEvent event(wxEVT_SETVALUE, ID_METER_AUDIO_SPIN);
    event.SetInt(m_OrganController->GetVolume());
    wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);

    m_sound.GetEngine().SetVolume(m_OrganController->GetVolume());
  }

  wxCommandEvent event(wxEVT_WINTITLE, 0);
  event.SetString(m_OrganController->GetChurchName());
  wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);

  for (unsigned i = 0; i < m_OrganController->GetPanelCount(); i++)
    if (m_OrganController->GetPanel(i)->InitialOpenWindow())
      ShowPanel(i);

  const GOLogicalRect &mRect(
    m_OrganController->GetMainWindowData()->GetWindowRect());

  if (!mRect.IsEmpty() && p_MainWindow)
    p_MainWindow->SetPosSize(mRect);

  m_sound.AssignOrganFile(m_OrganController);
  m_OrganFileReady = true;
  m_listener.SetCallback(this);
  if (!cmb.IsEmpty())
    m_OrganController->SetOrganModified();

  /* The sound was open on GOFrame::Init.
   * m_sound.AssignOrganFile made all necessary for the new organController.
   * So the new opening is not necessary
  if (m_sound.OpenSound())
          return false;
   */
  return true;
}

bool GODocument::UpdateCache(GOProgressDialog *dlg, bool compress) {
  if (!m_OrganController)
    return false;
  return m_OrganController->UpdateCache(dlg, compress);
}

void GODocument::ShowPanel(unsigned id) {
  GOGUIPanel *panel = m_OrganController->GetPanel(id);

  if (!showWindow(GODocument::PANEL, panel)) {
    registerWindow(
      GODocument::PANEL, panel, GOGUIPanelView::createWithFrame(this, panel));
  }
}

void GODocument::SyncState() {
  m_OrganController->SetVolume(m_sound.GetEngine().GetVolume());
  if (p_MainWindow)
    m_OrganController->GetMainWindowData()->SetWindowRect(
      p_MainWindow->GetPosSize());
  for (unsigned i = 0; i < m_OrganController->GetPanelCount(); i++)
    m_OrganController->GetPanel(i)->SetInitialOpenWindow(false);
  GODocumentBase::SyncState();
}

bool GODocument::Save() {
  SyncState();
  return m_OrganController->Save();
}

bool GODocument::Export(const wxString &cmb) {
  SyncState();
  return m_OrganController->Export(cmb);
}

void GODocument::CloseOrgan() {
  m_listener.SetCallback(NULL);
  m_sound.AssignOrganFile(NULL);
  // m_sound.CloseSound();
  CloseWindows();
  wxTheApp->ProcessPendingEvents();

  m_OrganFileReady = false;
  GOMutexLocker locker(m_lock);
  if (m_OrganController) {
    delete m_OrganController;
    m_OrganController = 0;
  }

  wxCommandEvent event(wxEVT_WINTITLE, 0);
  event.SetString(wxEmptyString);
  wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);
}

void GODocument::OnMidiEvent(const GOMidiEvent &event) {
  GOMutexLocker locker(m_lock);

  if (!m_OrganFileReady)
    return;

  if (m_OrganController)
    m_OrganController->ProcessMidi(event);
}

void GODocument::ShowOrganSettingsDialog() {
  if (!showWindow(GODocument::ORGAN_DIALOG, NULL) && m_OrganController) {
    registerWindow(
      GODocument::ORGAN_DIALOG,
      NULL,
      new GOOrganSettingsDialog(this, NULL, m_OrganController));
  }
}

void GODocument::ShowMidiList() {
  if (!showWindow(GODocument::MIDI_LIST, NULL) && m_OrganController) {
    registerWindow(
      GODocument::MIDI_LIST,
      NULL,
      new GOMidiListDialog(
        this,
        NULL,
        m_OrganController->GetConfig().m_DialogSizes,
        m_OrganController->GetMidiConfigurators()));
  }
}

void GODocument::ShowMIDIEventDialog(
  void *element,
  const wxString &title,
  const wxString &dialogSelector,
  GOMidiReceiverBase *event,
  GOMidiSender *sender,
  GOMidiShortcutReceiver *key,
  GOMidiSender *division) {
  if (!showWindow(GODocument::MIDI_EVENT, element) && m_OrganController) {
    GOMidiEventDialog *dlg = new GOMidiEventDialog(
      this,
      NULL,
      title,
      m_OrganController->GetSettings(),
      dialogSelector,
      event,
      sender,
      key,
      division);
    dlg->RegisterMIDIListener(m_OrganController->GetMidi());
    dlg->SetModificationListener(m_OrganController);
    registerWindow(GODocument::MIDI_EVENT, element, dlg);
  }
}
