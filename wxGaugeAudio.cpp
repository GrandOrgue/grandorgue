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

#include "wxGaugeAudio.h"
#include <wx/dcmemory.h>
#include "GrandOrgue.h"

BEGIN_EVENT_TABLE(wxGaugeAudio, wxControl)
	EVT_ERASE_BACKGROUND(wxGaugeAudio::OnErase)
END_EVENT_TABLE()

wxGaugeAudio::wxGaugeAudio(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size) : wxControl(parent, id, pos, size, wxNO_BORDER)
{
	m_value = 0;
	m_clip = false;
    m_frame = (GOrgueFrame*)(GetGrandParent()->GetParent());
}

wxGaugeAudio::~wxGaugeAudio(void)
{
}

void wxGaugeAudio::OnErase(wxEraseEvent& event)
{
    wxMemoryDC* mdc = m_frame->m_gaugedc;
    if (!mdc)
        return;
	wxDC* dc = event.GetDC();
	int split = (m_value + 1) << 1;
	dc->Blit(0, 0, split, 11, mdc, 0, 11);
	if (66 - split)
		dc->Blit(split, 0, 66 - split, 11, mdc, split, 0);
	dc->Blit(66, 0, 7, 11, mdc, 66, m_clip ? 11 : 0);
}

void wxGaugeAudio::SetValue(int what)
{
    wxMemoryDC* mdc = m_frame->m_gaugedc;
	wxClientDC dc(this);
	if (what < 0)
		what = 0;
	if (what > 32)
	{
		what = 32;
		if (!m_clip)
		{
			m_clip = true;
			if (mdc)
                dc.Blit(66, 0, 7, 11, mdc, 66, 11);
		}
	}

	if (mdc)
	{
        int split;
        if (what > m_value)
        {
            split = (m_value + 1) << 1;
            dc.Blit(split, 0, (what - m_value) << 1, 11, mdc, split, 11);
        }
        else if (what < m_value)
        {
            split = (what + 1) << 1;
            dc.Blit(split, 0, (m_value - what) << 1, 11, mdc, split, 0);
        }
	}

	m_value = what;
}

void wxGaugeAudio::ResetClip()
{
    wxMemoryDC* mdc = m_frame->m_gaugedc;
	wxClientDC dc(this);
	if (m_clip)
	{
		m_clip = false;
		if (mdc)
            dc.Blit(66, 0, 7, 11, mdc, 66, 0);
	}
}
