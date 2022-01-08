/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOPanelView.h"

#include <wx/display.h>
#include <wx/frame.h>
#include <wx/icon.h>
#include <wx/image.h>

#include "Images.h"
#include "gui/GOGUIPanel.h"
#include "gui/GOGUIPanelWidget.h"

BEGIN_EVENT_TABLE(GOPanelView, wxScrolledWindow)
EVT_SIZE(GOPanelView::OnSize)
END_EVENT_TABLE()

GOPanelView *
GOPanelView::createWithFrame(GODocumentBase *doc, GOGUIPanel *panel) {
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
  wxIcon icon;
  icon.CopyFromBitmap(GetImage_GOIcon());
  frame->SetIcon(icon);
  return new GOPanelView(doc, panel, frame);
}

GOPanelView::GOPanelView(
  GODocumentBase *doc, GOGUIPanel *panel, wxWindow *parent)
  : wxScrolledWindow(parent),
    GOView(doc, parent),
    m_panelwidget(NULL),
    m_panel(panel),
    m_TopWindow(dynamic_cast<wxTopLevelWindow *>(parent)) {
  wxWindow *frame = parent;

  GOGUIPanelWidget *panelwidget = new GOGUIPanelWidget(panel, this);

  // Set maximum size to the size the window would have with scaling 100%
  // However, still within the default maximum limits, whatever those may be
  // Set an initial client size to what it would be with scaling 100%
  frame->SetMaxSize(wxSize(wxDefaultCoord, wxDefaultCoord));
  frame->SetClientSize(panelwidget->GetSize());
  // frame->SetMaxSize(frame->GetSize());

  // Set a minimum size for the window, just some small value
  wxSize minsize(100, 100);
  frame->SetMinClientSize(minsize);

  // Set a black background color (especially useful if user switches to full
  // screen)
  frame->SetBackgroundColour(*wxBLACK);

  // Get the position and size of the window as saved by the user previously,
  // or otherwise its default values
  wxRect const savedRect = panel->GetWindowRect();

  // If both width and height are set, set position and size of the window
  // E.g. in case of corrupted preferences, this might not be the case
  if (savedRect.GetWidth() && savedRect.GetHeight())
    frame->SetSize(savedRect);

  // However, even if this worked, we cannot be sure that the window is now
  // fully within the client area of an existing display.
  // For example, the user may have disconnected the display, or lowered its
  // resolution. So, we need to get the client area of the desired display, or
  // if it does not exist anymore, the client area of the default display
  int const savedDisplayNum = panel->GetDisplayNum();
  int nr = savedDisplayNum >= 0 && savedDisplayNum < (int)wxDisplay::GetCount()
    ? savedDisplayNum
    : wxDisplay::GetFromWindow(frame);
  wxDisplay display(nr != wxNOT_FOUND ? nr : 0);
  wxRect max = display.GetClientArea();
  // If our current window is within this area, all is fine
  wxRect current = frame->GetRect();
  if (!max.Contains(current)) {
    // Otherwise, check and correct width and height,
    // and place the frame at the center of the Client Area of the display
    if (current.GetWidth() > max.GetWidth())
      current.SetWidth(max.GetWidth());
    if (current.GetHeight() > max.GetHeight())
      current.SetHeight(max.GetHeight());
    frame->SetSize(current.CenterIn(max, wxBOTH));
  }
  if (m_TopWindow && panel->IsMaximized())
    m_TopWindow->Maximize(true);

  m_panelwidget = panelwidget;

  // At this point, the new window may fill the whole display and still not be
  // large enough to show all contents. So, before showing anything, lets see
  // what scaling is needed to fit the content completely to the window.
  wxSize scaledsize = m_panelwidget->UpdateSize(frame->GetClientSize());

  frame->Show();
  frame->Update();

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

GOPanelView::~GOPanelView() {
  if (m_panel)
    m_panel->SetView(NULL);
}

void GOPanelView::RemoveView() {
  if (m_panel)
    m_panel->SetView(NULL);
  m_panel = NULL;
  GOView::RemoveView();
}

void GOPanelView::AddEvent(GOGUIControl *control) {
  wxCommandEvent event(wxEVT_GOCONTROL, 0);
  event.SetClientData(control);
  m_panelwidget->GetEventHandler()->AddPendingEvent(event);
}

void GOPanelView::SyncState() {
  m_panel->SetWindowRect(GetParent()->GetRect());

  if (m_TopWindow) {
    bool isMaximized = m_TopWindow->IsMaximized();

    m_panel->SetMaximized(isMaximized);

    // calculate the best display num
    int displayNum = wxDisplay::GetFromWindow(m_TopWindow);

    if (displayNum == wxNOT_FOUND)
      // actually wxNOT_FOUND == -1, but we don't want to depend on it
      displayNum = -1;
    else if (!isMaximized) {
      // check that the window fits the display
      // we do not check it for maximized window because their decoration may be
      // outside the display
      wxDisplay const display(displayNum);
      wxRect const clientArea = display.GetClientArea();

      if (!clientArea.Contains(m_TopWindow->GetRect()))
        // do not store the dispplay num if the window does not fit it
        displayNum = -1;
    }
    m_panel->SetDisplayNum(displayNum);
  }
  m_panel->SetInitialOpenWindow(true);
}

bool GOPanelView::Destroy() {
  if (m_panel)
    m_panel->SetView(NULL);
  return wxScrolledWindow::Destroy();
}

void GOPanelView::Raise() {
  wxScrolledWindow::Raise();
  m_panelwidget->CallAfter(&GOGUIPanelWidget::Focus);
}

void GOPanelView::OnSize(wxSizeEvent &event) {
  if (m_panelwidget) {
    wxSize max = event.GetSize();
    max = m_panelwidget->UpdateSize(max);
    wxSize scaledsize = max;
    this->SetVirtualSize(max);

    int x, xu, y, yu;
    GetScrollPixelsPerUnit(&xu, &yu);

    GetViewStart(&x, &y);

    if (xu && x * xu + max.GetWidth() > event.GetSize().GetWidth())
      x = (event.GetSize().GetWidth() - max.GetWidth()) / xu;
    if (yu && y * yu + max.GetHeight() > event.GetSize().GetHeight())
      y = (event.GetSize().GetHeight() - max.GetHeight()) / yu;
    this->Scroll(x, y);

    // Reset the position of the panel, before centering it
    // (some wrong value from previous centering may remain otherwise)
    m_panelwidget->SetPosition(wxPoint(0, 0));
    // In a direction where the actual window size is larger than the
    // size of the panel (=scaledsize), the panel needs to be centered
    wxSize actualsize = GetParent()->GetClientSize();
    if (actualsize.GetWidth() > scaledsize.GetWidth())
      m_panelwidget->CentreOnParent(wxHORIZONTAL);
    if (actualsize.GetHeight() > scaledsize.GetHeight())
      m_panelwidget->CentreOnParent(wxVERTICAL);
  }
  event.Skip();
}
