/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOEVENT_H
#define GOEVENT_H

#include <wx/event.h>

DECLARE_LOCAL_EVENT_TYPE(wxEVT_METERS, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_LOADFILE, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_SETVALUE, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_WINTITLE, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_SHOWMSG, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_RENAMEFILE, -1)

class wxMsgBoxEvent : public wxEvent {
private:
  wxString m_Title;
  wxString m_Text;
  long m_Style;

public:
  wxMsgBoxEvent(
    const wxString &title = wxEmptyString,
    const wxString &text = wxEmptyString,
    long style = 0);
  wxMsgBoxEvent(const wxMsgBoxEvent &e);

  const wxString &getTitle();
  const wxString &getText();
  long getStyle();

  wxEvent *Clone() const;
  DECLARE_DYNAMIC_CLASS(wxMsgBoxEvent)
};

class wxRenameFileEvent : public wxEvent {
private:
  wxString m_Filename;
  wxString m_Directory;
  wxString m_Filter;

public:
  wxRenameFileEvent(
    const wxString &filename = wxEmptyString,
    const wxString &directory = wxEmptyString,
    const wxString &filter = wxEmptyString);
  wxRenameFileEvent(const wxRenameFileEvent &e);

  const wxString &getFilename();
  const wxString &getDirectory();
  const wxString &getFilter();

  wxEvent *Clone() const;
  DECLARE_DYNAMIC_CLASS(wxRenameFileEvent)
};

typedef void (wxEvtHandler::*wxMsgBoxFunction)(wxMsgBoxEvent &);

typedef void (wxEvtHandler::*wxRenameFileFunction)(wxRenameFileEvent &);

#define EVT_MSGBOX(fn)                                                         \
  DECLARE_EVENT_TABLE_ENTRY(                                                   \
    wxEVT_SHOWMSG,                                                             \
    wxID_ANY,                                                                  \
    wxID_ANY,                                                                  \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(                 \
      wxMsgBoxFunction, &fn),                                                  \
    (wxObject *)NULL),

#define EVT_RENAMEFILE(fn)                                                     \
  DECLARE_EVENT_TABLE_ENTRY(                                                   \
    wxEVT_RENAMEFILE,                                                          \
    wxID_ANY,                                                                  \
    wxID_ANY,                                                                  \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(                 \
      wxRenameFileFunction, &fn),                                              \
    (wxObject *)NULL),

void GOMessageBox(
  const wxString &text,
  const wxString title,
  long style,
  wxWindow *parent = NULL);
void GOAskRenameFile(
  const wxString &file, const wxString directory, const wxString &filter);

#endif
