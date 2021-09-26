/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
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

float GOrgueTemperamentUser::GetNoteOffset(unsigned note)
{
	return m_Tuning[note];
}

void GOrgueTemperamentUser::SetNoteOffset(unsigned note, float offset)
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
