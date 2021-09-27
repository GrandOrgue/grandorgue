/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrguePiston.h"

#include "GOrgueConfigReader.h"
#include "GOrgueCoupler.h"
#include "GOrgueDrawStop.h"
#include "GOrgueManual.h"
#include "GOrgueStop.h"
#include "GOrgueSwitch.h"
#include "GOrgueTremulant.h"
#include "GrandOrgueFile.h"
#include <wx/intl.h>
#include <wx/log.h>

GOrguePiston::GOrguePiston(GrandOrgueFile* organfile) :
	GOrguePushbutton(organfile),
	drawstop(NULL)
{
	m_organfile->RegisterControlChangedHandler(this);
}

void GOrguePiston::Load(GOrgueConfigReader& cfg, wxString group)
{
	int i, j;
	wxString type = cfg.ReadStringTrim(ODFSetting, group, wxT("ObjectType"));
	type.MakeUpper();

	if (type == wxT("STOP"))
	{
		i = cfg.ReadInteger(ODFSetting, group, wxT("ManualNumber"), m_organfile->GetFirstManualIndex(), m_organfile->GetODFManualCount() - 1);
		j = cfg.ReadInteger(ODFSetting, group, wxT("ObjectNumber"), 1, m_organfile->GetManual(i)->GetStopCount()) - 1;
		drawstop = m_organfile->GetManual(i)->GetStop(j);
	}
	else if (type == wxT("COUPLER"))
	{
		i = cfg.ReadInteger(ODFSetting, group, wxT("ManualNumber"), m_organfile->GetFirstManualIndex(), m_organfile->GetODFManualCount() - 1);
		j = cfg.ReadInteger(ODFSetting, group, wxT("ObjectNumber"), 1, m_organfile->GetManual(i)->GetODFCouplerCount()) - 1;
		drawstop = m_organfile->GetManual(i)->GetCoupler(j);
	}
	else if (type == wxT("TREMULANT"))
	{
		j = cfg.ReadInteger(ODFSetting, group, wxT("ObjectNumber"), 1, m_organfile->GetTremulantCount()) - 1;
		drawstop = m_organfile->GetTremulant(j);
	}
	else if (type == wxT("SWITCH"))
	{
		j = cfg.ReadInteger(ODFSetting, group, wxT("ObjectNumber"), 1, m_organfile->GetSwitchCount()) - 1;
		drawstop = m_organfile->GetSwitch(j);
	}
	else
	  throw wxString::Format(_("Invalid object type for reversible piston"));

	if (drawstop->IsReadOnly())
		wxLogError(_("Reversible piston connect to a read-only object: %s"), group.c_str());

	GOrguePushbutton::Load(cfg, group);
	ControlChanged(drawstop);
}

void GOrguePiston::ControlChanged(void* control)
{
	if (control == drawstop)
		Display(drawstop->IsEngaged() ^ drawstop->DisplayInverted());
}

void GOrguePiston::Push()
{
	this->drawstop->Push();
}

wxString GOrguePiston::GetMidiType()
{
	return _("Piston");
}

