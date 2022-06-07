#ifndef GOHELPREQUESTOR_H
#define GOHELPREQUESTOR_H

class wxCommandEvent;
class wxEvtHandler;
class wxString;

class GOHelpController;

class GOHelpRequestor {
private:
  static wxEvtHandler *p_EventHandler;

  GOHelpController *m_HelpController;

  void OnShowHelp(wxCommandEvent &event);

public:
  GOHelpRequestor(wxEvtHandler *peh);
  ~GOHelpRequestor();

  static void DisplayHelp(const wxString &x, const bool isModal);
};

#endif /* GOHELPREQUESTOR_H */
