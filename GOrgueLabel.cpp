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
#include "GOrgueDisplayMetrics.h"
#include "IniFileConfig.h"

/* TODO: This should not be... */
extern GrandOrgueFile* organfile;

GOrgueLabel::GOrgueLabel() :
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

void GOrgueLabel::Load(IniFileConfig& cfg, const char* group, GOrgueDisplayMetrics* displayMetrics)
{
	Name=cfg.ReadString( group,"Name",   64);
	FreeXPlacement=cfg.ReadBoolean( group,"FreeXPlacement");
	FreeYPlacement=cfg.ReadBoolean( group,"FreeYPlacement");

	if (!FreeXPlacement)
	{
		DispDrawstopCol=cfg.ReadInteger(group, "DispDrawstopCol", 1, displayMetrics->NumberOfDrawstopColsToDisplay());
		DispSpanDrawstopColToRight=cfg.ReadBoolean(group, "DispSpanDrawstopColToRight");
	}
	else
		DispXpos=cfg.ReadInteger(group, "DispXpos", 0, displayMetrics->GetScreenWidth());

	if (!FreeYPlacement)
		DispAtTopOfDrawstopCol=cfg.ReadBoolean(group, "DispAtTopOfDrawstopCol");
	else
		DispYpos=cfg.ReadInteger(group, "DispYpos", 0, displayMetrics->GetScreenHeight());

	// NOTICE: this should not be allowed, but some existing definition files use improper values
	if (!DispXpos)
		DispYpos++;
	if (!DispYpos)
		DispYpos++;

	DispLabelColour = cfg.ReadColor(group, "DispLabelColour");
	DispLabelFontSize = cfg.ReadFontSize(group, "DispLabelFontSize");
	DispImageNum = cfg.ReadInteger(group, "DispImageNum", 1, 1);
}


