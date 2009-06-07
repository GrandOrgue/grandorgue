/*
 * OurOrgan - free pipe organ simulator based on MyOrgan
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

#include "MyOrgan.h"

extern MySound* g_sound;
extern MyOrganFile* organfile;

BEGIN_EVENT_TABLE(MyMeter, wxControl)
	EVT_TEXT(ID_METER_AUDIO_SPIN, MyMeter::OnVolume)
	EVT_TEXT(ID_METER_POLY_SPIN,  MyMeter::OnPolyphony)
	EVT_TEXT(ID_METER_FRAME_SPIN, MyMeter::OnFrame)
    EVT_TEXT(ID_METER_TRANSPOSE_SPIN, MyMeter::OnTranspose)
	EVT_TEXT_ENTER(ID_METER_AUDIO_SPIN, MyMeter::OnEnter)
	EVT_TEXT_ENTER(ID_METER_POLY_SPIN,  MyMeter::OnEnter)
	EVT_TEXT_ENTER(ID_METER_FRAME_SPIN,  MyMeter::OnEnter)
    EVT_TEXT_ENTER(ID_METER_TRANSPOSE_SPIN, MyMeter::OnEnter)
END_EVENT_TABLE()

MyMeter::MyMeter(wxWindow* parent, wxWindowID id, int count) : wxControl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER)
{
	static const wxChar* strings[] = {_("Transpose: "),_("Program: "), _("Polyphony: "), _("Volume (%): ")};
	if (count < 0 || count > 3)
		count = 0;
	m_count = count;
	m_block = false;

	wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);

	topSizer->Add(new wxStaticText(this, wxID_ANY, strings[count]), 0, wxLEFT | wxALIGN_CENTER_VERTICAL, 5);

	switch (count)
	{
        case 0:
            m_spin = new wxSpinCtrl(this, id++, wxEmptyString, wxDefaultPosition, wxSize(48, wxDefaultCoord), wxSP_ARROW_KEYS, -11, 11);
            break;
		case 1:
		case 3:
		    m_spin = new wxSpinCtrl(this, id++, wxEmptyString, wxDefaultPosition, wxSize(48, wxDefaultCoord), wxSP_ARROW_KEYS, 1, (count == 2) ? 512 : 100);
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
            SetValue(wxConfig::Get()->Read("PolyphonyLimit", 1024));
            break;
		case 3:
            SetValue(wxConfig::Get()->Read("Volume", 50));
            break;
	}
}

int MyMeter::GetValue()
{
	return m_spin->GetValue();
}

void MyMeter::SetValue(int n)
{
	m_spin->SetValue(n);
}

void MyMeter::SetValue(int which, int n)
{
	n &= 255;
	m_meters[which]->SetValue(n);
}

void MyMeter::OnVolume(wxCommandEvent& event)
{
	wxString str;
	long n = m_spin->GetValue(), v;
	if (!event.GetString().ToLong(&v) || n != v)
	{
		m_spin->SetValue(n);
		m_spin->SetSelection(-1, -1);
		return;
	}
	if (g_sound)
	    g_sound->volume = n;

	m_meters[0]->ResetClip();
	m_meters[1]->ResetClip();

	wxConfig::Get()->Write("Volume", n);
}

void MyMeter::OnPolyphony(wxCommandEvent& event)
{
	wxString str;
	long n = m_spin->GetValue(), v;
	if (!event.GetString().ToLong(&v) || n != v)
	{
		m_spin->SetValue(n);
		m_spin->SetSelection(-1, -1);
		return;
	}

	wxColour colour = m_meters[0]->GetBackgroundColour();
	m_meters[0]->ResetClip();

	wxConfig::Get()->Write("PolyphonyLimit", n);
	if (g_sound)
	{
		g_sound->polyphony = n;
		g_sound->poly_soft = (g_sound->polyphony * 3) / 4;
    }
}

void MyMeter::OnFrame(wxCommandEvent& event)
{
	wxString str;
	long n = m_spin->GetValue(), v;
	if (!event.GetString().ToLong(&v) || n != v)
	{
		m_spin->SetValue(n);
		m_spin->SetSelection(-1, -1);
		return;
	}

	if (organfile)
		organfile->framegeneral[n - 1].Push();
}

void MyMeter::OnTranspose(wxCommandEvent& event)
{
	long n = m_spin->GetValue(), v;
	if (!event.GetString().ToLong(&v) || n != v)
	{
		m_spin->SetValue(n);
		m_spin->SetSelection(-1, -1);
		return;
	}
    if (g_sound)
    {
        g_sound->transpose = n;
		g_sound->ResetSound();
    }
}
void MyMeter::OnEnter(wxCommandEvent& event)
{
	if (event.GetId() == ID_METER_FRAME_SPIN && organfile)
		organfile->framegeneral[m_spin->GetValue() - 1].Push();
	::wxGetApp().frame->SetFocus();
}

MyMeter::~MyMeter(void)
{
}
