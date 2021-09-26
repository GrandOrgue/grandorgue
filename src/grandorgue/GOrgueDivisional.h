/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEDIVISIONAL_H
#define GORGUEDIVISIONAL_H

#include "GOrgueCombination.h"
#include "GOrguePushbutton.h"

class GOrgueConfigReader;
class GOrgueConfigWriter;

class GOrgueDivisional : public GOrguePushbutton, public GOrgueCombination
{
protected:
	int m_DivisionalNumber;
	unsigned m_ManualNumber;
	bool m_IsSetter;

	bool PushLocal();
public:
	GOrgueDivisional(GrandOrgueFile* organfile, GOrgueCombinationDefinition& divisional_template, bool is_setter);
	void Init(GOrgueConfigReader& cfg, wxString group, int manualNumber, int divisionalNumber, wxString name);
	void Load(GOrgueConfigReader& cfg, wxString group, int manualNumber, int divisionalNumber);
	void LoadCombination(GOrgueConfigReader& cfg);
	void Save(GOrgueConfigWriter& cfg);
	void Push();

	wxString GetMidiType();
};

#endif
