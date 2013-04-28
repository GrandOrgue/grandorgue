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

#include "GOrgueConfigReader.h"
#include "GOrgueConfigWriter.h"
#include "GOrgueDrawStop.h"
#include "GOrgueLCD.h"

const struct IniFileEnumEntry GOrgueDrawstop::m_function_types[] = {
	{ wxT("Input"), FUNCTION_INPUT},
};

GOrgueDrawstop::GOrgueDrawstop(GrandOrgueFile* organfile) :
	GOrgueButton(organfile, MIDI_RECV_DRAWSTOP, false),
	m_Type(FUNCTION_INPUT),
	m_GCState(0),
	m_ActiveState(false),
	m_CombinationState(false),
	m_ControlledDrawstops(),
	m_ControllingDrawstops()
{
}

void GOrgueDrawstop::RegisterControlled(GOrgueDrawstop* sw)
{
	m_ControlledDrawstops.push_back(sw);
}

void GOrgueDrawstop::Init(GOrgueConfigReader& cfg, wxString group, wxString name)
{
	m_Type = FUNCTION_INPUT;
	m_Engaged = cfg.ReadBoolean(CMBSetting, group, wxT("DefaultToEngaged"));
	m_GCState = 0;
	GOrgueButton::Init(cfg, group, name);
}

void GOrgueDrawstop::Load(GOrgueConfigReader& cfg, wxString group)
{
	m_Type = (GOrgueFunctionType)cfg.ReadEnum(ODFSetting, group, wxT("Function"), m_function_types, sizeof(m_function_types) / sizeof(m_function_types[0]), false, FUNCTION_INPUT);

	if (m_Type == FUNCTION_INPUT)
	{
		m_Engaged = cfg.ReadBoolean(UserSetting, group, wxT("DefaultToEngaged"));
		cfg.ReadBoolean(ODFSetting, group, wxT("DefaultToEngaged"));
		m_GCState = cfg.ReadInteger(ODFSetting, group, wxT("GCState"), -1, 1, false, 0);
	}
	else
	{
		m_ReadOnly = true;
	}

	GOrgueButton::Load(cfg, group);
}

void GOrgueDrawstop::Save(GOrgueConfigWriter& cfg)
{
	if (!IsReadOnly())
		cfg.Write(m_group, wxT("DefaultToEngaged"), IsEngaged());
	GOrgueButton::Save(cfg);
}

void GOrgueDrawstop::Set(bool on)
{
	if (IsEngaged() == on)
		return;
	Display(on);
	GOrgueLCD_WriteLineTwo(GetName(), 2000);
	SetState(on);
}

void GOrgueDrawstop::Reset()
{
	if (IsReadOnly())
		return;
	if (m_GCState < 0)
		return;
	Set(m_GCState > 0 ? true : false);
}

void GOrgueDrawstop::SetState(bool on)
{
	if (IsActive() == on)
		return;
	if (IsReadOnly())
	{
		Display(on);
		GOrgueLCD_WriteLineTwo(GetName(), 2000);
	}
	m_ActiveState = on;
	ChangeState(on);
	for(unsigned i = 0; i < m_ControlledDrawstops.size(); i++)
		m_ControlledDrawstops[i]->Update();
}

void GOrgueDrawstop::SetCombination(bool on)
{
	if (IsReadOnly())
		return;
	m_CombinationState = on;
	Set(on);
}

bool GOrgueDrawstop::IsActive() const
{
	return m_ActiveState;
}

void GOrgueDrawstop::PreparePlayback()
{
	GOrgueButton::PreparePlayback();
	Update();
}

void GOrgueDrawstop::Update()
{
	switch(m_Type)
	{
	case FUNCTION_INPUT:
		SetState(IsEngaged());
		break;
	}
}

bool GOrgueDrawstop::GetCombinationState() const
{
	return IsEngaged();
}
