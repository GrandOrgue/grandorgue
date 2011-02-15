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

#ifndef GORGUECOUPLER_H
#define GORGUECOUPLER_H

#include <wx/wx.h>
#include "GOrgueDrawStop.h"

class GOrgueCoupler : public GOrgueDrawstop
{
public:
  GOrgueCoupler():
	GOrgueDrawstop(),UnisonOff(false),
	CoupleToSubsequentUnisonIntermanualCouplers(false),
	CoupleToSubsequentUpwardIntermanualCouplers(false),
	CoupleToSubsequentDownwardIntermanualCouplers(false),
	CoupleToSubsequentUpwardIntramanualCouplers(false),
	CoupleToSubsequentDownwardIntramanualCouplers(false),
	DestinationManual(0),DestinationKeyshift(0)
  {}


	void Load(IniFileConfig& cfg, const char* group);
    void Save(IniFileConfig& cfg, bool prefix) { GOrgueDrawstop::Save(cfg, prefix, "Coupler"); }
	bool Set(bool on);

	bool UnisonOff : 1;
	bool CoupleToSubsequentUnisonIntermanualCouplers : 1;
	bool CoupleToSubsequentUpwardIntermanualCouplers : 1;
	bool CoupleToSubsequentDownwardIntermanualCouplers : 1;
	bool CoupleToSubsequentUpwardIntramanualCouplers : 1;
	bool CoupleToSubsequentDownwardIntramanualCouplers : 1;
	wxInt16 DestinationManual;
	wxInt16 DestinationKeyshift;
};

#endif
