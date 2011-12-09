/*
 * GrandOrgue - free pipe organ simulator
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

#ifndef GORGUEDIVISIONALCOUPLER_H
#define GORGUEDIVISIONALCOUPLER_H

#include <vector>
#include <wx/wx.h>
#include "GOrgueDrawStop.h"

class GOrgueDivisionalCoupler : public GOrgueDrawstop
{
private:
 	bool m_BiDirectionalCoupling;
	std::vector<unsigned> m_manuals;

public:
	GOrgueDivisionalCoupler(GrandOrgueFile* organfile);
	void Load(IniFileConfig& cfg, wxString group);
	void Save(IniFileConfig& cfg, bool prefix);
	void Set(bool on);

	unsigned GetNumberOfManuals();
	unsigned GetManual(unsigned index);
	bool IsBidirectional();
};

#endif /* GORGUEDIVISIONALCOUPLER_H */
