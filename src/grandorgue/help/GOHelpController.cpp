#include "GOHelpController.h"

#include <wx/log.h>

/*
 * Borrowed from
 * https://github.com/wxWidgets/wxWidgets/blob/master/src/html/helpctrl.cpp
 * with adding SetExtraStyle(wxTOPLEVEL_EX_DIALOG);
 */

wxHtmlHelpFrame *GOHelpController::CreateHelpFrame(wxHtmlHelpData *data) {
  wxHtmlHelpFrame *frame = new wxHtmlHelpFrame(data);
  if (m_IsModal)
    frame->SetExtraStyle(frame->GetExtraStyle() | wxTOPLEVEL_EX_DIALOG);
  frame->SetController(this);
  frame->SetTitleFormat(m_titleFormat);
  frame->Create(
    m_parentWindow,
    -1,
    wxEmptyString,
    m_FrameStyle
#if wxUSE_CONFIG
    ,
    m_Config,
    m_ConfigRoot
#endif // wxUSE_CONFIG
  );
  frame->SetShouldPreventAppExit(m_shouldPreventAppExit);
  m_helpFrame = frame;
  return frame;
};

bool GOHelpController::Display(const wxString &x, const bool isModal) {
  wxHtmlHelpFrame *oldFrame = GetFrame();

  if (oldFrame) {
    wxRect rect(oldFrame->GetScreenRect());

    if (rect.width <= 0 || rect.height <= 0 || isModal != m_IsModal) {
      oldFrame->Close(true);
      oldFrame = NULL;
    }
  }
  if (isModal != m_IsModal)
    m_IsModal = isModal;
  if (oldFrame && oldFrame->IsIconized())
    oldFrame->Iconize(false);
  return wxHtmlHelpController::Display(x);
}
