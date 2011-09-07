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

#include "GOrgueLabel.h"
#include "GrandOrgueFile.h"
#include "GOGUIDisplayMetrics.h"
#include "IniFileConfig.h"

GOrgueLabel::GOrgueLabel(GrandOrgueFile* organfile) :
	m_organfile(organfile),
	FreeXPlacement(false),
	FreeYPlacement(false),
	DispSpanDrawstopColToRight(false),
	DispAtTopOfDrawstopCol(false),
	DispDrawstopCol(0),
	DispXpos(0),
	DispYpos(0),
	DispLabelFontSize(0),
	DispImageNum(0),
	DispLabelColour(0,0,0),
	Name()
{

}

void GOrgueLabel::Load(IniFileConfig& cfg, wxString group, GOGUIDisplayMetrics* displayMetrics)
{

	Name = cfg.ReadString(group, wxT("Name"), 64);
	FreeXPlacement = cfg.ReadBoolean(group, wxT("FreeXPlacement"));
	FreeYPlacement = cfg.ReadBoolean(group, wxT("FreeYPlacement"));
	DisplayMetrics = displayMetrics;

	if (!FreeXPlacement)
	{
		DispDrawstopCol = cfg.ReadInteger(group, wxT("DispDrawstopCol"), 1, displayMetrics->NumberOfDrawstopColsToDisplay());
		DispSpanDrawstopColToRight = cfg.ReadBoolean(group, wxT("DispSpanDrawstopColToRight"));
	}
	else
	{
		DispXpos = cfg.ReadInteger(group, wxT("DispXpos"), 0, displayMetrics->GetScreenWidth());
	}

	if (!FreeYPlacement)
	{
		DispAtTopOfDrawstopCol = cfg.ReadBoolean(group, wxT("DispAtTopOfDrawstopCol"));
	}
	else
	{
		DispYpos = cfg.ReadInteger(group, wxT("DispYpos"), 0, displayMetrics->GetScreenHeight());
	}

	/* NOTICE: this should not be allowed, but some existing definition files
	 * use improper values */
	if (!DispXpos)
		DispYpos++;
	if (!DispYpos)
		DispYpos++;

	DispLabelColour = cfg.ReadColor(group, wxT("DispLabelColour"));
	DispLabelFontSize = cfg.ReadFontSize(group, wxT("DispLabelFontSize"));
	DispImageNum = cfg.ReadInteger(group, wxT("DispImageNum"), 1, 1);
}

void GOrgueLabel::Draw(wxDC& dc)
{

	if (!FreeXPlacement)
	{
		int i = DisplayMetrics->NumberOfDrawstopColsToDisplay() >> 1;
		if (DispDrawstopCol <= i)
			DispXpos = DisplayMetrics->GetJambLeftX()  + (DispDrawstopCol - 1) * 78 + 1;
		else
			DispXpos = DisplayMetrics->GetJambRightX() + (DispDrawstopCol - 1 - i) * 78 + 1;
		if (DispSpanDrawstopColToRight)
			DispXpos += 39;
	}

	if (!FreeYPlacement)
	{
		DispYpos = DisplayMetrics->GetJambLeftRightY() + 1;
		if (!DispAtTopOfDrawstopCol)
			DispYpos += DisplayMetrics->GetJambLeftRightHeight() - 32;
	}

	wxRect rect(
		DispXpos - 1,
		DispYpos - 1,
		78,
		22);

	dc.DrawBitmap(*m_organfile->GetImage(8), rect.x, rect.y, false);

	wxFont font = DisplayMetrics->GetGroupLabelFont();
	font.SetPointSize(DispLabelFontSize);
	dc.SetTextForeground(DispLabelColour);
	dc.SetFont(font);
	dc.DrawLabel(Name, rect, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);

}
