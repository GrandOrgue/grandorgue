/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2012 GrandOrgue contributors (see AUTHORS)
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

#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <wx/wx.h>

class wxSplashScreenWindow;

class wxSplashScreenModal : public wxDialog
{
public:
    wxSplashScreenModal(const wxBitmap& bitmap, long splashStyle, int milliseconds,
                   wxWindow* parent, wxWindowID id,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   long style = wxSIMPLE_BORDER|wxFRAME_NO_TASKBAR|wxSTAY_ON_TOP);
    ~wxSplashScreenModal();

    void OnCloseWindow(wxCloseEvent& event);
    void OnNotify(wxTimerEvent& event);

    long GetSplashStyle() const { return m_splashStyle; }
    wxSplashScreenWindow* GetSplashWindow() const { return m_window; }
    int GetTimeout() const { return m_milliseconds; }

protected:
    wxSplashScreenWindow*   m_window;
    long                    m_splashStyle;
    int                     m_milliseconds;
    wxTimer                 m_timer;

    DECLARE_EVENT_TABLE()
    DECLARE_NO_COPY_CLASS(wxSplashScreenModal)
};

#endif
