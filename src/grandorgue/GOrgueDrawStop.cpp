/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2012 GrandOrgue contributors (see AUTHORS)
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

#include "GOrgueConfigReader.h"
#include "GOrgueConfigWriter.h"
#include "GOrgueDrawStop.h"
#include "GOrgueLCD.h"

GOrgueDrawstop::GOrgueDrawstop(GrandOrgueFile* organfile) :
	GOrgueButton(organfile, MIDI_RECV_DRAWSTOP, false)
{
}

void GOrgueDrawstop::Load(GOrgueConfigReader& cfg, wxString group, wxString name)
{
	m_Engaged = cfg.ReadBoolean(UserSetting, group, wxT("DefaultToEngaged"));
	GOrgueButton::Load(cfg, group, name);
}

void GOrgueDrawstop::Save(GOrgueConfigWriter& cfg)
{
	cfg.Write(m_group, wxT("DefaultToEngaged"), IsEngaged() ? wxT("Y") : wxT("N"));
	GOrgueButton::Save(cfg);
}

void GOrgueDrawstop::Set(bool on)
{
	if (IsEngaged() == on)
		return;
	Display(on);
	GOrgueLCD_WriteLineTwo(GetName(), 2000);
}

