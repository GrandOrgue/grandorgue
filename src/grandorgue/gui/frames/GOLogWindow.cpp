/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOLogWindow.h"

#include <wx/artprov.h>
#include <wx/clipbrd.h>
#include <wx/dataobj.h>
#include <wx/imaglist.h>
#include <wx/listctrl.h>
#include <wx/menu.h>
#include <wx/thread.h>

DEFINE_LOCAL_EVENT_TYPE(wxEVT_ADD_LOG_MESSAGE)

static constexpr unsigned LOG_MESSAGE_BATCH_SIZE = 100;
static unsigned log_message_count = 0;

BEGIN_EVENT_TABLE(GOLogWindow, wxFrame)
EVT_COMMAND(0, wxEVT_ADD_LOG_MESSAGE, GOLogWindow::OnLog)
EVT_CLOSE(GOLogWindow::OnCloseWindow)
EVT_CONTEXT_MENU(GOLogWindow::OnPopup)
EVT_MENU(wxID_COPY, GOLogWindow::OnCopy)
EVT_MENU(wxID_CLEAR, GOLogWindow::OnClear)
END_EVENT_TABLE()

GOLogWindow::GOLogWindow(
  wxWindow *parent,
  wxWindowID id,
  const wxString &title,
  const wxPoint &pos,
  const wxSize &size,
  long style)
  : wxFrame(parent, id, title, pos, size, style) {
  m_List = new wxListCtrl(
    this,
    wxID_ANY,
    wxDefaultPosition,
    wxDefaultSize,
    wxBORDER_SIMPLE | wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL);
  m_List->InsertColumn(0, wxT("Message"));
  m_List->InsertColumn(1, wxT("Time"));

  const int img_size = 16;
  wxImageList *imgs = new wxImageList(img_size, img_size);
  imgs->Add(wxArtProvider::GetBitmap(
    wxART_ERROR, wxART_MESSAGE_BOX, wxSize(img_size, img_size)));
  imgs->Add(wxArtProvider::GetBitmap(
    wxART_WARNING, wxART_MESSAGE_BOX, wxSize(img_size, img_size)));
  imgs->Add(wxArtProvider::GetBitmap(
    wxART_INFORMATION, wxART_MESSAGE_BOX, wxSize(img_size, img_size)));
  m_List->AssignImageList(imgs, wxIMAGE_LIST_SMALL);

  m_List->SetColumnWidth(0, wxLIST_AUTOSIZE);
  m_List->SetColumnWidth(1, wxLIST_AUTOSIZE);
}

GOLogWindow::~GOLogWindow() {}

void GOLogWindow::OnCopy(wxCommandEvent &event) {
  wxString text;
  for (long i = m_List->GetItemCount() - 1; i >= 0; i--) {
    wxListItem i1, i2;
    i1.SetId(i);
    i1.SetColumn(1);
    i2.SetId(i);
    i2.SetColumn(0);
    i1.SetMask(wxLIST_MASK_TEXT);
    i2.SetMask(wxLIST_MASK_TEXT);
    m_List->GetItem(i1);
    m_List->GetItem(i2);
    text += wxString::Format(
      _("%s: %s\n"), i1.GetText().c_str(), i2.GetText().c_str());
  }

  if (wxTheClipboard->Open()) {
    wxTheClipboard->SetData(new wxTextDataObject(text));
    wxTheClipboard->Close();
  }
}

void GOLogWindow::OnPopup(wxContextMenuEvent &event) {
  wxMenu popup;
  popup.Append(wxID_CLEAR, _("&Clear"), wxEmptyString, wxITEM_NORMAL);
  popup.AppendSeparator();
  popup.Append(wxID_COPY, _("C&opy"), wxEmptyString, wxITEM_NORMAL);
  PopupMenu(&popup);
}

void GOLogWindow::OnClear(wxCommandEvent &event) { m_List->DeleteAllItems(); }

void GOLogWindow::OnLog(wxCommandEvent &event) {
  wxDateTime time((time_t)event.GetTimestamp());
  wxString timestr = time.Format();

  long line = m_List->InsertItem(0, event.GetString(), event.GetInt());
  m_List->SetItem(line, 1, timestr);

  m_List->SetColumnWidth(0, wxLIST_AUTOSIZE);
  m_List->SetColumnWidth(1, wxLIST_AUTOSIZE);

  if (!IsShown())
    Show();
}

void GOLogWindow::OnCloseWindow(wxCloseEvent &event) {
  Show(false);
  m_List->DeleteAllItems();
}

bool GOLogWindow::HasPendingLogEvents() {
  wxCriticalSectionLocker locker(m_pendingEventsLock);

  return m_pendingEvents && !m_pendingEvents->IsEmpty();
}

void GOLogWindow::LogMsg(
  wxLogLevel level, const wxString &msg, time_t timestamp) {
  int l;
  switch (level) {
  case wxLOG_FatalError:
  case wxLOG_Error:
    l = 0;
    break;
  case wxLOG_Warning:
    l = 1;
    break;
  default:
    l = 2;
  }
  wxCommandEvent e(wxEVT_ADD_LOG_MESSAGE, 0);
  e.SetString(msg);
  e.SetInt(l);
  e.SetTimestamp(timestamp);
  GetEventHandler()->AddPendingEvent(e);
  log_message_count++;
  /* A long operation running in the GUI thread may emit thousands of messages
     without returning to the event loop, so the pending event queue has to be
     drained from time to time.
     Only this window's own queue is processed. A full wxApp::Yield() would
     dispatch any other pending event as well, and log messages are emitted
     from places that must not be re-entered: it used to dispatch wxEVT_METERS
     into GOSoundOrganEngine::GetMeterInfo() and a pending toolbar click into
     GOAppWindow::OnAudioPanic() while GOSoundOrganEngine::BuildEngine() was
     holding m_LifecycleMutex, which deadlocked the application (issue #2606).
     ProcessPendingEvents() delivers only a single event per call, so the
     whole accumulated batch is drained by looping until the queue is
     actually empty.
   */
  if ((log_message_count % LOG_MESSAGE_BATCH_SIZE) == 0)
    while (HasPendingLogEvents())
      GetEventHandler()->ProcessPendingEvents();
}
