/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUETEMPERAMENTUSER_H
#define GORGUETEMPERAMENTUSER_H

#include "GOrgueTemperamentCent.h"

class GOrgueConfigReader;
class GOrgueConfigWriter;

class GOrgueTemperamentUser : public GOrgueTemperamentCent
{
public:
	GOrgueTemperamentUser(wxString name, wxString title, wxString group);
	GOrgueTemperamentUser(GOrgueConfigReader& cfg, wxString group);

	void Save(GOrgueConfigWriter& cfg, wxString group);

	float GetNoteOffset(unsigned note);
	void SetNoteOffset(unsigned note, float offset);

	void SetTitle(wxString title);
	void SetGroup(wxString group);
};

#endif
