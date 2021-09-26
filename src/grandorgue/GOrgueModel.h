/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
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

	unsigned GetStopCount();
	unsigned GetCouplerCount();
	unsigned GetODFCouplerCount();
};

#endif
