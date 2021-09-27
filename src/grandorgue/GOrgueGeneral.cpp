/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueGeneral.h"

#include <wx/intl.h>

GOrgueGeneral::GOrgueGeneral(GOrgueCombinationDefinition& general_template, GrandOrgueFile* organfile, bool is_setter):
	GOrguePushbutton(organfile),
	m_general(general_template, organfile, is_setter)
{
}

void GOrgueGeneral::Load(GOrgueConfigReader& cfg, wxString group)
{
	m_general.Load(cfg, group);
	GOrguePushbutton::Load(cfg, group);
}

void GOrgueGeneral::Push()
{
	m_general.Push();
}

GOrgueFrameGeneral& GOrgueGeneral::GetGeneral()
{
	return m_general;
}

wxString GOrgueGeneral::GetMidiType()
{
	return _("General");
}
