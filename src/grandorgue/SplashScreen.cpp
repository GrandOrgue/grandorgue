/*
 * GrandOrgue - a free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2011 GrandOrgue contributors (see AUTHORS)
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
 * MA 02111-1307, USA.
 */

#include <wx/splash.h>
#include "SplashScreen.h"

#define wxSPLASH_TIMER_ID 9999

BEGIN_EVENT_TABLE(wxSplashScreenModal, wxDialog)
    EVT_TIMER(wxSPLASH_TIMER_ID, wxSplashScreenModal::OnNotify)
    EVT_CLOSE(wxSplashScreenModal::OnCloseWindow)
END_EVENT_TABLE()

wxSplashScreenModal::wxSplashScreenModal(const wxBitmap& bitmap, long splashStyle, int milliseconds, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style):
    wxDialog(parent, id, wxEmptyString, wxPoint(0,0), wxSize(100, 100), style)
{

    m_window = NULL;
    m_splashStyle = splashStyle;
    m_milliseconds = milliseconds;

    m_window = new wxSplashScreenWindow(bitmap, this, wxID_ANY, pos, size, wxNO_BORDER);

    SetClientSize(bitmap.GetWidth(), bitmap.GetHeight());

    if (m_splashStyle & wxSPLASH_CENTRE_ON_PARENT)
        CentreOnParent();
    else if (m_splashStyle & wxSPLASH_CENTRE_ON_SCREEN)
        CentreOnScreen();

    if (m_splashStyle & wxSPLASH_TIMEOUT)
    {
        m_timer.SetOwner(this, wxSPLASH_TIMER_ID);
        m_timer.Start(milliseconds, true);
    }

    Show(true);
    m_window->SetFocus();
    Update(); // Without this, you see a blank screen for an instant
    wxYieldIfNeeded(); // Should eliminate this
}

wxSplashScreenModal::~wxSplashScreenModal()
{
    m_timer.Stop();
}

void wxSplashScreenModal::OnNotify(wxTimerEvent& WXUNUSED(event))
{
    Close(true);
}

void wxSplashScreenModal::OnCloseWindow(wxCloseEvent& WXUNUSED(event))
{
    m_timer.Stop();
    this->Destroy();
}
