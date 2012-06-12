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
 */

#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <wx/wx.h>

class GOrgueSplashBitmap : public wxControl
{
private:
	wxBitmap m_Bitmap;

	void OnPaint(wxPaintEvent& event);
	void OnClick(wxMouseEvent& event);
	void OnKey(wxKeyEvent& event);
	DECLARE_EVENT_TABLE()

public:
	GOrgueSplashBitmap(wxWindow *parent, wxWindowID id, wxBitmap& bitmap);
};

class GOrgueSplash : private wxDialog
{
private:
	GOrgueSplashBitmap* m_Image;
	wxTimer  m_Timer;

	void OnCloseWindow(wxCloseEvent& event);
	void OnNotify(wxTimerEvent& event);

	void DrawText(wxBitmap& bitmap);

	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(GOrgueSplash)

public:
	GOrgueSplash
		(int                has_timeout
		,wxWindow          *parent
		,wxWindowID         id
		);
	~GOrgueSplash();

};

#endif
