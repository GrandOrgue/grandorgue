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

#ifndef ORGANPANEL_H
#define ORGANPANEL_H

#include <wx/wx.h>
#include <wx/docview.h>

class GOGUIDisplayMetrics;
class GOrgueDrawstop;
class GOrguePushbutton;

class OrganPanel : public wxPanel
{

private:
	wxBitmap m_clientBitmap;
	wxPoint m_clientOrigin;
	wxBrush m_pedalBrush;

	GOGUIDisplayMetrics* m_display_metrics;

	void DrawClickables(wxDC* dc = NULL, int xx = 0, int yy = 0, bool right = false, int scroll = 0);
	void CopyToScreen(wxDC* mdc, const wxRect& rect);
	void HelpDrawStop(GOrgueDrawstop* stop, wxDC* dc, int xx, int yy, bool right);
	void HelpDrawButton(GOrguePushbutton* button, wxDC* dc, int xx, int yy, bool right);

public:
	OrganPanel(wxWindow* parent);
	void OnErase(wxEraseEvent& event);
	void OnPaint(wxPaintEvent& event);
	void OnDraw(wxDC *dc);
	void OnGOControl(wxCommandEvent& event);
	void OnDrawstop(wxCommandEvent& event);
	void OnNoteOnOff(wxCommandEvent& event);
	void OnUpdate(wxView *sender, wxObject *hint = (wxObject *) NULL);
	void OnMouseLeftDown(wxMouseEvent& event);
	void OnMouseRightDown(wxMouseEvent& event);
	void OnMouseScroll(wxMouseEvent& event);
	void OnKeyCommand(wxKeyEvent& event);

	static void TileWood(wxDC& dc, int which, int sx, int sy, int cx, int cy);
	static void WrapText(wxDC& dc, wxString& ptr, int width);

	DECLARE_EVENT_TABLE();

};

#endif
