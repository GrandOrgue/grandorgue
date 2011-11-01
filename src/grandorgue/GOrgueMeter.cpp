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

#include <wx/confbase.h>

extern GOrgueSound* g_sound;

BEGIN_EVENT_TABLE(GOrgueMeter, wxControl)
	EVT_TEXT(ID_METER_AUDIO_SPIN, GOrgueMeter::OnVolume)
	EVT_TEXT(ID_METER_FRAME_SPIN, GOrgueMeter::OnFrame)
	EVT_TEXT_ENTER(ID_METER_AUDIO_SPIN, GOrgueMeter::OnEnter)
	EVT_TEXT_ENTER(ID_METER_FRAME_SPIN,  GOrgueMeter::OnEnter)
	EVT_SLIDER(0, GOrgueMeter::OnChange)
END_EVENT_TABLE()

GOrgueMeter::GOrgueMeter(wxWindow* parent, wxWindowID id, int count)
: wxControl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER)
{
	if (count < 0 || count > 3)
		count = 0;
	m_count = count;
	m_block = false;

	wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);

	switch (count)
	{
	case 1:
		m_spin = new wxSpinCtrl(this, id++, wxEmptyString, wxDefaultPosition, wxSize(46, wxDefaultCoord), wxSP_ARROW_KEYS, 0, 999);
		break;
	case 3:
		m_spin = new wxSpinCtrl(this, id++, wxEmptyString, wxDefaultPosition, wxSize(46, wxDefaultCoord), wxSP_ARROW_KEYS, 1, 100);
		break;
	}

	topSizer->Add(m_spin, 0, wxALIGN_CENTER_VERTICAL);

	SetSizer(topSizer);
	topSizer->Fit(this);
	Layout();

	switch (count)
	{
	case 1:
		SetValue(0);
		break;
	case 3:
		SetValue(wxConfigBase::Get()->Read(wxT("Volume"), 50));
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

void GOrgueMeter::ChangeValue(int n)
{
	wxCommandEvent event(wxEVT_COMMAND_SLIDER_UPDATED, 0);
	event.SetInt(n);
	AddPendingEvent(event);
}

void GOrgueMeter::OnChange(wxCommandEvent& event)
{
	m_spin->SetValue(event.GetInt());
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
	    g_sound->GetEngine().SetVolume(n);

	wxConfigBase::Get()->Write(wxT("Volume"), n);
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
		::wxGetApp().frame->ChangeSetter(n);
}

void GOrgueMeter::OnEnter(wxCommandEvent& event)
{
	if (event.GetId() == ID_METER_FRAME_SPIN)
		if (::wxGetApp().frame)
			::wxGetApp().frame->ChangeSetter(m_spin->GetValue());
	::wxGetApp().frame->SetFocus();
}

GOrgueMeter::~GOrgueMeter(void)
{
}
