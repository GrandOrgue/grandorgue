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

#include "GOrguePiston.h"
#include "GrandOrgueFile.h"

/* TODO: This should not be... */
extern GrandOrgueFile* organfile;

GOrguePiston::GOrguePiston() :
	GOrguePushbutton(),
	drawstop(NULL)
{

}

void GOrguePiston::Load(IniFileConfig& cfg, const char* group)
{

	int i, j;
	wxString type = cfg.ReadString(group, "ObjectType");
	type.MakeUpper();

	if (type == "STOP")
	{
		i = cfg.ReadInteger(group, "ManualNumber", organfile->m_FirstManual, organfile->m_NumberOfManuals);
		j = cfg.ReadInteger(group, "ObjectNumber", 1, organfile->m_manual[i].NumberOfStops) - 1;
		drawstop = organfile->m_manual[i].stop[j];
	}
	if (type == "COUPLER")
	{
		i = cfg.ReadInteger(group, "ManualNumber", organfile->m_FirstManual, organfile->m_NumberOfManuals);
		j = cfg.ReadInteger(group, "ObjectNumber", 1, organfile->m_manual[i].NumberOfCouplers) - 1;
		drawstop = &organfile->m_manual[i].coupler[j];
	}
	if (type == "TREMULANT")
	{
		j = ObjectNumber=cfg.ReadInteger(group, "ObjectNumber", 1, organfile->m_NumberOfTremulants) - 1;
		drawstop = &organfile->m_tremulant[j];
	}

	GOrguePushbutton::Load(cfg, group);
	if (drawstop->DefaultToEngaged ^ drawstop->DisplayInInvertedState)
		DispImageNum ^= 2;

}

void GOrguePiston::Save(IniFileConfig& cfg, bool prefix)
{
	GOrguePushbutton::Save(cfg, prefix, "ReversiblePiston");
}

void GOrguePiston::Push(int WXUNUSED(depth))
{
	this->drawstop->Push();
}


