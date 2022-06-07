#ifndef GOHELPCONTROLLER_H
#define GOHELPCONTROLLER_H

#include <wx/html/helpctrl.h>

/*
 * The standard wxHtmlHelpController can not bring the help above a modal dialog
 * under linux. This class creates wxHtmlHelpFrame with wxTOPLEVEL_EX_DIALOG
 * extra style for being able to do this.
 */
class GOHelpController : public wxHtmlHelpController {
  bool m_IsModal = false;

public:
  GOHelpController(int style) : wxHtmlHelpController(style | wxHF_FRAME) {}

  bool Display(const wxString &x, const bool isModal);

  wxHtmlHelpFrame *CreateHelpFrame(wxHtmlHelpData *data) override;
};

#endif /* GOHELPCONTROLLER_H */
