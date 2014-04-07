/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2014 GrandOrgue contributors (see AUTHORS)
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

#include "GOrgueTemperamentUser.h"

#include "GOrgueConfigReader.h"
#include "GOrgueConfigWriter.h"


GOrgueTemperamentUser::GOrgueTemperamentUser(wxString name, wxString title, wxString group) :
	GOrgueTemperamentCent(name, title, group)
{
}

GOrgueTemperamentUser::GOrgueTemperamentUser(GOrgueConfigReader& cfg, wxString group) :
	GOrgueTemperamentCent(wxEmptyString, wxEmptyString, wxEmptyString)
{
	m_Name = cfg.ReadString(CMBSetting, group, wxT("Name"));
	m_Title = cfg.ReadString(CMBSetting, group, wxT("Title"));
	m_Group = cfg.ReadString(CMBSetting, group, wxT("Group"));
	for(unsigned i = 0; i < 12; i++)
		m_Tuning[i] = cfg.ReadFloat(CMBSetting, group, wxString::Format(wxT("Offset%d"), i), -1200, 1200);
}

void GOrgueTemperamentUser::Save(GOrgueConfigWriter& cfg, wxString group)
{
	cfg.WriteString(group, wxT("Name"), m_Name);
	cfg.WriteString(group, wxT("Title"), m_Title);
	cfg.WriteString(group, wxT("Group"), m_Group);
	for(unsigned i = 0; i < 12; i++)
		cfg.WriteFloat(group, wxString::Format(wxT("Offset%d"), i), m_Tuning[i]);
}

float GOrgueTemperamentUser::GetOffset(unsigned note)
{
	return m_Tuning[note];
}

void GOrgueTemperamentUser::SetOffset(unsigned note, float offset)
{
	if (offset > 1200)
		offset = 1200;
	if (offset < -1200)
		offset = -1200;
	m_Tuning[note] = offset;
}

void GOrgueTemperamentUser::SetTitle(wxString title)
{
	m_Title = title;
}

void GOrgueTemperamentUser::SetGroup(wxString group)
{
	m_Group = group;
}
