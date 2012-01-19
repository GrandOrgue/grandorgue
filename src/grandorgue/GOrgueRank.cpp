/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2011 GrandOrgue contributors (see AUTHORS)
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
 * MA 02111-1307, USA.
 */

#include "GOrgueRank.h"
#include "GOrgueStop.h"
#include "IniFileConfig.h"

GOrgueRank::GOrgueRank(GOrgueStop* stop,GrandOrgueFile* organfile) :
	m_organfile(organfile),
	m_Name(),
	m_Group(),
	m_Stop(stop)
{
}

GOrgueRank::~GOrgueRank()
{
}

void GOrgueRank::Load(IniFileConfig& cfg, wxString group)
{
	m_Group = group;
	m_Name = cfg.ReadString(group, wxT("Name"), 64, true);
}

void GOrgueRank::Save(IniFileConfig& cfg, bool prefix)
{
}

GOrguePipe* GOrgueRank::GetPipe(unsigned index)
{
	return m_Stop->GetPipe(index);
}

unsigned GOrgueRank::GetPipeCount()
{
	return m_Stop->GetPipeCount();
}

const wxString& GOrgueRank::GetName()
{
	return m_Name;
}

GOrguePipeConfig& GOrgueRank::GetPipeConfig()
{
	return m_Stop->GetPipeConfig();
}

void GOrgueRank::UpdateAmplitude()
{
        m_Stop->UpdateAmplitude();
}

void GOrgueRank::UpdateTuning()
{
        m_Stop->UpdateTuning();
}

void GOrgueRank::SetTemperament(const GOrgueTemperament& temperament)
{
	m_Stop->SetTemperament(temperament);
}

void GOrgueRank::Abort()
{
	m_Stop->Abort();
}

void GOrgueRank::PreparePlayback()
{
}

