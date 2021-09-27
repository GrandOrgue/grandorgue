/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEDIVISIONALCOUPLER_H
#define GORGUEDIVISIONALCOUPLER_H

#include "GOrgueDrawStop.h"
#include <vector>

class GOrgueDivisionalCoupler : public GOrgueDrawstop
{
private:
 	bool m_BiDirectionalCoupling;
	std::vector<unsigned> m_manuals;

	void ChangeState(bool on);
	void SetupCombinationState();

public:
	GOrgueDivisionalCoupler(GrandOrgueFile* organfile);
	void Load(GOrgueConfigReader& cfg, wxString group);

	unsigned GetNumberOfManuals();
	unsigned GetManual(unsigned index);
	bool IsBidirectional();

	wxString GetMidiType();
};

#endif /* GORGUEDIVISIONALCOUPLER_H */
