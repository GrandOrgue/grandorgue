/*
 * GrandOrgue - a free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "SplashScreen.h"

#include "go_defs.h"
#include "Images.h"
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/frame.h>
#include <wx/image.h>

BEGIN_EVENT_TABLE(GOSplashBitmap, wxControl)
	EVT_PAINT(GOSplashBitmap::OnPaint)
	EVT_LEFT_DOWN(GOSplashBitmap::OnClick)
	EVT_KEY_DOWN(GOSplashBitmap::OnKey)
END_EVENT_TABLE()

<<<<<<< HEAD

GOSplashBitmap::GOSplashBitmap(wxWindow *parent, wxWindowID id, wxBitmap& bitmap) :
=======
GOrgueSplashBitmap::GOrgueSplashBitmap(wxWindow *parent, wxWindowID id, wxBitmap& bitmap) :
>>>>>>> 0716053f0 ([IMP] Add ASIO Image to Splash Screen)
wxControl(parent, id, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE),
	m_Bitmap(bitmap)
{
}

void GOSplashBitmap::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);
	dc.DrawBitmap(m_Bitmap, 0, 0);
}

void GOSplashBitmap::OnClick(wxMouseEvent& event)
{
	GetParent()->Close(true);
}

void GOSplashBitmap::OnKey(wxKeyEvent& event)
{
	GetParent()->Close(true);
}

#define GO_SPLASH_TIMER_ID       (9999)
#define GO_SPLASH_TIMEOUT_LENGTH (3000)

BEGIN_EVENT_TABLE(GOSplash, wxDialog)
	EVT_SHOW(GOSplash::OnShowWindow)
	EVT_TIMER(GO_SPLASH_TIMER_ID, GOSplash::OnNotify)
	EVT_CLOSE(GOSplash::OnCloseWindow)
END_EVENT_TABLE()

GOSplash::GOSplash(bool has_timeout, wxWindow *parent, wxWindowID id) :
	wxDialog(parent, id, wxEmptyString, wxPoint(0, 0), wxSize(100, 100), wxBORDER_NONE | wxFRAME_NO_TASKBAR | wxSTAY_ON_TOP),
	m_Timer(this, GO_SPLASH_TIMER_ID), m_hasTimeout(has_timeout)
{
	SetExtraStyle(GetExtraStyle() | wxWS_EX_TRANSIENT);
	wxBitmap bitmap = GetImage_Splash();
	DrawText(bitmap);
	m_Image = new GOSplashBitmap(this, wxID_ANY, bitmap);
	SetClientSize(bitmap.GetWidth(), bitmap.GetHeight());
	CentreOnScreen();
}

void GOSplash::DrawText(wxBitmap& bitmap)
{
	wxMemoryDC dc(bitmap);
	wxFont font;

    /* Set ASIO license image */
    wxImage asio = GetImage_ASIO();
    wxBitmap m_asio = (wxBitmap)asio.Scale(asio.GetWidth() * 0.1, asio.GetHeight() * 0.1, wxIMAGE_QUALITY_HIGH);
    dc.DrawBitmap(m_asio, 70, 10);

	font = *wxNORMAL_FONT;
	font.SetPointSize(14);
	font.SetWeight(wxFONTWEIGHT_BOLD);
	dc.SetFont(font);
	dc.DrawLabel(wxString::Format(_("GrandOrgue %s"), wxT(APP_VERSION)), wxRect(60, 0, 370, 22), wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);

	font = *wxNORMAL_FONT;
	font.SetWeight(wxFONTWEIGHT_BOLD);
	font.SetPointSize(10);
	dc.SetFont(font);
	dc.DrawLabel(_("virtual pipe organ simulator"), wxRect(60, 22, 370, 20), wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);

	font = *wxNORMAL_FONT;
	font.SetPointSize(10);
	font.SetWeight(wxFONTWEIGHT_NORMAL);
	dc.SetFont(font);
	dc.DrawLabel(_("licensed under the GNU GPLv2"), wxRect(60, 42, 370, 20), wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);

	font = *wxNORMAL_FONT;
	font.SetWeight(wxFONTWEIGHT_NORMAL);
	font.SetPointSize(7);
	dc.SetFont(font);
	wxString msg = _("Copyright 2006 Milan Digital Audio LLC\nCopyright 2009-2021 GrandOrgue contributors\n\nThis software comes with no warranty.\n\nASIO Interface Technology by Steinberg Media Technologies GmbH,\nby use of the Steinberg ASIO SDK, Version 2.2.\nASIO is a trademark and software of Steinberg Media Technologies GmbH.");
	dc.DrawLabel(msg, wxRect(60, 62, 370, 100), wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
}

GOSplash::~GOSplash()
{
	m_Timer.Stop();
}

void GOSplash::OnShowWindow(wxShowEvent &event)
{
  if (m_hasTimeout)
    m_Timer.Start(GO_SPLASH_TIMEOUT_LENGTH, true);
  m_Image->SetFocus();
  Update();
}

void GOSplash::OnNotify(wxTimerEvent& WXUNUSED(event))
{
  Close(true);
}

void GOSplash::OnCloseWindow(wxCloseEvent& WXUNUSED(event))
{
  m_Timer.Stop();
  if (m_hasTimeout)
    Destroy();
  else
    EndModal(wxID_OK);
}

void GOSplash::DoSplash(bool hasTimeout, wxWindow *parent) {
  GOSplash* const splash = new GOSplash(hasTimeout, parent, wxID_ANY);
  
  if (hasTimeout)
    splash->Show();
  else
  {
    splash->ShowModal();
    splash->Destroy();
  }
}
