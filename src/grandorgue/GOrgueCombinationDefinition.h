/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUECOMBINATIONDEFINITION_H
#define GORGUECOMBINATIONDEFINITION_H

#include <wx/string.h>
#include <vector>

class GOrgueCombinationElement;
class GOrgueDrawstop;
class GOrgueManual;
class GrandOrgueFile;

class GOrgueCombinationDefinition
{
public:
	typedef enum { COMBINATION_STOP = 0, COMBINATION_COUPLER = 1, COMBINATION_TREMULANT = 2, COMBINATION_DIVISIONALCOUPLER = 3, COMBINATION_SWITCH = 4 } CombinationType;
	typedef struct
	{
		CombinationType type;
		int manual;
		unsigned index;
		bool store_unconditional;
		wxString group;
		GOrgueCombinationElement* control;
	} CombinationSlot;
private:
	GrandOrgueFile* m_organfile;
	std::vector<CombinationSlot> m_Content;

	void AddGeneral(GOrgueDrawstop* control, CombinationType type, int manual, unsigned index);
	void AddDivisional(GOrgueDrawstop* control, CombinationType type, int manual, unsigned index);
	void Add(GOrgueDrawstop* control, CombinationType type, int manual, unsigned index, bool store_unconditional);

public:
	GOrgueCombinationDefinition(GrandOrgueFile* organfile);
	~GOrgueCombinationDefinition();

	void InitGeneral();
	void InitDivisional(unsigned manual_number);

	int findEntry(CombinationType type, int manual, unsigned index);

	const std::vector<CombinationSlot>& GetCombinationElements();
};

#endif
