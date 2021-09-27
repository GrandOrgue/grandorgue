/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueDivisionalCoupler.h"

#include "GOrgueConfigReader.h"
#include "GrandOrgueFile.h"
#include <wx/intl.h>

GOrgueDivisionalCoupler::GOrgueDivisionalCoupler(GrandOrgueFile* organfile) :
	GOrgueDrawstop(organfile),
	m_BiDirectionalCoupling(false),
	m_manuals(0)
{
}


void GOrgueDivisionalCoupler::Load(GOrgueConfigReader& cfg, wxString group)
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

void GOrgueDivisionalCoupler::SetupCombinationState()
{
	m_StoreDivisional = false;
	m_StoreGeneral = m_organfile->GeneralsStoreDivisionalCouplers();
}

void GOrgueDivisionalCoupler::ChangeState(bool on)
{
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

wxString GOrgueDivisionalCoupler::GetMidiType()
{
	return _("Divisional Coupler");
}
