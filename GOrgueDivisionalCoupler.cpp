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

#include "GOrgueDivisionalCoupler.h"
#include "GrandOrgueFile.h"

/* TODO: This should not be... */
extern GrandOrgueFile* organfile;

GOrgueDivisionalCoupler::GOrgueDivisionalCoupler() :
	GOrgueDrawstop(),
	BiDirectionalCoupling(false),
	NumberOfManuals(0),
	manual()
{
	for (int i = 0; i < 7; ++i)
	{
		manual[i]=0;
	}
}


void GOrgueDivisionalCoupler::Load(IniFileConfig& cfg, const char* group)
{

	int i;
	char buffer[64];

	BiDirectionalCoupling=cfg.ReadBoolean( group,"BiDirectionalCoupling");
	NumberOfManuals=cfg.ReadInteger( group,"NumberOfManuals",    1,    6);
	for (i = 0; i < NumberOfManuals; i++)
	{
		sprintf(buffer, "Manual%03d", i + 1);
		manual[i] = cfg.ReadInteger(group, buffer, organfile->m_FirstManual, organfile->m_NumberOfManuals);
	}
	GOrgueDrawstop::Load(cfg, group);

}

void GOrgueDivisionalCoupler::Save(IniFileConfig& cfg, bool prefix)
{
	GOrgueDrawstop::Save(cfg, prefix, "DivisionalCoupler");
}

bool GOrgueDivisionalCoupler::Set(bool on)
{

	if (DefaultToEngaged == on)
		return on;
	return GOrgueDrawstop::Set(on);

}

