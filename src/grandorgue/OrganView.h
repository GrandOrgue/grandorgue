/*
 * GrandOrgue - free pipe organ simulator
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

#ifndef ORGANVIEW_H
#define ORGANVIEW_H

#include <wx/docview.h>

class GOGUIPanelWidget;
class OrganDocument;

class OrganView : public wxView
{
private:
	GOGUIPanelWidget* m_panel;
	wxScrolledWindow* m_container;
	wxWindow* m_frame;
	OrganDocument* m_doc;
	unsigned m_panelID;

	bool CreateWindow();
public:
	OrganView(unsigned m_panelID = 0);
	~OrganView();

	void OnWindowClosed();
	bool OnCreate(wxDocument *doc, long flags);
	bool OnClose(bool deleteWindow = true);
	void OnDraw(wxDC*);
	void OnUpdate(wxView *sender, wxObject *hint = (wxObject *) NULL);

private:
	DECLARE_DYNAMIC_CLASS(OrganView);
};

#endif
