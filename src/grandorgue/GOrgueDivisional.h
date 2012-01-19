/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2012 GrandOrgue contributors (see AUTHORS)
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 * MA 02111-1307, USA.
 */

#ifndef GORGUEDIVISIONAL_H
#define GORGUEDIVISIONAL_H

#include <vector>
#include "GOrguePushbutton.h"

class IniFileConfig;

class GOrgueDivisional : public GOrguePushbutton
{
protected:
	void PushLocal();

public:

	int m_DivisionalNumber;
	unsigned m_ManualNumber;
	std::vector<int> m_Stops;
	std::vector<int> m_Couplers;
	std::vector<int> m_Tremulants;
	bool m_Protected;

	GOrgueDivisional(GrandOrgueFile* organfile);
	void Load(IniFileConfig& cfg, wxString group, int manualNumber, int divisionalNumber, wxString name = wxT(""));
	void LoadCombination(IniFileConfig& cfg);
	void Save(IniFileConfig& cfg, bool prefix);
	void Push();

};

#endif
