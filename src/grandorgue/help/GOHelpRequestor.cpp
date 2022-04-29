#include "GOHelpRequestor.h"

#include <wx/event.h>
#include <wx/log.h>

#include "GOHelpController.h"
#include "GOStdPath.h"

wxDECLARE_EVENT(wxEVT_SHOWHELP, wxCommandEvent);

wxDEFINE_EVENT(wxEVT_SHOWHELP, wxCommandEvent);

wxEvtHandler *GOHelpRequestor::p_EventHandler = NULL;

void GOHelpRequestor::OnShowHelp(wxCommandEvent &event) {
  m_HelpController->Display(event.GetString(), (bool)event.GetInt());
}

GOHelpRequestor::GOHelpRequestor(wxEvtHandler *peh) {
  m_HelpController = new GOHelpController(
    wxHF_CONTENTS | wxHF_INDEX | wxHF_SEARCH | wxHF_ICONS_BOOK
    | wxHF_FLAT_TOOLBAR);

  wxString result;
  wxString lang = wxGetLocale()->GetCanonicalName();

  wxString searchpath;
  searchpath.Append(
    GOStdPath::GetResourceDir() + wxFILE_SEP_PATH + wxT("help"));

  if (!wxFindFileInPath(
        &result, searchpath, wxT("GrandOrgue_") + lang + wxT(".htb"))) {
    if (lang.Find(wxT('_')))
      lang = lang.Left(lang.Find(wxT('_')));
    if (!wxFindFileInPath(
          &result, searchpath, wxT("GrandOrgue_") + lang + wxT(".htb"))) {
      if (!wxFindFileInPath(&result, searchpath, wxT("GrandOrgue.htb")))
        result = wxT("GrandOrgue.htb");
    }
  }
  wxLogDebug(
    _("Using helpfile %s (search path: %s)"),
    result.c_str(),
    searchpath.c_str());
  m_HelpController->AddBook(result);
  p_EventHandler = peh;
  p_EventHandler->Bind(wxEVT_SHOWHELP, &GOHelpRequestor::OnShowHelp, this);
}

GOHelpRequestor::~GOHelpRequestor() {
  if (p_EventHandler) {
    p_EventHandler->Unbind(wxEVT_SHOWHELP, &GOHelpRequestor::OnShowHelp, this);
    p_EventHandler = NULL;
  }
  if (m_HelpController) {
    delete m_HelpController;
    m_HelpController = NULL;
  }
}

void GOHelpRequestor::DisplayHelp(const wxString &x, const bool isModal) {
  if (p_EventHandler) {
    wxCommandEvent helpEvt(wxEVT_SHOWHELP, 0);

    helpEvt.SetString(x);
    helpEvt.SetInt(isModal);
    p_EventHandler->AddPendingEvent(helpEvt);
  }
}
