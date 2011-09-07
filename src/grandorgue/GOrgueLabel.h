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

#ifndef GORGUELABEL_H
#define GORGUELABEL_H

#include <wx/wx.h>

class GOrgueDisplayMetrics;
class GrandOrgueFile;
class IniFileConfig;

class GOrgueLabel
{

private:
	GrandOrgueFile* m_organfile;

	bool FreeXPlacement;
	bool FreeYPlacement;
	bool DispSpanDrawstopColToRight;
	bool DispAtTopOfDrawstopCol;
	int DispDrawstopCol;
	int DispXpos;
	int DispYpos;
	int DispLabelFontSize;
	int DispImageNum;
	wxColour DispLabelColour;
	wxString Name;
	GOrgueDisplayMetrics* DisplayMetrics;

public:

	GOrgueLabel(GrandOrgueFile* m_organfile);
	void Load(IniFileConfig& cfg, wxString group, GOrgueDisplayMetrics* displayMetrics);
	void Draw(wxDC& dc);

};

#endif
