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
#include "GOrgueDrawStop.h"
#include "GOrgueCoupler.h"
#include "GOrgueStop.h"
#include "GOrgueTremulant.h"
#include "IniFileConfig.h"

GOrguePiston::GOrguePiston(GrandOrgueFile* organfile) :
	GOrguePushbutton(organfile),
	drawstop(NULL)
{
}

void GOrguePiston::Load(IniFileConfig& cfg, wxString group, GOGUIDisplayMetrics* displayMetrics)
{

	int i, j;
	wxString type = cfg.ReadString(group, wxT("ObjectType"));
	type.MakeUpper();

	if (type == wxT("STOP"))
	{
		i = cfg.ReadInteger(group, wxT("ManualNumber"), m_organfile->GetFirstManualIndex(), m_organfile->GetManualAndPedalCount());
		j = cfg.ReadInteger(group, wxT("ObjectNumber"), 1, m_organfile->GetManual(i)->GetStopCount()) - 1;
		drawstop = m_organfile->GetManual(i)->GetStop(j);
	}
	if (type == wxT("COUPLER"))
	{
		i = cfg.ReadInteger(group, wxT("ManualNumber"), m_organfile->GetFirstManualIndex(), m_organfile->GetManualAndPedalCount());
		j = cfg.ReadInteger(group, wxT("ObjectNumber"), 1, m_organfile->GetManual(i)->GetCouplerCount()) - 1;
		drawstop = m_organfile->GetManual(i)->GetCoupler(j);
	}
	if (type == wxT("TREMULANT"))
	{
		j = ObjectNumber=cfg.ReadInteger(group, wxT("ObjectNumber"), 1, m_organfile->GetTremulantCount()) - 1;
		drawstop = m_organfile->GetTremulant(j);
	}

	GOrguePushbutton::Load(cfg, group, displayMetrics);
	Display(drawstop->IsEngaged() ^ drawstop->DisplayInverted());

}

void GOrguePiston::Save(IniFileConfig& cfg, bool prefix)
{
	GOrguePushbutton::Save(cfg, prefix);
}

void GOrguePiston::Push()
{
	this->drawstop->Push();
}


