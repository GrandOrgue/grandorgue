/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGUIPanelView.h"

#include <wx/display.h>
#include <wx/frame.h>
#include <wx/image.h>

#include "primitives/go_gui_utils.h"

#include "GOGUIPanel.h"
#include "GOGUIPanelWidget.h"

BEGIN_EVENT_TABLE(GOGUIPanelView, wxScrolledWindow)
EVT_SIZE(GOGUIPanelView::OnSize)
END_EVENT_TABLE()

GOGUIPanelView *GOGUIPanelView::createWithFrame(
  GODocumentBase *doc, GOGUIPanel *panel) {
  wxFrame *frame = new wxFrame(
    NULL,
    -1,
    panel->GetName(),
    wxDefaultPosition,
    wxDefaultSize,
    wxMINIMIZE_BOX | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX
      | wxCLIP_CHILDREN | wxFULL_REPAINT_ON_RESIZE | wxMAXIMIZE_BOX);
  // By adding wxMAXIMIZE_BOX, maximize (on Windows, Linux) will work, but only
  // fill the screen if it is smaller than the maximum allowed frame size.
  // We probably should change this to add black borders to it. Also, for these
  // platforms a full-screen mode would be beneficial (on MacOS is already
  // works).
  frame->SetIcon(get_go_icon());
  return new GOGUIPanelView(doc, panel, frame);
}

GOGUIPanelView::GOGUIPanelView(
  GODocumentBase *doc, GOGUIPanel *panel, wxTopLevelWindow *topWindow)
  : wxScrolledWindow(topWindow),
    GOView(doc, topWindow),
    m_panelwidget(new GOGUIPanelWidget(panel, this)),
    m_panel(panel),
    m_TopWindow(topWindow) {
  // Set a black background color (especially useful if user switches to full
  // screen)
  topWindow->SetBackgroundColour(*wxBLACK);

  // Set maximum size to the size the window would have with scaling 100%
  // However, still within the default maximum limits, whatever those may be
  // Set an initial client size to what it would be with scaling 100%
  topWindow->SetMaxSize(wxSize(wxDefaultCoord, wxDefaultCoord));
  topWindow->SetClientSize(m_panelwidget->GetSize());
  // frame->SetMaxSize(frame->GetSize());

  // Set a minimum size for the window, just some small value
  topWindow->SetMinClientSize(wxSize(100, 100));

  // set the windows position adn size as it has been saved
  panel->ApplySizeInfo(*topWindow);

  // At this point, the new window may fill the whole display and still not be
  // large enough to show all contents. So, before showing anything, lets see
  // what scaling is needed to fit the content completely to the window.
  wxSize scaledsize = m_panelwidget->UpdateSize(topWindow->GetClientSize());

  topWindow->Show();
  topWindow->Update();

  // Ensure that scrollbars will appear when they are needed
  // Current design aims for avoiding this as much as possible
  this->SetScrollRate(5, 5);
  this->SetVirtualSize(scaledsize);

  // Set position of panelwidget to initial value
  m_panelwidget->SetPosition(wxPoint(0, 0));
  // In a direction where the actual window size is larger than the
  // size of the panel (=scaledsize), the panel needs to be centered
  wxSize actualsize = GetParent()->GetClientSize();
  if (actualsize.GetWidth() > scaledsize.GetWidth())
    m_panelwidget->CentreOnParent(wxHORIZONTAL);
  if (actualsize.GetHeight() > scaledsize.GetHeight())
    m_panelwidget->CentreOnParent(wxVERTICAL);

  m_panel->SetView(this);
}

GOGUIPanelView::~GOGUIPanelView() {
  if (m_panel)
    m_panel->SetView(NULL);
}

void GOGUIPanelView::RemoveView() {
  if (m_panel)
    m_panel->SetView(NULL);
  m_panel = NULL;
  GOView::RemoveView();
}

void GOGUIPanelView::AddEvent(GOGUIControl *control) {
  wxCommandEvent event(wxEVT_GOCONTROL, 0);
  event.SetClientData(control);
  m_panelwidget->GetEventHandler()->AddPendingEvent(event);
}

void GOGUIPanelView::SyncState() {
  if (m_TopWindow)
    m_panel->CaptureSizeInfo(*m_TopWindow);
}

bool GOGUIPanelView::Destroy() {
  if (m_panel)
    m_panel->SetView(NULL);
  return wxScrolledWindow::Destroy();
}

void GOGUIPanelView::Raise() {
  wxScrolledWindow::Raise();
  m_panelwidget->CallAfter(&GOGUIPanelWidget::Focus);
}

void GOGUIPanelView::OnSize(wxSizeEvent &event) {
  if (m_panelwidget) {
    // Scale out the panel according the new size

    const wxSize newSize = event.GetSize();
    const wxSize maxSize = m_panelwidget->UpdateSize(newSize);

    this->SetVirtualSize(maxSize);

    int x, xu, y, yu;
    GetScrollPixelsPerUnit(&xu, &yu);

    GetViewStart(&x, &y);

    if (xu && x * xu + maxSize.GetWidth() > newSize.GetWidth())
      x = (newSize.GetWidth() - maxSize.GetWidth()) / xu;
    if (yu && y * yu + maxSize.GetHeight() > newSize.GetHeight())
      y = (newSize.GetHeight() - maxSize.GetHeight()) / yu;
    this->Scroll(x, y);

    // Reset the position of the panel, before centering it
    // (some wrong value from previous centering may remain otherwise)
    m_panelwidget->SetPosition(wxPoint(0, 0));

    // In a direction where the actual window size is larger than the
    // size of the panel (=scaledsize), the panel needs to be centered
    wxSize actualsize = GetParent()->GetClientSize();

    if (actualsize.GetWidth() > maxSize.GetWidth())
      m_panelwidget->CentreOnParent(wxHORIZONTAL);
    if (actualsize.GetHeight() > maxSize.GetHeight())
      m_panelwidget->CentreOnParent(wxVERTICAL);
  }
  event.Skip();
}
