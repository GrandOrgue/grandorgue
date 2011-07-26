/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "GOrgueCoupler.h"
#include "IniFileConfig.h"
#include "GOrgueSound.h"
#include "GrandOrgueFile.h"

extern GrandOrgueFile* organfile;

GOrgueCoupler::GOrgueCoupler() :
	GOrgueDrawstop(),
	UnisonOff(false),
	CoupleToSubsequentUnisonIntermanualCouplers(false),
	CoupleToSubsequentUpwardIntermanualCouplers(false),
	CoupleToSubsequentDownwardIntermanualCouplers(false),
	CoupleToSubsequentUpwardIntramanualCouplers(false),
	CoupleToSubsequentDownwardIntramanualCouplers(false),
	DestinationManual(0),
	DestinationKeyshift(0)
{

}

void GOrgueCoupler::Load(IniFileConfig& cfg, wxString group, int firstValidManualIndex, int numberOfManuals, GOrgueDisplayMetrics* displayMetrics)
{

	UnisonOff                                     = cfg.ReadBoolean(group, wxT("UnisonOff"));
	DestinationManual                             = cfg.ReadInteger(group, wxT("DestinationManual"), firstValidManualIndex, numberOfManuals, !UnisonOff);
	DestinationKeyshift                           = cfg.ReadInteger(group, wxT("DestinationKeyshift"), -24, 24, !UnisonOff);
	CoupleToSubsequentUnisonIntermanualCouplers   = cfg.ReadBoolean(group, wxT("CoupleToSubsequentUnisonIntermanualCouplers"), !UnisonOff);
	CoupleToSubsequentUpwardIntermanualCouplers   = cfg.ReadBoolean(group, wxT("CoupleToSubsequentUpwardIntermanualCouplers"), !UnisonOff);
	CoupleToSubsequentDownwardIntermanualCouplers = cfg.ReadBoolean(group, wxT("CoupleToSubsequentDownwardIntermanualCouplers"), !UnisonOff);
	CoupleToSubsequentUpwardIntramanualCouplers   = cfg.ReadBoolean(group, wxT("CoupleToSubsequentUpwardIntramanualCouplers"), !UnisonOff);
	CoupleToSubsequentDownwardIntramanualCouplers = cfg.ReadBoolean(group, wxT("CoupleToSubsequentDownwardIntramanualCouplers"), !UnisonOff);
	GOrgueDrawstop::Load(cfg, group, displayMetrics);

}

void GOrgueCoupler::Save(IniFileConfig& cfg, bool prefix, wxString group)
{

	GOrgueDrawstop::Save(cfg, prefix, group);

}

bool GOrgueCoupler::Set(bool on)
{

	if (DefaultToEngaged == on)
		return on;

	DefaultToEngaged = on;
	organfile->MIDIPretend(true);
	DefaultToEngaged = !on;
	organfile->MIDIPretend(false);

	return GOrgueDrawstop::Set(on);

}
