/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSplash.h"

#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/frame.h>
#include <wx/image.h>

#include "Images.h"
#include "go_defs.h"

class GOSplashBitmap : public wxControl {
private:
  wxBitmap m_Bitmap;

  void OnPaint(wxPaintEvent &event);
  void OnClick(wxMouseEvent &event);
  void OnKey(wxKeyEvent &event);

public:
  GOSplashBitmap(wxWindow *parent, wxWindowID id, wxBitmap &bitmap);

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(GOSplashBitmap, wxControl)
EVT_PAINT(GOSplashBitmap::OnPaint)
EVT_LEFT_DOWN(GOSplashBitmap::OnClick)
EVT_KEY_DOWN(GOSplashBitmap::OnKey)
END_EVENT_TABLE()

GOSplashBitmap::GOSplashBitmap(
  wxWindow *parent, wxWindowID id, wxBitmap &bitmap)
  : wxControl(parent, id, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE),
    m_Bitmap(bitmap) {}

void GOSplashBitmap::OnPaint(wxPaintEvent &event) {
  wxPaintDC dc(this);
  dc.DrawBitmap(m_Bitmap, 0, 0);
}

void GOSplashBitmap::OnClick(wxMouseEvent &event) { GetParent()->Close(true); }

void GOSplashBitmap::OnKey(wxKeyEvent &event) { GetParent()->Close(true); }

#define GO_SPLASH_TIMER_ID (9999)
#define GO_SPLASH_TIMEOUT_LENGTH (3000)

BEGIN_EVENT_TABLE(GOSplash, wxDialog)
EVT_TIMER(GO_SPLASH_TIMER_ID, GOSplash::OnNotify)
END_EVENT_TABLE()

GOSplash::GOSplash(bool has_timeout, wxWindow *parent, wxWindowID id)
  : wxDialog(
    parent,
    id,
    wxEmptyString,
    wxPoint(0, 0),
    wxSize(100, 100),
    wxBORDER_NONE | wxFRAME_NO_TASKBAR | wxSTAY_ON_TOP),
    GODialogCloser(this),
    m_Timer(this, GO_SPLASH_TIMER_ID),
    m_hasTimeout(has_timeout) {
  SetExtraStyle(GetExtraStyle() | wxWS_EX_TRANSIENT);
  wxBitmap bitmap = GetImage_Splash();
  DrawText(bitmap);
  m_Image = new GOSplashBitmap(this, wxID_ANY, bitmap);
  SetClientSize(bitmap.GetWidth(), bitmap.GetHeight());
  CentreOnScreen();
}

void GOSplash::DrawText(wxBitmap &bitmap) {
  wxMemoryDC dc(bitmap);
  wxFont font;

#ifdef ASIO_INCLUDED
  /* Set ASIO license image */
  wxImage asio = GetImage_ASIO();
  wxBitmap m_asio = (wxBitmap)asio.Scale(
    asio.GetWidth() * 0.1, asio.GetHeight() * 0.1, wxIMAGE_QUALITY_HIGH);
  dc.DrawBitmap(m_asio, 70, 62);
#endif

  font = *wxNORMAL_FONT;
  font.SetPointSize(14);
  font.SetWeight(wxFONTWEIGHT_BOLD);
  dc.SetFont(font);
  dc.DrawLabel(
    wxString::Format(_("GrandOrgue %s"), wxT(APP_VERSION)),
    wxRect(60, 0, 370, 22),
    wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);

  font = *wxNORMAL_FONT;
  font.SetWeight(wxFONTWEIGHT_BOLD);
  font.SetPointSize(10);
  dc.SetFont(font);
  dc.DrawLabel(
    _("virtual pipe organ simulator"),
    wxRect(60, 22, 370, 20),
    wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);

  font = *wxNORMAL_FONT;
  font.SetPointSize(10);
  font.SetWeight(wxFONTWEIGHT_NORMAL);
  dc.SetFont(font);
  dc.DrawLabel(
    _("licensed under the GNU GPLv2"),
    wxRect(60, 42, 370, 20),
    wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);

  font = *wxNORMAL_FONT;
  font.SetWeight(wxFONTWEIGHT_NORMAL);
  font.SetPointSize(7);
  dc.SetFont(font);
  wxString msg
    = _("Copyright 2006 Milan Digital Audio LLC\nCopyright 2009-2024 "
        "GrandOrgue contributors\n\nThis software comes with no warranty");
  dc.DrawLabel(
    msg,
    wxRect(60, 72, 370, 40),
    wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
#ifdef ASIO_INCLUDED
  font = *wxNORMAL_FONT;
  font.SetWeight(wxFONTWEIGHT_NORMAL);
  font.SetPointSize(7);
  dc.SetFont(font);
  msg = _("ASIO Interface Technology by Steinberg Media Technologies GmbH,\nby "
          "use of the Steinberg ASIO SDK, Version 2.3.3.\nASIO is a trademark "
          "and software of Steinberg Media Technologies GmbH.");
  dc.DrawLabel(
    msg,
    wxRect(60, 122, 370, 40),
    wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
#endif
}

GOSplash::~GOSplash() { m_Timer.Stop(); }

void GOSplash::OnShow() {
  GODialogCloser::OnShow();
  if (m_hasTimeout)
    m_Timer.Start(GO_SPLASH_TIMEOUT_LENGTH, true);
  m_Image->SetFocus();
  Update();
}

void GOSplash::OnNotify(wxTimerEvent &WXUNUSED(event)) { Close(true); }

void GOSplash::OnCloseWindow(wxCloseEvent &event) {
  m_Timer.Stop();
  GODialogCloser::OnCloseWindow(event);
}

void GOSplash::DoSplash(bool hasTimeout, wxWindow *parent) {
  GOSplash *const splash = new GOSplash(hasTimeout, parent, wxID_ANY);

  if (hasTimeout)
    splash->ShowAdvanced(true);
  else
    splash->ShowModalAdvanced(true);
}
