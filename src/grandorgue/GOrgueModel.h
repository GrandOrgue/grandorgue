/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2016 GrandOrgue contributors (see AUTHORS)
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
 */

#ifndef GORGUEMODEL_H
#define GORGUEMODEL_H

#include "ptrvector.h"

class GOrgueConfigReader;
class GOrgueDivisionalCoupler;
class GOrgueEnclosure;
class GOrgueGeneral;
class GOrgueManual;
class GOrguePiston;
class GOrgueRank;
class GOrgueSwitch;
class GOrgueTremulant;
class GOrgueWindchest;
class GrandOrgueFile;

class GOrgueModel
{
protected:
	ptr_vector<GOrgueWindchest> m_windchest;
	ptr_vector<GOrgueManual> m_manual;
	ptr_vector<GOrgueEnclosure> m_enclosure;
	ptr_vector<GOrgueSwitch> m_switches;
	ptr_vector<GOrgueTremulant> m_tremulant;
	ptr_vector<GOrgueRank> m_ranks;
	ptr_vector<GOrguePiston> m_piston;
	ptr_vector<GOrgueDivisionalCoupler> m_divisionalcoupler;
	ptr_vector<GOrgueGeneral> m_general;
	unsigned m_FirstManual;
	unsigned m_ODFManualCount;
	unsigned m_ODFRankCount;

	void Load(GOrgueConfigReader& cfg, GrandOrgueFile* organfile);

public:
	GOrgueModel();
	~GOrgueModel();

	void UpdateTremulant(GOrgueTremulant* tremulant);
	void UpdateVolume();

	unsigned GetWindchestGroupCount();
	unsigned AddWindchest(GOrgueWindchest* windchest);
	GOrgueWindchest* GetWindchest(unsigned index);

	GOrgueEnclosure* GetEnclosureElement(unsigned index);
	unsigned GetEnclosureCount();
	unsigned AddEnclosure(GOrgueEnclosure* enclosure);

	unsigned GetSwitchCount();
	GOrgueSwitch* GetSwitch(unsigned index);

	unsigned GetTremulantCount();
	GOrgueTremulant* GetTremulant(unsigned index);

	unsigned GetManualAndPedalCount();
	unsigned GetODFManualCount();
	unsigned GetFirstManualIndex();
	GOrgueManual* GetManual(unsigned index);

	GOrgueRank* GetRank(unsigned index);
	unsigned GetODFRankCount();
	void AddRank(GOrgueRank* rank);

	unsigned GetNumberOfReversiblePistons();
	GOrguePiston* GetPiston(unsigned index);

	unsigned GetDivisionalCouplerCount();
	GOrgueDivisionalCoupler* GetDivisionalCoupler(unsigned index);

	unsigned GetGeneralCount();
	GOrgueGeneral* GetGeneral(unsigned index);
};

#endif
