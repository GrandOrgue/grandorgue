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
#include "GrandOrgueFile.h"
#include "GOrgueSound.h"
extern GrandOrgueFile* organfile;

void GOrgueCoupler::Load(IniFileConfig& cfg, const char* group)
{
  UnisonOff										= cfg.ReadBoolean( group,"UnisonOff");
  DestinationManual								= cfg.ReadInteger( group,"DestinationManual", organfile->m_FirstManual, organfile->m_NumberOfManuals, !UnisonOff);
  DestinationKeyshift							= cfg.ReadInteger( group,"DestinationKeyshift",  -24,   24, !UnisonOff);
  CoupleToSubsequentUnisonIntermanualCouplers	= cfg.ReadBoolean( group,"CoupleToSubsequentUnisonIntermanualCouplers", !UnisonOff);
  CoupleToSubsequentUpwardIntermanualCouplers	= cfg.ReadBoolean( group,"CoupleToSubsequentUpwardIntermanualCouplers", !UnisonOff);
  CoupleToSubsequentDownwardIntermanualCouplers	= cfg.ReadBoolean( group,"CoupleToSubsequentDownwardIntermanualCouplers", !UnisonOff);
  CoupleToSubsequentUpwardIntramanualCouplers	= cfg.ReadBoolean( group,"CoupleToSubsequentUpwardIntramanualCouplers", !UnisonOff);
  CoupleToSubsequentDownwardIntramanualCouplers	= cfg.ReadBoolean( group,"CoupleToSubsequentDownwardIntramanualCouplers", !UnisonOff);

  GOrgueDrawstop::Load(cfg, group);
}

bool GOrgueCoupler::Set(bool on)
{
  if (DefaultToEngaged == on)
	return on;
  DefaultToEngaged = on;
  GOrgueSound::MIDIPretend(true);
  DefaultToEngaged = !on;
  GOrgueSound::MIDIPretend(false);
  return GOrgueDrawstop::Set(on);
}
