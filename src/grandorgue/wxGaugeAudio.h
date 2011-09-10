/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef WXGAUGEAUDIO_H
#define WXGAUGEAUDIO_H

#include <wx/wx.h>

class wxGaugeAudio : public wxControl
{
public:
	wxGaugeAudio(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
	~wxGaugeAudio(void);

	void OnErase(wxEraseEvent& event);

	void SetValue(int what);
	void ResetClip();

	DECLARE_EVENT_TABLE()
protected:
	int m_value;
	bool m_clip;
	wxMemoryDC m_gaugedc;
	wxBitmap m_gauge;
};

#endif
