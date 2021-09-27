/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueSwitch.h"

#include "GrandOrgueFile.h"
#include <wx/intl.h>

GOrgueSwitch::GOrgueSwitch(GrandOrgueFile* organfile) :
	GOrgueDrawstop(organfile)
{
}

GOrgueSwitch::~GOrgueSwitch()
{
}

void GOrgueSwitch::ChangeState(bool on)
{
}

void GOrgueSwitch::SetupCombinationState()
{
	m_StoreDivisional = m_organfile->CombinationsStoreNonDisplayedDrawstops() || IsDisplayed();
	m_StoreGeneral = m_organfile->CombinationsStoreNonDisplayedDrawstops() || IsDisplayed();
}

wxString GOrgueSwitch::GetMidiType()
{
	return _("Drawstop");
}
