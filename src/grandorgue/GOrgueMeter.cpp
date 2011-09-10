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

#include "GOrgueMeter.h"

#include "GOrgueMidi.h"
#include "GOrgueSound.h"
#include "GrandOrgue.h"
#include "GrandOrgueFrame.h"
#include "GrandOrgueID.h"
#include "wxGaugeAudio.h"

#if defined(__WXMSW__)
    #include <wx/msw/regconf.h>
#endif

extern GOrgueSound* g_sound;

BEGIN_EVENT_TABLE(GOrgueMeter, wxControl)
	EVT_TEXT(ID_METER_AUDIO_SPIN, GOrgueMeter::OnVolume)
	EVT_TEXT(ID_METER_POLY_SPIN,  GOrgueMeter::OnPolyphony)
	EVT_TEXT(ID_METER_FRAME_SPIN, GOrgueMeter::OnFrame)
	EVT_TEXT(ID_METER_TRANSPOSE_SPIN, GOrgueMeter::OnTranspose)
	EVT_TEXT_ENTER(ID_METER_AUDIO_SPIN, GOrgueMeter::OnEnter)
	EVT_TEXT_ENTER(ID_METER_POLY_SPIN,  GOrgueMeter::OnEnter)
	EVT_TEXT_ENTER(ID_METER_FRAME_SPIN,  GOrgueMeter::OnEnter)
	EVT_TEXT_ENTER(ID_METER_TRANSPOSE_SPIN, GOrgueMeter::OnEnter)
END_EVENT_TABLE()

GOrgueMeter::GOrgueMeter(wxWindow* parent, wxWindowID id, int count)
: wxControl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER)
{
	//static const wxChar* strings[] = {_("T"),_("M"), _("P"), _("V")};
	static const wxChar* strings[] = {wxT(""),wxT(""), wxT(""), wxT("")};
	if (count < 0 || count > 3)
		count = 0;
	m_count = count;
	m_block = false;

	wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);

	topSizer->Add(new wxStaticText(this, wxID_ANY, strings[count]), 0, wxLEFT | wxALIGN_CENTER_VERTICAL, 0);

	switch (count)
	{
	case 0:
		m_spin = new wxSpinCtrl(this, id++, wxEmptyString, wxDefaultPosition, wxSize(46, wxDefaultCoord), wxSP_ARROW_KEYS, -11, 11);
		break;
	case 1:
	case 3:
		m_spin = new wxSpinCtrl(this, id++, wxEmptyString, wxDefaultPosition, wxSize(46, wxDefaultCoord), wxSP_ARROW_KEYS, 1, (count == 1) ? 999 : 100);
		break;
	case 2:
		m_spin = new wxSpinCtrl(this, id++, wxEmptyString, wxDefaultPosition, wxSize(56, wxDefaultCoord), wxSP_ARROW_KEYS, 1, 4096);
		break;
	}

	topSizer->Add(m_spin, 0, wxALIGN_CENTER_VERTICAL);

	if (count>1)
	{
		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		for (int i = 0; i < (count-1); i++)
		{
			m_meters[i] = new wxGaugeAudio(this, id++, wxDefaultPosition, wxSize(73, 11));
			sizer->Add(m_meters[i], 0, wxFIXED_MINSIZE);
		}
		topSizer->Add(sizer, 0, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);
	}

	SetSizer(topSizer);
	topSizer->Fit(this);
	Layout();

	switch (count)
	{
	case 0:
		SetValue(0);
		break;
	case 1:
		SetValue(1);
		break;
	case 2:
		SetValue(wxConfig::Get()->Read(wxT("PolyphonyLimit"), 2048));
		break;
	case 3:
		SetValue(wxConfig::Get()->Read(wxT("Volume"), 50));
		break;
	}
}

int GOrgueMeter::GetValue()
{
	return m_spin->GetValue();
}

void GOrgueMeter::SetValue(int n)
{
	m_spin->SetValue(n);
}

void GOrgueMeter::SetValue(int which, int n)
{
	n &= 255;
	m_meters[which]->SetValue(n);
}

void GOrgueMeter::OnVolume(wxCommandEvent& event)
{
	wxString str;
	long n = m_spin->GetValue(), v;
	if (!event.GetString().ToLong(&v) || n != v)
	{
		m_spin->SetValue(n);
		m_spin->SetSelection(-1, -1);
#ifdef __WXMSW__
		return;
#endif
	}
	if (g_sound)
	    g_sound->SetVolume(n);

	m_meters[0]->ResetClip();
	m_meters[1]->ResetClip();

	wxConfig::Get()->Write(wxT("Volume"), n);
}

void GOrgueMeter::OnPolyphony(wxCommandEvent& event)
{
	wxString str;
	long n = m_spin->GetValue(), v;
	if (!event.GetString().ToLong(&v) || n != v)
	{
		m_spin->SetValue(n);
		m_spin->SetSelection(-1, -1);
#ifdef __WXMSW__
		return;
#endif
	}

	wxColour colour = m_meters[0]->GetBackgroundColour();
	m_meters[0]->ResetClip();

	wxConfig::Get()->Write(wxT("PolyphonyLimit"), n);
	if (g_sound)
	{
		g_sound->SetPolyphonyLimit(n);
		g_sound->SetPolyphonySoftLimit((n * 3) / 4);
    }
}

void GOrgueMeter::OnFrame(wxCommandEvent& event)
{
	wxString str;
	long n = m_spin->GetValue(), v;
	if (!event.GetString().ToLong(&v) || n != v)
	{
		m_spin->SetValue(n);
		m_spin->SetSelection(-1, -1);
#ifdef __WXMSW__
		return;
#endif
	}

	if (::wxGetApp().frame)
		::wxGetApp().frame->ChangeSetter(n - 1);
}

void GOrgueMeter::OnTranspose(wxCommandEvent& event)
{
	long n = m_spin->GetValue(), v;
	if (!event.GetString().ToLong(&v) || n != v)
	{
		m_spin->SetValue(n);
		m_spin->SetSelection(-1, -1);
#ifdef __WXMSW__
		return;
#endif
	}
	if (g_sound)
	{
		g_sound->GetMidi().SetTranspose(n);
		g_sound->ResetSound();
	}
}
void GOrgueMeter::OnEnter(wxCommandEvent& event)
{
	if (event.GetId() == ID_METER_FRAME_SPIN)
		if (::wxGetApp().frame)
			::wxGetApp().frame->ChangeSetter(m_spin->GetValue() - 1);
	::wxGetApp().frame->SetFocus();
}

GOrgueMeter::~GOrgueMeter(void)
{
}
