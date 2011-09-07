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

#include "OrganView.h"
#include "OrganPanel.h"
#include "GOrgueSound.h"
#include "GrandOrgue.h"

IMPLEMENT_DYNAMIC_CLASS(OrganView, wxView)

BEGIN_EVENT_TABLE(OrganView, wxView)
	EVT_COMMAND(0, wxEVT_GOCONTROL, OrganView::OnGOControl)
END_EVENT_TABLE()

bool OrganView::OnCreate(wxDocument *doc, long flags)
{

	m_panel=new OrganPanel(::wxGetApp().frame);
	SetFrame(::wxGetApp().frame);
	return true;

}

void OrganView::OnUpdate(wxView *sender, wxObject *hint)
{

	m_panel->OnUpdate(sender,hint);

}


void OrganView::OnDraw(wxDC* dc)
{

	m_panel->OnDraw(dc);

}

void OrganView::OnGOControl(wxCommandEvent& event)
{
	m_panel->OnGOControl(event);
}

bool OrganView::OnClose(bool deleteWindow)
{

	m_panel->Destroy();
	GetFrame()->SetBackgroundStyle(wxBG_STYLE_SYSTEM);
	GetFrame()->Refresh();
	if (!GetDocument()->Close())
		return false;

	SetFrame(0);
	Activate(false);
	return true;

}
