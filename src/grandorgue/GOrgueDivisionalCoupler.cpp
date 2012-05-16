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
 * MA 02111-1307, USA.
 */

#include "GOrgueDivisionalCoupler.h"
#include "GrandOrgueFile.h"
#include "IniFileConfig.h"

GOrgueDivisionalCoupler::GOrgueDivisionalCoupler(GrandOrgueFile* organfile) :
	GOrgueDrawstop(organfile),
	m_BiDirectionalCoupling(false),
	m_manuals(0)
{
}


void GOrgueDivisionalCoupler::Load(IniFileConfig& cfg, wxString group)
{
	wxString buffer;

	m_BiDirectionalCoupling=cfg.ReadBoolean(ODFSetting, group,wxT("BiDirectionalCoupling"));
	unsigned NumberOfManuals=cfg.ReadInteger(ODFSetting, group,wxT("NumberOfManuals"),  1, m_organfile->GetManualAndPedalCount() - m_organfile->GetFirstManualIndex() + 1);

	m_manuals.resize(0);
	for (unsigned i = 0; i < NumberOfManuals; i++)
	{
		buffer.Printf(wxT("Manual%03d"), i + 1);
		m_manuals.push_back(cfg.ReadInteger(ODFSetting, group, buffer, m_organfile->GetFirstManualIndex(), m_organfile->GetManualAndPedalCount()));
	}
	GOrgueDrawstop::Load(cfg, group);

}

void GOrgueDivisionalCoupler::Save(IniFileConfig& cfg, bool prefix)
{
	GOrgueDrawstop::Save(cfg, prefix);
}

void GOrgueDivisionalCoupler::Set(bool on)
{
	GOrgueDrawstop::Set(on);
}

unsigned GOrgueDivisionalCoupler::GetNumberOfManuals()
{
	return m_manuals.size();
}

unsigned GOrgueDivisionalCoupler::GetManual(unsigned index)
{
	return m_manuals[index];
}

bool GOrgueDivisionalCoupler::IsBidirectional()
{
	return m_BiDirectionalCoupling;
}
