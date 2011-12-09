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

#include "GOrgueDrawStop.h"
#include "GOrgueLCD.h"
#include "IniFileConfig.h"

GOrgueDrawstop::GOrgueDrawstop(GrandOrgueFile* organfile) :
	GOrgueButton(organfile, MIDI_RECV_DRAWSTOP, false)
{
}

void GOrgueDrawstop::Load(IniFileConfig& cfg, wxString group, wxString name)
{
	m_Engaged = cfg.ReadBoolean(group, wxT("DefaultToEngaged"));
	GOrgueButton::Load(cfg, group, name);
}

void GOrgueDrawstop::Save(IniFileConfig& cfg, bool prefix)
{
	cfg.SaveHelper(prefix, m_group, wxT("DefaultToEngaged"), IsEngaged() ? wxT("Y") : wxT("N"));
	GOrgueButton::Save(cfg, prefix);
}

void GOrgueDrawstop::Set(bool on)
{
	if (IsEngaged() == on)
		return;
	Display(on);
	GOrgueLCD_WriteLineTwo(GetName(), 2000);
}

