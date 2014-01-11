/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2013 GrandOrgue contributors (see AUTHORS)
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "GOrguePiston.h"

#include "GOrgueConfigReader.h"
#include "GOrgueCoupler.h"
#include "GOrgueDrawStop.h"
#include "GOrgueManual.h"
#include "GOrgueStop.h"
#include "GOrgueSwitch.h"
#include "GOrgueTremulant.h"
#include "GrandOrgueFile.h"
#include <wx/intl.h>

GOrguePiston::GOrguePiston(GrandOrgueFile* organfile) :
	GOrguePushbutton(organfile),
	drawstop(NULL)
{
}

void GOrguePiston::Load(GOrgueConfigReader& cfg, wxString group)
{

	int i, j;
	wxString type = cfg.ReadStringLen(ODFSetting, group, wxT("ObjectType"));
	type.MakeUpper();

	if (type == wxT("STOP"))
	{
		i = cfg.ReadInteger(ODFSetting, group, wxT("ManualNumber"), m_organfile->GetFirstManualIndex(), m_organfile->GetODFManualCount() - 1);
		j = cfg.ReadInteger(ODFSetting, group, wxT("ObjectNumber"), 1, m_organfile->GetManual(i)->GetStopCount()) - 1;
		drawstop = m_organfile->GetManual(i)->GetStop(j);
	}
	else if (type == wxT("COUPLER"))
	{
		i = cfg.ReadInteger(ODFSetting, group, wxT("ManualNumber"), m_organfile->GetFirstManualIndex(), m_organfile->GetODFManualCount() - 1);
		j = cfg.ReadInteger(ODFSetting, group, wxT("ObjectNumber"), 1, m_organfile->GetManual(i)->GetCouplerCount()) - 1;
		drawstop = m_organfile->GetManual(i)->GetCoupler(j);
	}
	else if (type == wxT("TREMULANT"))
	{
		j = cfg.ReadInteger(ODFSetting, group, wxT("ObjectNumber"), 1, m_organfile->GetTremulantCount()) - 1;
		drawstop = m_organfile->GetTremulant(j);
	}
	else if (type == wxT("SWITCH"))
	{
		j = cfg.ReadInteger(ODFSetting, group, wxT("ObjectNumber"), 1, m_organfile->GetSwitchCount()) - 1;
		drawstop = m_organfile->GetSwitch(j);
	}
	else
	  throw wxString::Format(_("Invalid object type for reversible piston"));

	GOrguePushbutton::Load(cfg, group);
	ControlChanged(drawstop);
}

void GOrguePiston::ControlChanged(void* control)
{
	if (control == drawstop)
		Display(drawstop->IsEngaged() ^ drawstop->DisplayInverted());
}

void GOrguePiston::Save(GOrgueConfigWriter& cfg)
{
	GOrguePushbutton::Save(cfg);
}

void GOrguePiston::Push()
{
	this->drawstop->Push();
}


