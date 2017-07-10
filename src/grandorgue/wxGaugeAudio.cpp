/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2017 GrandOrgue contributors (see AUTHORS)
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

#include "wxGaugeAudio.h"

#include "Images.h"
#include <wx/dcclient.h>
#include <wx/image.h>

BEGIN_EVENT_TABLE(wxGaugeAudio, wxControl)
	EVT_PAINT(wxGaugeAudio::OnPaint)
END_EVENT_TABLE()

wxGaugeAudio::wxGaugeAudio(wxWindow* parent, wxWindowID id, const wxPoint& pos) :
	wxControl(parent, id, pos, wxSize(73, 11), wxNO_BORDER),
	m_Value(0),
	m_Clip(false),
	m_Update(false)
{
        m_gauge = wxBitmap(GetImage_gauge());
	m_gaugedc.SelectObject(m_gauge);
}

wxGaugeAudio::~wxGaugeAudio(void)
{
}

void wxGaugeAudio::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);
	m_Update = false;
	int split = (m_Value + 1) << 1;
	dc.Blit(0, 0, split, 11, &m_gaugedc, 0, 11);
	if (66 - split)
		dc.Blit(split, 0, 66 - split, 11, &m_gaugedc, split, 0);
	dc.Blit(66, 0, 7, 11, &m_gaugedc, 66, m_Clip ? 11 : 0);
}

void wxGaugeAudio::SetValue(int what)
{
	if (what < 0)
		what = 0;
	if (what > 32)
	{
		what = 32;
		if (!m_Clip)
			m_Clip = true;
	}
        if (what != m_Value)
	{
		m_Value = what;
		Update();
	}
}

void wxGaugeAudio::ResetClip()
{
	if (m_Clip)
	{
		m_Clip = false;
		Update();
	}
}

void wxGaugeAudio::Update()
{
	if (m_Update)
		return;
	m_Update = true;
	Refresh(false);
}
