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

#include "GOrgueTremulant.h"

GOrgueTremulant::GOrgueTremulant() :
	GOrgueDrawstop(),
	Period(0),
	StartRate(0),
	StopRate(0),
	AmpModDepth(0),
	pipe(NULL)
{

}

void GOrgueTremulant::Load(IniFileConfig& cfg, const char* group)
{
	Period=cfg.ReadLong( group,"Period",   32,  441000);
	StartRate=cfg.ReadInteger( group,"StartRate",    1,  100);
	StopRate=cfg.ReadInteger( group,"StopRate",    1,  100);
	AmpModDepth=cfg.ReadInteger( group,"AmpModDepth",    1,  100);
	GOrgueDrawstop::Load(cfg, group);
}

void GOrgueTremulant::Save(IniFileConfig& cfg, bool prefix)
{
	GOrgueDrawstop::Save(cfg, prefix, "Tremulant");
}

bool GOrgueTremulant::Set(bool on)
{
	if (DefaultToEngaged == on)
		return on;
	pipe->Set(on);
	return GOrgueDrawstop::Set(on);
}

